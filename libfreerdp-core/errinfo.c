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

#include "errinfo.h"

int connectErrorCode;

/* Protocol-independent codes */

#define ERRINFO_RPC_INITIATED_DISCONNECT_STRING \
		"The disconnection was initiated by an administrative tool on the server in another session."

#define ERRINFO_RPC_INITIATED_LOGOFF_STRING \
		"The disconnection was due to a forced logoff initiated by an administrative tool on the server in another session."

#define ERRINFO_IDLE_TIMEOUT_STRING \
		"The idle session limit timer on the server has elapsed."

#define ERRINFO_LOGON_TIMEOUT_STRING \
		"The active session limit timer on the server has elapsed."

#define ERRINFO_DISCONNECTED_BY_OTHER_CONNECTION_STRING \
		"Another user connected to the server, forcing the disconnection of the current connection."

#define ERRINFO_OUT_OF_MEMORY_STRING \
		"The server ran out of available memory resources."

#define ERRINFO_SERVER_DENIED_CONNECTION_STRING \
		"The server denied the connection."

#define ERRINFO_SERVER_INSUFFICIENT_PRIVILEGES_STRING \
		"The user cannot connect to the server due to insufficient access privileges."

#define ERRINFO_SERVER_FRESH_CREDENTIALS_REQUIRED_STRING \
		"The server does not accept saved user credentials and requires that the user enter their credentials for each connection."

#define ERRINFO_RPC_INITIATED_DISCONNECT_BY_USER_STRING \
		"The disconnection was initiated by an administrative tool on the server running in the user's session."

/* Protocol-independent licensing codes */

#define ERRINFO_LICENSE_INTERNAL_STRING \
		"An internal error has occurred in the Terminal Services licensing component."

#define ERRINFO_LICENSE_NO_LICENSE_SERVER_STRING \
		"A Remote Desktop License Server ([MS-RDPELE] section 1.1) could not be found to provide a license."

#define ERRINFO_LICENSE_NO_LICENSE_STRING \
		"There are no Client Access Licenses ([MS-RDPELE] section 1.1) available for the target remote computer."

#define ERRINFO_LICENSE_BAD_CLIENT_MSG_STRING \
		"The remote computer received an invalid licensing message from the client."

#define ERRINFO_LICENSE_HWID_DOESNT_MATCH_LICENSE_STRING \
		"The Client Access License ([MS-RDPELE] section 1.1) stored by the client has been modified."

#define ERRINFO_LICENSE_BAD_CLIENT_LICENSE_STRING \
		"The Client Access License ([MS-RDPELE] section 1.1) stored by the client is in an invalid format."

#define ERRINFO_LICENSE_CANT_FINISH_PROTOCOL_STRING \
		"Network problems have caused the licensing protocol ([MS-RDPELE] section 1.3.3) to be terminated."

#define ERRINFO_LICENSE_CLIENT_ENDED_PROTOCOL_STRING \
		"The client prematurely ended the licensing protocol ([MS-RDPELE] section 1.3.3)."

#define ERRINFO_LICENSE_BAD_CLIENT_ENCRYPTION_STRING \
		"A licensing message ([MS-RDPELE] sections 2.2 and 5.1) was incorrectly encrypted."

#define ERRINFO_LICENSE_CANT_UPGRADE_LICENSE_STRING \
		"The Client Access License ([MS-RDPELE] section 1.1) stored by the client could not be upgraded or renewed."

#define ERRINFO_LICENSE_NO_REMOTE_CONNECTIONS_STRING \
		"The remote computer is not licensed to accept remote connections."

/* RDP specific codes */

#define ERRINFO_UNKNOWN_DATA_PDU_TYPE_STRING \
		"Unknown pduType2 field in a received Share Data Header (section 2.2.8.1.1.1.2)."

#define ERRINFO_UNKNOWN_PDU_TYPE_STRING \
		"Unknown pduType field in a received Share Control Header (section 2.2.8.1.1.1.1)."

#define ERRINFO_DATA_PDU_SEQUENCE_STRING \
		"An out-of-sequence Slow-Path Data PDU (section 2.2.8.1.1.1.1) has been received."

#define ERRINFO_CONTROL_PDU_SEQUENCE_STRING \
		"An out-of-sequence Slow-Path Non-Data PDU (section 2.2.8.1.1.1.1) has been received."

#define ERRINFO_INVALID_CONTROL_PDU_ACTION_STRING \
		"A Control PDU (sections 2.2.1.15 and 2.2.1.16) has been received with an invalid action field."

#define ERRINFO_INVALID_INPUT_PDU_TYPE_STRING \
		"(a) A Slow-Path Input Event (section 2.2.8.1.1.3.1.1) has been received with an invalid messageType field.\n" \
		"(b) A Fast-Path Input Event (section 2.2.8.1.2.2) has been received with an invalid eventCode field."

#define ERRINFO_INVALID_INPUT_PDU_MOUSE_STRING \
		"(a) A Slow-Path Mouse Event (section 2.2.8.1.1.3.1.1.3) or Extended Mouse Event " \
		"(section 2.2.8.1.1.3.1.1.4) has been received with an invalid pointerFlags field.\n" \
		"(b) A Fast-Path Mouse Event (section 2.2.8.1.2.2.3) or Fast-Path Extended Mouse Event " \
		"(section 2.2.8.1.2.2.4) has been received with an invalid pointerFlags field."

#define ERRINFO_INVALID_REFRESH_RECT_PDU_STRING \
		"An invalid Refresh Rect PDU (section 2.2.11.2) has been received."

#define ERRINFO_CREATE_USER_DATA_FAILED_STRING \
		"The server failed to construct the GCC Conference Create Response user data (section 2.2.1.4)."

#define ERRINFO_CONNECT_FAILED_STRING \
		"Processing during the Channel Connection phase of the RDP Connection Sequence " \
		"(see section 1.3.1.1 for an overview of the RDP Connection Sequence phases) has failed."

#define ERRINFO_CONFIRM_ACTIVE_HAS_WRONG_SHAREID_STRING \
		"A Confirm Active PDU (section 2.2.1.13.2) was received from the client with an invalid shareId field."

#define ERRINFO_CONFIRM_ACTIVE_HAS_WRONG_ORIGINATOR_STRING \
		"A Confirm Active PDU (section 2.2.1.13.2) was received from the client with an invalid originatorId field."

#define ERRINFO_PERSISTENT_KEY_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process a Persistent Key List PDU (section 2.2.1.17)."

#define ERRINFO_PERSISTENT_KEY_PDU_ILLEGAL_FIRST_STRING \
		"A Persistent Key List PDU (section 2.2.1.17) marked as PERSIST_PDU_FIRST (0x01) was received after the reception " \
		"of a prior Persistent Key List PDU also marked as PERSIST_PDU_FIRST."

#define ERRINFO_PERSISTENT_KEY_PDU_TOO_MANY_TOTAL_KEYS_STRING \
		"A Persistent Key List PDU (section 2.2.1.17) was received which specified a total number of bitmap cache entries larger than 262144."

#define ERRINFO_PERSISTENT_KEY_PDU_TOO_MANY_CACHE_KEYS_STRING \
		"A Persistent Key List PDU (section 2.2.1.17) was received which specified an invalid total number of keys for a bitmap cache " \
		"(the number of entries that can be stored within each bitmap cache is specified in the Revision 1 or 2 Bitmap Cache Capability Set " \
		"(section 2.2.7.1.4) that is sent from client to server)."

#define ERRINFO_INPUT_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process Input Event PDU Data (section 2.2.8.1.1.3.1) or a Fast-Path Input Event PDU (section 2.2.8.1.2)." \

#define ERRINFO_BITMAP_CACHE_ERROR_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process the shareDataHeader, NumInfoBlocks, " \
		"Pad1, and Pad2 fields of the Bitmap Cache Error PDU Data ([MS-RDPEGDI] section 2.2.2.3.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT_STRING \
		"(a) The dataSignature field of the Fast-Path Input Event PDU (section 2.2.8.1.2) does not contain enough data.\n" \
		"(b) The fipsInformation and dataSignature fields of the Fast-Path Input Event PDU (section 2.2.8.1.2) do not contain enough data."

#define ERRINFO_VCHANNEL_DATA_TOO_SHORT_STRING \
		"(a) There is not enough data in the Client Network Data (section 2.2.1.3.4) to read the virtual channel configuration data.\n" \
		"(b) There is not enough data to read a complete Channel PDU Header (section 2.2.6.1.1)."

#define ERRINFO_SHARE_DATA_TOO_SHORT_STRING \
		"(a) There is not enough data to process Control PDU Data (section 2.2.1.15.1).\n" \
		"(b) There is not enough data to read a complete Share Control Header (section 2.2.8.1.1.1.1).\n" \
		"(c) There is not enough data to read a complete Share Data Header (section 2.2.8.1.1.1.2) of a Slow-Path Data PDU (section 2.2.8.1.1.1.1).\n" \
		"(d) There is not enough data to process Font List PDU Data (section 2.2.1.18.1)."

#define ERRINFO_BAD_SUPPRESS_OUTPUT_PDU_STRING \
		"(a) There is not enough data to process Suppress Output PDU Data (section 2.2.11.3.1).\n" \
		"(b) The allowDisplayUpdates field of the Suppress Output PDU Data (section 2.2.11.3.1) is invalid."

#define ERRINFO_CONFIRM_ACTIVE_PDU_TOO_SHORT_STRING \
		"(a) There is not enough data to read the shareControlHeader, shareId, originatorId, lengthSourceDescriptor, " \
		"and lengthCombinedCapabilities fields of the Confirm Active PDU Data (section 2.2.1.13.2.1).\n" \
		"(b) There is not enough data to read the sourceDescriptor, numberCapabilities, pad2Octets, and capabilitySets " \
		"fields of the Confirm Active PDU Data (section 2.2.1.13.2.1)."

#define ERRINFO_CAPABILITY_SET_TOO_SMALL_STRING \
		"There is not enough data to read the capabilitySetType and the lengthCapability fields in a received Capability Set (section 2.2.1.13.1.1.1)."

#define ERRINFO_CAPABILITY_SET_TOO_LARGE_STRING \
		"A Capability Set (section 2.2.1.13.1.1.1) has been received with a lengthCapability " \
		"field that contains a value greater than the total length of the data received."

#define ERRINFO_NO_CURSOR_CACHE_STRING \
		"(a) Both the colorPointerCacheSize and pointerCacheSize fields in the Pointer Capability Set (section 2.2.7.1.5) are set to zero.\n" \
		"(b) The pointerCacheSize field in the Pointer Capability Set (section 2.2.7.1.5) is not present, and the colorPointerCacheSize field is set to zero."

#define ERRINFO_BAD_CAPABILITIES_STRING \
		"The capabilities received from the client in the Confirm Active PDU (section 2.2.1.13.2) were not accepted by the server."

#define ERRINFO_VIRTUAL_CHANNEL_DECOMPRESSION_STRING \
		"An error occurred while using the bulk compressor (section 3.1.8 and [MS-RDPEGDI] section 3.1.8) to decompress a Virtual Channel PDU (section 2.2.6.1)"

#define ERRINFO_INVALID_VC_COMPRESSION_TYPE_STRING \
		"An invalid bulk compression package was specified in the flags field of the Channel PDU Header (section 2.2.6.1.1)."

#define ERRINFO_INVALID_CHANNEL_ID_STRING \
		"An invalid MCS channel ID was specified in the mcsPdu field of the Virtual Channel PDU (section 2.2.6.1)."

#define ERRINFO_VCHANNELS_TOO_MANY_STRING \
		"The client requested more than the maximum allowed 31 static virtual channels in the Client Network Data (section 2.2.1.3.4)."

#define ERRINFO_REMOTEAPP_NOT_ENABLED_STRING \
		"The INFO_RAIL flag (0x00008000) MUST be set in the flags field of the Info Packet (section 2.2.1.11.1.1) " \
		"as the session on the remote server can only host remote applications."

#define ERRINFO_CACHE_CAP_NOT_SET_STRING \
		"The client sent a Persistent Key List PDU (section 2.2.1.17) without including the prerequisite Revision 2 Bitmap Cache " \
		"Capability Set (section 2.2.7.1.4.2) in the Confirm Active PDU (section 2.2.1.13.2)."

#define ERRINFO_BITMAP_CACHE_ERROR_PDU_BAD_LENGTH2_STRING \
		"The NumInfoBlocks field in the Bitmap Cache Error PDU Data is inconsistent with the amount of data in the " \
		"Info field ([MS-RDPEGDI] section 2.2.2.3.1.1)."

#define ERRINFO_OFFSCREEN_CACHE_ERROR_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process an Offscreen Bitmap Cache Error PDU ([MS-RDPEGDI] section 2.2.2.3.2)."

#define ERRINFO_DRAWNINEGRID_CACHE_ERROR_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process a DrawNineGrid Cache Error PDU ([MS-RDPEGDI] section 2.2.2.3.3)."

#define ERRINFO_GDIPLUS_PDU_BAD_LENGTH_STRING \
		"There is not enough data to process a GDI+ Error PDU ([MS-RDPEGDI] section 2.2.2.3.4)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT2_STRING \
		"There is not enough data to read a Basic Security Header (section 2.2.8.1.1.2.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT3_STRING \
		"There is not enough data to read a Non-FIPS Security Header (section 2.2.8.1.1.2.2) or FIPS Security Header (section 2.2.8.1.1.2.3)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT4_STRING \
		"There is not enough data to read the basicSecurityHeader and length fields of the Security Exchange PDU Data (section 2.2.1.10.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT5_STRING \
		"There is not enough data to read the CodePage, flags, cbDomain, cbUserName, cbPassword, cbAlternateShell, " \
		"cbWorkingDir, Domain, UserName, Password, AlternateShell, and WorkingDir fields in the Info Packet (section 2.2.1.11.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT6_STRING \
		"There is not enough data to read the CodePage, flags, cbDomain, cbUserName, cbPassword, cbAlternateShell, " \
		"and cbWorkingDir fields in the Info Packet (section 2.2.1.11.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT7_STRING \
		"There is not enough data to read the clientAddressFamily and cbClientAddress fields in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT8_STRING \
		"There is not enough data to read the clientAddress field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT9_STRING \
		"There is not enough data to read the cbClientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT10_STRING \
		"There is not enough data to read the clientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT11_STRING \
		"There is not enough data to read the clientTimeZone field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT12_STRING \
		"There is not enough data to read the clientSessionId field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT13_STRING \
		"There is not enough data to read the performanceFlags field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT14_STRING \
		"There is not enough data to read the cbAutoReconnectLen field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT15_STRING \
		"There is not enough data to read the autoReconnectCookie field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT16_STRING \
		"The cbAutoReconnectLen field in the Extended Info Packet (section 2.2.1.11.1.1.1) contains a value " \
		"which is larger than the maximum allowed length of 128 bytes."

#define ERRINFO_SECURITY_DATA_TOO_SHORT17_STRING \
		"There is not enough data to read the clientAddressFamily and cbClientAddress fields in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT18_STRING \
		"There is not enough data to read the clientAddress field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT19_STRING \
		"There is not enough data to read the cbClientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT20_STRING \
		"There is not enough data to read the clientDir field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT21_STRING \
		"There is not enough data to read the clientTimeZone field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT22_STRING \
		"There is not enough data to read the clientSessionId field in the Extended Info Packet (section 2.2.1.11.1.1.1)."

#define ERRINFO_SECURITY_DATA_TOO_SHORT23_STRING \
		"There is not enough data to read the Client Info PDU Data (section 2.2.1.11.1)."

#define ERRINFO_BAD_MONITOR_DATA_STRING \
		"The monitorCount field in the Client Monitor Data (section 2.2.1.3.6) is invalid."

#define ERRINFO_VC_DECOMPRESSED_REASSEMBLE_FAILED_STRING \
		"The server-side decompression buffer is invalid, or the size of the decompressed VC data exceeds " \
		"the chunking size specified in the Virtual Channel Capability Set (section 2.2.7.1.10)."

#define ERRINFO_VC_DATA_TOO_LONG_STRING \
		"The size of a received Virtual Channel PDU (section 2.2.6.1) exceeds the chunking size specified " \
		"in the Virtual Channel Capability Set (section 2.2.7.1.10)."

#define ERRINFO_GRAPHICS_MODE_NOT_SUPPORTED_STRING \
		"The graphics mode requested by the client is not supported by the server."

#define ERRINFO_GRAPHICS_SUBSYSTEM_RESET_FAILED_STRING \
		"The server-side graphics subsystem failed to reset."

#define ERRINFO_UPDATE_SESSION_KEY_FAILED_STRING \
		"An attempt to update the session keys while using Standard RDP Security mechanisms (section 5.3.7) failed."

#define ERRINFO_DECRYPT_FAILED_STRING \
		"(a) Decryption using Standard RDP Security mechanisms (section 5.3.6) failed.\n" \
		"(b) Session key creation using Standard RDP Security mechanisms (section 5.3.5) failed."

#define ERRINFO_ENCRYPT_FAILED_STRING \
		"Encryption using Standard RDP Security mechanisms (section 5.3.6) failed."

#define ERRINFO_ENCRYPTION_PACKAGE_MISMATCH_STRING \
		"Failed to find a usable Encryption Method (section 5.3.2) in the encryptionMethods field of the Client Security Data (section 2.2.1.4.3)."

#define ERRINFO_DECRYPT_FAILED2_STRING \
		"Unencrypted data was encountered in a protocol stream which is meant to be encrypted with Standard RDP Security mechanisms (section 5.3.6)."

/* Special codes */
#define ERRINFO_SUCCESS_STRING "Success."
#define ERRINFO_NONE_STRING ""

static const ERRINFO ERRINFO_CODES[] =
{
		ERRINFO_DEFINE(SUCCESS),

		/* Protocol-independent codes */
		ERRINFO_DEFINE(RPC_INITIATED_DISCONNECT),
		ERRINFO_DEFINE(RPC_INITIATED_LOGOFF),
		ERRINFO_DEFINE(IDLE_TIMEOUT),
		ERRINFO_DEFINE(LOGON_TIMEOUT),
		ERRINFO_DEFINE(DISCONNECTED_BY_OTHER_CONNECTION),
		ERRINFO_DEFINE(OUT_OF_MEMORY),
		ERRINFO_DEFINE(SERVER_DENIED_CONNECTION),
		ERRINFO_DEFINE(SERVER_INSUFFICIENT_PRIVILEGES),
		ERRINFO_DEFINE(SERVER_FRESH_CREDENTIALS_REQUIRED),
		ERRINFO_DEFINE(RPC_INITIATED_DISCONNECT_BY_USER),

		/* Protocol-independent licensing codes */
		ERRINFO_DEFINE(LICENSE_INTERNAL),
		ERRINFO_DEFINE(LICENSE_NO_LICENSE_SERVER),
		ERRINFO_DEFINE(LICENSE_NO_LICENSE),
		ERRINFO_DEFINE(LICENSE_BAD_CLIENT_MSG),
		ERRINFO_DEFINE(LICENSE_HWID_DOESNT_MATCH_LICENSE),
		ERRINFO_DEFINE(LICENSE_BAD_CLIENT_LICENSE),
		ERRINFO_DEFINE(LICENSE_CANT_FINISH_PROTOCOL),
		ERRINFO_DEFINE(LICENSE_CLIENT_ENDED_PROTOCOL),
		ERRINFO_DEFINE(LICENSE_BAD_CLIENT_ENCRYPTION),
		ERRINFO_DEFINE(LICENSE_CANT_UPGRADE_LICENSE),
		ERRINFO_DEFINE(LICENSE_NO_REMOTE_CONNECTIONS),

		/* RDP specific codes */
		ERRINFO_DEFINE(UNKNOWN_DATA_PDU_TYPE),
		ERRINFO_DEFINE(UNKNOWN_PDU_TYPE),
		ERRINFO_DEFINE(DATA_PDU_SEQUENCE),
		ERRINFO_DEFINE(CONTROL_PDU_SEQUENCE),
		ERRINFO_DEFINE(INVALID_CONTROL_PDU_ACTION),
		ERRINFO_DEFINE(INVALID_INPUT_PDU_TYPE),
		ERRINFO_DEFINE(INVALID_INPUT_PDU_MOUSE),
		ERRINFO_DEFINE(INVALID_REFRESH_RECT_PDU),
		ERRINFO_DEFINE(CREATE_USER_DATA_FAILED),
		ERRINFO_DEFINE(CONNECT_FAILED),
		ERRINFO_DEFINE(CONFIRM_ACTIVE_HAS_WRONG_SHAREID),
		ERRINFO_DEFINE(CONFIRM_ACTIVE_HAS_WRONG_ORIGINATOR),
		ERRINFO_DEFINE(PERSISTENT_KEY_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(PERSISTENT_KEY_PDU_ILLEGAL_FIRST),
		ERRINFO_DEFINE(PERSISTENT_KEY_PDU_TOO_MANY_TOTAL_KEYS),
		ERRINFO_DEFINE(PERSISTENT_KEY_PDU_TOO_MANY_CACHE_KEYS),
		ERRINFO_DEFINE(INPUT_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(BITMAP_CACHE_ERROR_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT),
		ERRINFO_DEFINE(VCHANNEL_DATA_TOO_SHORT	),
		ERRINFO_DEFINE(SHARE_DATA_TOO_SHORT),
		ERRINFO_DEFINE(BAD_SUPPRESS_OUTPUT_PDU),
		ERRINFO_DEFINE(CONFIRM_ACTIVE_PDU_TOO_SHORT),
		ERRINFO_DEFINE(CAPABILITY_SET_TOO_SMALL),
		ERRINFO_DEFINE(CAPABILITY_SET_TOO_LARGE),
		ERRINFO_DEFINE(NO_CURSOR_CACHE),
		ERRINFO_DEFINE(BAD_CAPABILITIES),
		ERRINFO_DEFINE(VIRTUAL_CHANNEL_DECOMPRESSION),
		ERRINFO_DEFINE(INVALID_VC_COMPRESSION_TYPE),
		ERRINFO_DEFINE(INVALID_CHANNEL_ID),
		ERRINFO_DEFINE(VCHANNELS_TOO_MANY),
		ERRINFO_DEFINE(REMOTEAPP_NOT_ENABLED),
		ERRINFO_DEFINE(CACHE_CAP_NOT_SET),
		ERRINFO_DEFINE(BITMAP_CACHE_ERROR_PDU_BAD_LENGTH2),
		ERRINFO_DEFINE(OFFSCREEN_CACHE_ERROR_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(DRAWNINEGRID_CACHE_ERROR_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(GDIPLUS_PDU_BAD_LENGTH),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT2),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT3),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT4),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT5),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT6),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT7),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT8),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT9),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT10),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT11),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT12),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT13),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT14),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT15),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT16),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT17),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT18),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT19),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT20),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT21),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT22),
		ERRINFO_DEFINE(SECURITY_DATA_TOO_SHORT23),
		ERRINFO_DEFINE(BAD_MONITOR_DATA),
		ERRINFO_DEFINE(VC_DECOMPRESSED_REASSEMBLE_FAILED),
		ERRINFO_DEFINE(VC_DATA_TOO_LONG),
		ERRINFO_DEFINE(GRAPHICS_MODE_NOT_SUPPORTED),
		ERRINFO_DEFINE(GRAPHICS_SUBSYSTEM_RESET_FAILED),
		ERRINFO_DEFINE(UPDATE_SESSION_KEY_FAILED),
		ERRINFO_DEFINE(DECRYPT_FAILED),
		ERRINFO_DEFINE(ENCRYPT_FAILED),
		ERRINFO_DEFINE(ENCRYPTION_PACKAGE_MISMATCH),
		ERRINFO_DEFINE(DECRYPT_FAILED2),

		ERRINFO_DEFINE(NONE)
};

void rdp_print_errinfo(uint32 code)
{
	const ERRINFO* errInfo;

	errInfo = &ERRINFO_CODES[0];

	while (errInfo->code != ERRINFO_NONE)
	{
		if (code == errInfo->code)
		{
			printf("%s (0x%08X):\n%s\n", errInfo->name, code, errInfo->info);
			return;
		}

		errInfo++;
	}

	printf("ERRINFO_UNKNOWN 0x%08X: Unknown error.\n", code);
}

/* Modeline for vim. Don't delete */
/* vim: set cindent:noet:sw=8:ts=8 */
