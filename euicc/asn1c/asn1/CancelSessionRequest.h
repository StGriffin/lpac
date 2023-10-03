/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "../../../asn1/rsp.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_CancelSessionRequest_H_
#define	_CancelSessionRequest_H_


#include "asn_application.h"

/* Including external dependencies */
#include "TransactionId.h"
#include "CancelSessionReason.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CancelSessionRequest */
typedef struct CancelSessionRequest {
	TransactionId_t	 transactionId;
	CancelSessionReason_t	 reason;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} CancelSessionRequest_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_CancelSessionRequest;

#ifdef __cplusplus
}
#endif

#endif	/* _CancelSessionRequest_H_ */
#include "asn_internal.h"