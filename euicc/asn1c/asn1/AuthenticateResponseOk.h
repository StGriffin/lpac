/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "RSPDefinitions"
 * 	found in "../../../asn1/rsp.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -no-gen-example`
 */

#ifndef	_AuthenticateResponseOk_H_
#define	_AuthenticateResponseOk_H_


#include "asn_application.h"

/* Including external dependencies */
#include "EuiccSigned1.h"
#include "OCTET_STRING.h"
#include "Certificate.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* AuthenticateResponseOk */
typedef struct AuthenticateResponseOk {
	EuiccSigned1_t	 euiccSigned1;
	OCTET_STRING_t	 euiccSignature1;
	Certificate_t	 euiccCertificate;
	Certificate_t	 eumCertificate;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AuthenticateResponseOk_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AuthenticateResponseOk;
extern asn_SEQUENCE_specifics_t asn_SPC_AuthenticateResponseOk_specs_1;
extern asn_TYPE_member_t asn_MBR_AuthenticateResponseOk_1[4];

#ifdef __cplusplus
}
#endif

#endif	/* _AuthenticateResponseOk_H_ */
#include "asn_internal.h"