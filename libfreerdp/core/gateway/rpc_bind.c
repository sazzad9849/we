/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * RPC Secure Context Binding
 *
 * Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <freerdp/config.h>

#include "../settings.h"

#include <winpr/crt.h>
#include <winpr/assert.h>

#include <freerdp/log.h>

#include "rpc_client.h"

#include "rts.h"

#include "rpc_bind.h"
#include "../utils.h"

#define TAG FREERDP_TAG("core.gateway.rpc")

#define AUTH_PKG NTLM_SSP_NAME

/**
 * Connection-Oriented RPC Protocol Client Details:
 * http://msdn.microsoft.com/en-us/library/cc243724/
 */

/* Syntax UUIDs */

const p_uuid_t TSGU_UUID = {
	0x44E265DD,                            /* time_low */
	0x7DAF,                                /* time_mid */
	0x42CD,                                /* time_hi_and_version */
	0x85,                                  /* clock_seq_hi_and_reserved */
	0x60,                                  /* clock_seq_low */
	{ 0x3C, 0xDB, 0x6E, 0x7A, 0x27, 0x29 } /* node[6] */
};

const p_uuid_t NDR_UUID = {
	0x8A885D04,                            /* time_low */
	0x1CEB,                                /* time_mid */
	0x11C9,                                /* time_hi_and_version */
	0x9F,                                  /* clock_seq_hi_and_reserved */
	0xE8,                                  /* clock_seq_low */
	{ 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60 } /* node[6] */
};

const p_uuid_t BTFN_UUID = {
	0x6CB71C2C,                            /* time_low */
	0x9812,                                /* time_mid */
	0x4540,                                /* time_hi_and_version */
	0x03,                                  /* clock_seq_hi_and_reserved */
	0x00,                                  /* clock_seq_low */
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } /* node[6] */
};

/**
 *           Secure Connection-Oriented RPC Packet Sequence
 *
 *     Client                                              Server
 *        |                                                   |
 *        |-------------------SECURE_BIND-------------------->|
 *        |                                                   |
 *        |<----------------SECURE_BIND_ACK-------------------|
 *        |                                                   |
 *        |--------------------RPC_AUTH_3-------------------->|
 *        |                                                   |
 *        |                                                   |
 *        |------------------REQUEST_PDU_#1------------------>|
 *        |------------------REQUEST_PDU_#2------------------>|
 *        |                                                   |
 *        |                        ...                        |
 *        |                                                   |
 *        |<-----------------RESPONSE_PDU_#1------------------|
 *        |<-----------------RESPONSE_PDU_#2------------------|
 *        |                                                   |
 *        |                        ...                        |
 */

/**
 * SECURE_BIND: RPC bind PDU with sec_trailer and auth_token. Auth_token is generated by calling
 * the implementation equivalent of the abstract GSS_Init_sec_context call. Upon receiving that, the
 * server calls the implementation equivalent of the abstract GSS_Accept_sec_context call, which
 * returns an auth_token and continue status in this example. Assume the following:
 *
 * 1) The client chooses the auth_context_id field in the sec_trailer sent with this PDU to be 1.
 *
 * 2) The client uses the RPC_C_AUTHN_LEVEL_PKT_PRIVACY authentication level and the
 *    Authentication Service (AS) NTLM.
 *
 * 3) The client sets the PFC_SUPPORT_HEADER_SIGN flag in the PDU header.
 */

static int rpc_bind_setup(rdpRpc* rpc)
{
	int rc = -1;
	rdpContext* context = NULL;
	rdpSettings* settings = NULL;
	freerdp* instance = NULL;
	SEC_WINNT_AUTH_IDENTITY identity = { 0 };

	WINPR_ASSERT(rpc);

	context = transport_get_context(rpc->transport);
	WINPR_ASSERT(context);

	settings = context->settings;
	WINPR_ASSERT(settings);

	instance = context->instance;
	WINPR_ASSERT(instance);

	credssp_auth_free(rpc->auth);
	rpc->auth = credssp_auth_new(context);
	if (!rpc->auth)
	{
		return -1;
	}

	rc = utils_authenticate_gateway(instance, GW_AUTH_RPC);
	switch (rc)
	{
		case AUTH_SUCCESS:
		case AUTH_SKIP:
			break;
		case AUTH_CANCELLED:
			freerdp_set_last_error_log(instance->context, FREERDP_ERROR_CONNECT_CANCELLED);
			return -1;
		case AUTH_NO_CREDENTIALS:
			WLog_INFO(TAG, "No credentials provided - using NULL identity");
			break;
		case AUTH_FAILED:
		default:
			return -1;
	}

	if (!credssp_auth_init(rpc->auth, AUTH_PKG, NULL))
	{
		return -1;
	}

	if (!identity_set_from_settings(&identity, settings, FreeRDP_GatewayUsername,
	                                FreeRDP_GatewayDomain, FreeRDP_GatewayPassword))
	{
		return -1;
	}

	SEC_WINNT_AUTH_IDENTITY* identityArg = (settings->GatewayUsername ? &identity : NULL);
	if (!credssp_auth_setup_client(rpc->auth, NULL, settings->GatewayHostname, identityArg, NULL))
	{
		sspi_FreeAuthIdentity(&identity);
		return -1;
	}
	sspi_FreeAuthIdentity(&identity);

	credssp_auth_set_flags(rpc->auth, ISC_REQ_USE_DCE_STYLE | ISC_REQ_DELEGATE |
	                                      ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT);

	if (credssp_auth_authenticate(rpc->auth) < 0)
	{
		return -1;
	}

	return 1;
}

int rpc_send_bind_pdu(rdpRpc* rpc, BOOL initial)
{
	int status = -1;
	wStream* buffer = NULL;
	UINT32 offset = 0;
	RpcClientCall* clientCall = NULL;
	p_cont_elem_t* p_cont_elem = NULL;
	rpcconn_bind_hdr_t bind_pdu = { 0 };
	RpcVirtualConnection* connection = NULL;
	RpcInChannel* inChannel = NULL;
	const SecBuffer* sbuffer = NULL;

	WINPR_ASSERT(rpc);

	connection = rpc->VirtualConnection;

	WINPR_ASSERT(connection);

	inChannel = connection->DefaultInChannel;

	if (initial && rpc_bind_setup(rpc) < 0)
	{
		return -1;
	}

	WLog_DBG(TAG, initial ? "Sending Bind PDU" : "Sending Alter Context PDU");

	sbuffer = credssp_auth_get_output_buffer(rpc->auth);

	if (!sbuffer)
	{
		goto fail;
	}

	bind_pdu.header = rpc_pdu_header_init(rpc);
	bind_pdu.header.auth_length = (UINT16)sbuffer->cbBuffer;
	bind_pdu.auth_verifier.auth_value = sbuffer->pvBuffer;
	bind_pdu.header.ptype = initial ? PTYPE_BIND : PTYPE_ALTER_CONTEXT;
	bind_pdu.header.pfc_flags =
	    PFC_FIRST_FRAG | PFC_LAST_FRAG | PFC_SUPPORT_HEADER_SIGN | PFC_CONC_MPX;
	bind_pdu.header.call_id = 2;
	bind_pdu.max_xmit_frag = rpc->max_xmit_frag;
	bind_pdu.max_recv_frag = rpc->max_recv_frag;
	bind_pdu.assoc_group_id = 0;
	bind_pdu.p_context_elem.n_context_elem = 2;
	bind_pdu.p_context_elem.reserved = 0;
	bind_pdu.p_context_elem.reserved2 = 0;
	bind_pdu.p_context_elem.p_cont_elem =
	    calloc(bind_pdu.p_context_elem.n_context_elem, sizeof(p_cont_elem_t));

	if (!bind_pdu.p_context_elem.p_cont_elem)
	{
		goto fail;
	}

	p_cont_elem = &bind_pdu.p_context_elem.p_cont_elem[0];
	p_cont_elem->p_cont_id = 0;
	p_cont_elem->n_transfer_syn = 1;
	p_cont_elem->reserved = 0;
	CopyMemory(&(p_cont_elem->abstract_syntax.if_uuid), &TSGU_UUID, sizeof(p_uuid_t));
	p_cont_elem->abstract_syntax.if_version = TSGU_SYNTAX_IF_VERSION;
	p_cont_elem->transfer_syntaxes = malloc(sizeof(p_syntax_id_t));

	if (!p_cont_elem->transfer_syntaxes)
	{
		goto fail;
	}

	CopyMemory(&(p_cont_elem->transfer_syntaxes[0].if_uuid), &NDR_UUID, sizeof(p_uuid_t));
	p_cont_elem->transfer_syntaxes[0].if_version = NDR_SYNTAX_IF_VERSION;
	p_cont_elem = &bind_pdu.p_context_elem.p_cont_elem[1];
	p_cont_elem->p_cont_id = 1;
	p_cont_elem->n_transfer_syn = 1;
	p_cont_elem->reserved = 0;
	CopyMemory(&(p_cont_elem->abstract_syntax.if_uuid), &TSGU_UUID, sizeof(p_uuid_t));
	p_cont_elem->abstract_syntax.if_version = TSGU_SYNTAX_IF_VERSION;
	p_cont_elem->transfer_syntaxes = malloc(sizeof(p_syntax_id_t));

	if (!p_cont_elem->transfer_syntaxes)
	{
		goto fail;
	}

	CopyMemory(&(p_cont_elem->transfer_syntaxes[0].if_uuid), &BTFN_UUID, sizeof(p_uuid_t));
	p_cont_elem->transfer_syntaxes[0].if_version = BTFN_SYNTAX_IF_VERSION;
	offset = 116;

	bind_pdu.auth_verifier.auth_type =
	    rpc_auth_pkg_to_security_provider(credssp_auth_pkg_name(rpc->auth));
	bind_pdu.auth_verifier.auth_level = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
	bind_pdu.auth_verifier.auth_reserved = 0x00;
	bind_pdu.auth_verifier.auth_context_id = 0x00000000;
	offset += (8 + bind_pdu.header.auth_length);
	bind_pdu.header.frag_length = offset;

	buffer = Stream_New(NULL, bind_pdu.header.frag_length);

	if (!buffer)
	{
		goto fail;
	}

	if (!rts_write_pdu_bind(buffer, &bind_pdu))
	{
		goto fail;
	}

	clientCall = rpc_client_call_new(bind_pdu.header.call_id, 0);

	if (!clientCall)
	{
		goto fail;
	}

	if (!ArrayList_Append(rpc->client->ClientCallList, clientCall))
	{
		rpc_client_call_free(clientCall);
		goto fail;
	}

	Stream_SealLength(buffer);
	status = rpc_in_channel_send_pdu(inChannel, Stream_Buffer(buffer), Stream_Length(buffer));
fail:

	if (bind_pdu.p_context_elem.p_cont_elem)
	{
		free(bind_pdu.p_context_elem.p_cont_elem[0].transfer_syntaxes);
		free(bind_pdu.p_context_elem.p_cont_elem[1].transfer_syntaxes);
	}

	free(bind_pdu.p_context_elem.p_cont_elem);
	bind_pdu.p_context_elem.p_cont_elem = NULL;

	Stream_Free(buffer, TRUE);
	return (status > 0) ? 1 : -1;
}

/**
 * Maximum Transmit/Receive Fragment Size Negotiation
 *
 * The client determines, and then sends in the bind PDU, its desired maximum size for transmitting
 * fragments, and its desired maximum receive fragment size. Similarly, the server determines its
 * desired maximum sizes for transmitting and receiving fragments. Transmit and receive sizes may be
 * different to help preserve buffering. When the server receives the client’s values, it sets its
 * operational transmit size to the minimum of the client’s receive size (from the bind PDU) and its
 * own desired transmit size. Then it sets its actual receive size to the minimum of the client’s
 * transmit size (from the bind) and its own desired receive size. The server then returns its
 * operational values in the bind_ack PDU. The client then sets its operational values from the
 * received bind_ack PDU. The received transmit size becomes the client’s receive size, and the
 * received receive size becomes the client’s transmit size. Either party may use receive buffers
 * larger than negotiated — although this will not provide any advantage — but may not transmit
 * larger fragments than negotiated.
 */

/**
 *
 * SECURE_BIND_ACK: RPC bind_ack PDU with sec_trailer and auth_token. The PFC_SUPPORT_HEADER_SIGN
 * flag in the PDU header is also set in this example. Auth_token is generated by the server in the
 * previous step. Upon receiving that PDU, the client calls the implementation equivalent of the
 * abstract GSS_Init_sec_context call, which returns an auth_token and continue status in this
 * example.
 */

BOOL rpc_recv_bind_ack_pdu(rdpRpc* rpc, wStream* s)
{
	BOOL rc = FALSE;
	const BYTE* auth_data = NULL;
	size_t pos;
	size_t end;
	rpcconn_hdr_t header = { 0 };
	SecBuffer buffer = { 0 };

	WINPR_ASSERT(rpc);
	WINPR_ASSERT(rpc->auth);
	WINPR_ASSERT(s);

	pos = Stream_GetPosition(s);
	if (!rts_read_pdu_header(s, &header))
	{
		goto fail;
	}

	WLog_DBG(TAG, header.common.ptype == PTYPE_BIND_ACK ? "Receiving BindAck PDU"
	                                                    : "Receiving AlterContextResp PDU");

	rpc->max_recv_frag = header.bind_ack.max_xmit_frag;
	rpc->max_xmit_frag = header.bind_ack.max_recv_frag;

	/* Get the correct offset in the input data and pass that on as input buffer.
	 * rts_read_pdu_header did already do consistency checks */
	end = Stream_GetPosition(s);
	Stream_SetPosition(s, pos + header.common.frag_length - header.common.auth_length);
	auth_data = Stream_ConstPointer(s);
	Stream_SetPosition(s, end);

	buffer.cbBuffer = header.common.auth_length;
	buffer.pvBuffer = malloc(buffer.cbBuffer);
	if (!buffer.pvBuffer)
	{
		goto fail;
	}
	memcpy(buffer.pvBuffer, auth_data, buffer.cbBuffer);
	credssp_auth_take_input_buffer(rpc->auth, &buffer);

	if (credssp_auth_authenticate(rpc->auth) < 0)
	{
		goto fail;
	}

	rc = TRUE;
fail:
	rts_free_pdu_header(&header, FALSE);
	return rc;
}

/**
 * RPC_AUTH_3: The client knows that this is an NTLM that uses three legs. It sends an rpc_auth_3
 * PDU with the auth_token obtained in the previous step. Upon receiving this PDU, the server calls
 * the implementation equivalent of the abstract GSS_Accept_sec_context call, which returns success
 * status in this example.
 */

int rpc_send_rpc_auth_3_pdu(rdpRpc* rpc)
{
	int status = -1;
	wStream* buffer = NULL;
	size_t offset = 0;
	const SecBuffer* sbuffer = NULL;
	RpcClientCall* clientCall = NULL;
	rpcconn_rpc_auth_3_hdr_t auth_3_pdu = { 0 };
	RpcVirtualConnection* connection = NULL;
	RpcInChannel* inChannel = NULL;

	WINPR_ASSERT(rpc);

	connection = rpc->VirtualConnection;
	WINPR_ASSERT(connection);

	inChannel = connection->DefaultInChannel;
	WINPR_ASSERT(inChannel);

	WLog_DBG(TAG, "Sending RpcAuth3 PDU");

	sbuffer = credssp_auth_get_output_buffer(rpc->auth);

	if (!sbuffer)
	{
		return -1;
	}

	auth_3_pdu.header = rpc_pdu_header_init(rpc);
	auth_3_pdu.header.auth_length = (UINT16)sbuffer->cbBuffer;
	auth_3_pdu.auth_verifier.auth_value = sbuffer->pvBuffer;
	auth_3_pdu.header.ptype = PTYPE_RPC_AUTH_3;
	auth_3_pdu.header.pfc_flags = PFC_FIRST_FRAG | PFC_LAST_FRAG | PFC_CONC_MPX;
	auth_3_pdu.header.call_id = 2;
	auth_3_pdu.max_xmit_frag = rpc->max_xmit_frag;
	auth_3_pdu.max_recv_frag = rpc->max_recv_frag;
	offset = 20;
	auth_3_pdu.auth_verifier.auth_pad_length = rpc_offset_align(&offset, 4);
	auth_3_pdu.auth_verifier.auth_type =
	    rpc_auth_pkg_to_security_provider(credssp_auth_pkg_name(rpc->auth));
	auth_3_pdu.auth_verifier.auth_level = RPC_C_AUTHN_LEVEL_PKT_INTEGRITY;
	auth_3_pdu.auth_verifier.auth_reserved = 0x00;
	auth_3_pdu.auth_verifier.auth_context_id = 0x00000000;
	offset += (8 + auth_3_pdu.header.auth_length);
	auth_3_pdu.header.frag_length = offset;

	buffer = Stream_New(NULL, auth_3_pdu.header.frag_length);

	if (!buffer)
	{
		return -1;
	}

	if (!rts_write_pdu_auth3(buffer, &auth_3_pdu))
	{
		goto fail;
	}

	clientCall = rpc_client_call_new(auth_3_pdu.header.call_id, 0);

	if (ArrayList_Append(rpc->client->ClientCallList, clientCall))
	{
		Stream_SealLength(buffer);
		status = rpc_in_channel_send_pdu(inChannel, Stream_Buffer(buffer), Stream_Length(buffer));
	}

fail:
	Stream_Free(buffer, TRUE);
	return (status > 0) ? 1 : -1;
}

enum RPC_BIND_STATE rpc_bind_state(rdpRpc* rpc)
{
	BOOL complete;
	BOOL have_token;
	WINPR_ASSERT(rpc);

	complete = credssp_auth_is_complete(rpc->auth);
	have_token = credssp_auth_have_output_token(rpc->auth);

	return complete ? (have_token ? RPC_BIND_STATE_LAST_LEG : RPC_BIND_STATE_COMPLETE)
	                : RPC_BIND_STATE_INCOMPLETE;
}

BYTE rpc_auth_pkg_to_security_provider(const char* name)
{
	if (strcmp(name, CREDSSP_AUTH_PKG_SPNEGO) == 0)
	{
		return RPC_C_AUTHN_GSS_NEGOTIATE;
	}
	if (strcmp(name, CREDSSP_AUTH_PKG_NTLM) == 0)
		return RPC_C_AUTHN_WINNT;
	else if (strcmp(name, CREDSSP_AUTH_PKG_KERBEROS) == 0)
		return RPC_C_AUTHN_GSS_KERBEROS;
	else if (strcmp(name, CREDSSP_AUTH_PKG_SCHANNEL) == 0)
		return RPC_C_AUTHN_GSS_SCHANNEL;
	else
		return RPC_C_AUTHN_NONE;
}
