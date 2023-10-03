#include "es10x.h"
#include "es10x.private.h"
#include "hexutil.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define APDU_ST33_MAGIC "\x90\xBD\x36\xBB\x00"
#define APDU_TERMINAL_CAPABILITIES "\x80\xAA\x00\x00\x0A\xA9\x08\x81\x00\x82\x01\x01\x83\x01\x07"
#define ISD_R_AID "\xA0\x00\x00\x05\x59\x10\x10\xFF\xFF\xFF\xFF\x89\x00\x00\x01\x00"
#define APDU_CONTINUE_READ_HEADER 0x80, 0xC0, 0x00, 0x00
#define APDU_EUICC_HEADER 0x80, 0xE2

static int es10x_transmit(struct euicc_ctx *ctx, struct apdu_response *response, struct apdu_request *req, unsigned req_len)
{
    req->cla = (req->cla & 0xF0) | (ctx->es10x_logic_channel & 0x0F);
    return euicc_apdu_transmit(ctx, response, req, req_len);
}

static int es10x_transmit_iter(struct euicc_ctx *ctx, struct apdu_request *req, unsigned req_len, int (*callback)(struct apdu_response *response, void *userdata), void *userdata)
{
    int ret;
    struct apdu_request *request = NULL;
    struct apdu_response response;

    ret = es10x_transmit(ctx, &response, req, req_len);
    if (ret < 0)
    {
        goto err;
    }
    do
    {
        if (response.length > 0)
        {
            ret = callback(&response, userdata);
            if (ret < 0)
                goto err;
        }

        switch (response.sw1)
        {
        case SW1_OK:
            return 0;
        case SW1_LAST:
            ret = euicc_apdu_le(ctx, &request, APDU_CONTINUE_READ_HEADER, response.sw2);
            if (ret < 0)
                goto err;
            ret = es10x_transmit(ctx, &response, request, ret);
            if (ret < 0)
                goto err;
            continue;
        default:
            if ((response.sw1 & 0xF0) == SW1_OK)
            {
                return 0;
            }
            goto err;
        }
    } while (1);

err:
    return -1;
}

int es10x_command_buildrequest(struct euicc_ctx *ctx, struct apdu_request **request, uint8_t p1, uint8_t p2, uint8_t *der_req, unsigned req_len)
{
    int ret;

    ret = euicc_apdu_lc(ctx, request, APDU_EUICC_HEADER, p1, p2, req_len);
    if (ret < 0)
        return ret;

    memcpy((*request)->data, der_req, req_len);

    return ret;
}

static int es10x_command_buildrequest_continue(struct euicc_ctx *ctx, uint8_t reqseq, struct apdu_request **request, uint8_t *der_req, unsigned req_len)
{
    return es10x_command_buildrequest(ctx, request, 0x11, reqseq, der_req, req_len);
}

static int es10x_command_buildrequest_last(struct euicc_ctx *ctx, uint8_t reqseq, struct apdu_request **request, uint8_t *der_req, unsigned req_len)
{
    return es10x_command_buildrequest(ctx, request, 0x91, reqseq, der_req, req_len);
}

int es10x_command_iter(struct euicc_ctx *ctx, uint8_t *der_req, unsigned req_len, int (*callback)(struct apdu_response *response, void *userdata), void *userdata)
{
    int ret, reqseq;
    struct apdu_request *req;
    uint8_t *req_ptr;

    reqseq = 0;
    req_ptr = der_req;
    while (req_len)
    {
        uint8_t rlen;
        if (req_len > 120)
        {
            rlen = 120;
            ret = es10x_command_buildrequest_continue(ctx, reqseq, &req, req_ptr, rlen);
        }
        else
        {
            rlen = req_len;
            ret = es10x_command_buildrequest_last(ctx, reqseq, &req, req_ptr, rlen);
        }
        req_len -= rlen;

        if (ret < 0)
            return -1;

        ret = es10x_transmit_iter(ctx, req, ret, callback, userdata);
        if (ret < 0)
            return -1;

        req_ptr += rlen;
        reqseq++;
    }

    return 0;
}

struct userdata_es10x_command
{
    uint8_t *resp;
    unsigned resp_len;
};

static int iter_es10x_command(struct apdu_response *response, void *userdata)
{
    struct userdata_es10x_command *ud = (struct userdata_es10x_command *)userdata;
    uint8_t *new_response_data;

    new_response_data = realloc(ud->resp, ud->resp_len + response->length);
    if (!new_response_data)
    {
        return -1;
    }
    ud->resp = new_response_data;
    memcpy(ud->resp + ud->resp_len, response->data, response->length);
    ud->resp_len += response->length;
    return 0;
}

int es10x_command(struct euicc_ctx *ctx, uint8_t **resp, unsigned *resp_len, uint8_t *der_req, unsigned req_len)
{
    int ret = 0;
    struct userdata_es10x_command ud;

    *resp = NULL;
    *resp_len = 0;
    memset(&ud, 0, sizeof(ud));

    ret = es10x_command_iter(ctx, der_req, req_len, iter_es10x_command, &ud);
    if (ret < 0)
    {
        free(ud.resp);
        return -1;
    }

    *resp = ud.resp;
    *resp_len = ud.resp_len;
    return 0;
}

int es10x_init(struct euicc_ctx *ctx)
{
    int ret;
    struct apdu_request *request = NULL;
    struct apdu_response response;

    ret = ctx->interface.apdu.connect();
    if (ret < 0)
    {
        return -1;
    }

    if (euicc_apdu_transmit(ctx, &response, (struct apdu_request *)APDU_ST33_MAGIC, sizeof(APDU_ST33_MAGIC) - 1) < 0)
    {
        return -1;
    }

    ctx->interface.apdu.disconnect();
    ret = ctx->interface.apdu.connect();
    if (ret < 0)
    {
        return -1;
    }

    ret = euicc_apdu_transmit(ctx, &response, (struct apdu_request *)APDU_TERMINAL_CAPABILITIES, sizeof(APDU_TERMINAL_CAPABILITIES) - 1);
    if (ret < 0)
    {
        return -1;
    }

    if ((ctx->es10x_logic_channel = ctx->interface.apdu.logic_channel_open(ISD_R_AID, sizeof(ISD_R_AID) - 1)) < 0)
    {
        return -1;
    }

    return 0;
}

void es10x_fini(struct euicc_ctx *ctx)
{
    ctx->interface.apdu.logic_channel_close(ctx->es10x_logic_channel);
    ctx->interface.apdu.disconnect();
    ctx->es10x_logic_channel = 0;
}