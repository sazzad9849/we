/**
 * FreeRDP: A Remote Desktop Protocol Client
 * Error Info
 *
 * Copyright 2011 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

#ifndef __ERRINFO_H
#define __ERRINFO_H

#include <freerdp/freerdp.h>

/* Protocol-independent codes */
#define ERRINFO_RPC_INITIATED_DISCONNECT			0x00000001
#define ERRINFO_RPC_INITIATED_LOGOFF				0x00000002
#define ERRINFO_IDLE_TIMEOUT					0x00000003
#define ERRINFO_LOGON_TIMEOUT					0x00000004
#define ERRINFO_DISCONNECTED_BY_OTHER_CONNECTION		0x00000005
#define ERRINFO_OUT_OF_MEMORY					0x00000006
#define ERRINFO_SERVER_DENIED_CONNECTION			0x00000007
#define ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES			0x00000009
#define ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED		0x0000000A
#define ERRINFO_RPC_INITIATED_DISCONNECT_BY_USER		0x0000000B

/* Protocol-independent licensing codes */
#define	ERRINFO_LICENSE_INTERNAL				0x00000100
#define ERRINFO_LICENSE_NO_LICENSE_SERVER			0x00000101
#define ERRINFO_LICENSE_NO_LICENSE				0x00000102
#define ERRINFO_LICENSE_BAD_CLIENT_MSG				0x00000103
#define ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE		0x00000104
#define ERRINFO_LICENSE_BAD_CLIENT_LICENSE			0x00000105
#define ERRINFO_LICENSE_CANT_FINISH_PROTOCOL			0x00000106
#define ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL			0x00000107
#define ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION			0x00000108
#define ERRINFO_LICENSE_CANT_UPGRADE_LICENSE			0x00000109
#define ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS			0x0000010A

/* RDP specific codes */
#define ERRINFO_UNKNOWN_DATA_PDU_TYPE				0x000010C9
#define ERRINFO_UNKNOWN_PDU_TYPE				0x000010CA
#define ERRINFO_DATA_PDU_SEQUENCE				0x000010CB
#define ERRINFO_CONTROL_PDU_SEQUENCE				0x000010CD
#define ERRINFO_INVALID_CONTROL_PDU_ACTION			0x000010CE
#define ERRINFO_INVALID_INPUT_PDU_TYPE				0x000010CF
#define ERRINFO_INVALID_INPUT_PDU_MOUSE				0x000010D0
#define ERRINFO_INVALID_REFRESH_RECT_PDU			0x000010D1
#define ERRINFO_CREATE_USER_DATA_FAILED				0x000010D2
#define ERRINFO_CONNECT_FAILED					0x000010D3
#define ERRINFO_CONFIRM_ACTIVE_HAS_WRONG_SHAREID		0x000010D4
#define ERRINFO_CONFIRM_ACTIVE_HAS_WRONG_ORIGINATOR		0x000010D5
#define ERRINFO_PERSISTENT_KEY_PDU_BAD_LENGTH			0x000010DA
#define ERRINFO_PERSISTENT_KEY_PDU_ILLEGAL_FIRST		0x000010DB
#define ERRINFO_PERSISTENT_KEY_PDU_TOO_MANY_TOTAL_KEYS		0x000010DC
#define ERRINFO_PERSISTENT_KEY_PDU_TOO_MANY_CACHE_KEYS		0x000010DD
#define ERRINFO_INPUT_PDU_BAD_LENGTH				0x000010DE
#define ERRINFO_BITMAP_CACHE_ERROR_PDU_BAD_LENGTH		0x000010DF
#define ERRINFO_SECURITY_DATA_TOO_SHORT				0x000010E0
#define ERRINFO_VCHANNEL_DATA_TOO_SHORT				0x000010E1
#define ERRINFO_SHARE_DATA_TOO_SHORT				0x000010E2
#define ERRINFO_BAD_SUPPRESS_OUTPUT_PDU				0x000010E3
#define ERRINFO_CONFIRM_ACTIVE_PDU_TOO_SHORT			0x000010E5
#define ERRINFO_CAPABILITY_SET_TOO_SMALL			0x000010E7
#define ERRINFO_CAPABILITY_SET_TOO_LARGE			0x000010E8
#define ERRINFO_NO_CURSOR_CACHE					0x000010E9
#define ERRINFO_BAD_CAPABILITIES				0x000010EA
#define ERRINFO_VIRTUAL_CHANNEL_DECOMPRESSION			0x000010EC
#define ERRINFO_INVALID_VC_COMPRESSION_TYPE			0x000010ED
#define ERRINFO_INVALID_CHANNEL_ID				0x000010EF
#define ERRINFO_VCHANNELS_TOO_MANY				0x000010F0
#define ERRINFO_REMOTEAPP_NOT_ENABLED				0x000010F3
#define ERRINFO_CACHE_CAP_NOT_SET				0x000010F4
#define ERRINFO_BITMAP_CACHE_ERROR_PDU_BAD_LENGTH2 		0x000010F5
#define ERRINFO_OFFSCREEN_CACHE_ERROR_PDU_BAD_LENGTH		0x000010F6
#define ERRINFO_DRAWNINEGRID_CACHE_ERROR_PDU_BAD_LENGTH		0x000010F7
#define ERRINFO_GDIPLUS_PDU_BAD_LENGTH				0x000010F8
#define ERRINFO_SECURITY_DATA_TOO_SHORT2			0x00001111
#define ERRINFO_SECURITY_DATA_TOO_SHORT3			0x00001112
#define ERRINFO_SECURITY_DATA_TOO_SHORT4			0x00001113
#define ERRINFO_SECURITY_DATA_TOO_SHORT5			0x00001114
#define ERRINFO_SECURITY_DATA_TOO_SHORT6			0x00001115
#define ERRINFO_SECURITY_DATA_TOO_SHORT7			0x00001116
#define ERRINFO_SECURITY_DATA_TOO_SHORT8			0x00001117
#define ERRINFO_SECURITY_DATA_TOO_SHORT9			0x00001118
#define ERRINFO_SECURITY_DATA_TOO_SHORT10			0x00001119
#define ERRINFO_SECURITY_DATA_TOO_SHORT11			0x0000111A
#define ERRINFO_SECURITY_DATA_TOO_SHORT12			0x0000111B
#define ERRINFO_SECURITY_DATA_TOO_SHORT13			0x0000111C
#define ERRINFO_SECURITY_DATA_TOO_SHORT14			0x0000111D
#define ERRINFO_SECURITY_DATA_TOO_SHORT15			0x0000111E
#define ERRINFO_SECURITY_DATA_TOO_SHORT16			0x0000111F
#define ERRINFO_SECURITY_DATA_TOO_SHORT17			0x00001120
#define ERRINFO_SECURITY_DATA_TOO_SHORT18			0x00001121
#define ERRINFO_SECURITY_DATA_TOO_SHORT19			0x00001122
#define ERRINFO_SECURITY_DATA_TOO_SHORT20			0x00001123
#define ERRINFO_SECURITY_DATA_TOO_SHORT21			0x00001124
#define ERRINFO_SECURITY_DATA_TOO_SHORT22			0x00001125
#define ERRINFO_SECURITY_DATA_TOO_SHORT23			0x00001126
#define ERRINFO_BAD_MONITOR_DATA				0x00001129
#define ERRINFO_VC_DECOMPRESSED_REASSEMBLE_FAILED		0x0000112A
#define ERRINFO_VC_DATA_TOO_LONG				0x0000112B
#define ERRINFO_GRAPHICS_MODE_NOT_SUPPORTED			0x0000112D
#define ERRINFO_GRAPHICS_SUBSYSTEM_RESET_FAILED			0x0000112E
#define ERRINFO_UPDATE_SESSION_KEY_FAILED			0x00001191
#define ERRINFO_DECRYPT_FAILED					0x00001192
#define ERRINFO_ENCRYPT_FAILED					0x00001193
#define ERRINFO_ENCRYPTION_PACKAGE_MISMATCH			0x00001194
#define ERRINFO_DECRYPT_FAILED2					0x00001195

#define ERRINFO_SUCCESS						0x00000000
#define ERRINFO_NONE						0xFFFFFFFF

struct _ERRINFO
{
	uint32 code;
	char* name;
	char* info;
};
typedef struct _ERRINFO ERRINFO;

#define ERRINFO_DEFINE(_code)	{ ERRINFO_##_code , "ERRINFO_" #_code , ERRINFO_##_code##_STRING }

void rdp_print_errinfo(uint32 code);

#endif
/* Modeline for vim. Don't delete */
/* vim: set cindent:noet:sw=8:ts=8 */
