/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Remote Applications Integrated Locally (RAIL)
 *
 * Copyright 2013 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FREERDP_CHANNEL_RAIL_CLIENT_RAIL_H
#define FREERDP_CHANNEL_RAIL_CLIENT_RAIL_H

#include <freerdp/api.h>
#include <freerdp/types.h>

#include <freerdp/rail.h>
#include <freerdp/message.h>
#include <freerdp/channels/rail.h>

/**
 * Client Interface
 */

#define RAIL_SVC_CHANNEL_NAME	"rail"

typedef struct _rail_client_context RailClientContext;

typedef UINT(*pcRailClientExecute)(RailClientContext* context, RAIL_EXEC_ORDER* exec);
typedef UINT(*pcRailClientActivate)(RailClientContext* context,
                                    const RAIL_ACTIVATE_ORDER* activate);
typedef UINT(*pcRailClientSystemParam)(RailClientContext* context,
                                       const RAIL_SYSPARAM_ORDER* sysparam);
typedef UINT(*pcRailServerSystemParam)(RailClientContext* context,
                                       const RAIL_SYSPARAM_ORDER* sysparam);
typedef UINT(*pcRailClientSystemCommand)(RailClientContext* context,
        const RAIL_SYSCOMMAND_ORDER* syscommand);
typedef UINT(*pcRailClientHandshake)(RailClientContext* context,
                                     const RAIL_HANDSHAKE_ORDER* handshake);
typedef UINT(*pcRailServerHandshake)(RailClientContext* context,
                                     const RAIL_HANDSHAKE_ORDER* handshake);
typedef UINT(*pcRailClientHandshakeEx)(RailClientContext* context,
                                       const RAIL_HANDSHAKE_EX_ORDER* handshakeEx);
typedef UINT(*pcRailServerHandshakeEx)(RailClientContext* context,
                                       const RAIL_HANDSHAKE_EX_ORDER* handshakeEx);
typedef UINT(*pcRailClientNotifyEvent)(RailClientContext* context,
                                       const RAIL_NOTIFY_EVENT_ORDER* notifyEvent);
typedef UINT(*pcRailClientWindowMove)(RailClientContext* context,
                                      const RAIL_WINDOW_MOVE_ORDER* windowMove);
typedef UINT(*pcRailServerLocalMoveSize)(RailClientContext* context,
        const RAIL_LOCALMOVESIZE_ORDER* localMoveSize);
typedef UINT(*pcRailServerMinMaxInfo)(RailClientContext* context,
                                      const RAIL_MINMAXINFO_ORDER* minMaxInfo);
typedef UINT(*pcRailClientInformation)(RailClientContext* context,
                                       const RAIL_CLIENT_STATUS_ORDER* clientStatus);
typedef UINT(*pcRailClientSystemMenu)(RailClientContext* context,
                                      const RAIL_SYSMENU_ORDER* sysmenu);
typedef UINT(*pcRailClientLanguageBarInfo)(RailClientContext* context,
        const RAIL_LANGBAR_INFO_ORDER* langBarInfo);
typedef UINT(*pcRailServerLanguageBarInfo)(RailClientContext* context,
        const RAIL_LANGBAR_INFO_ORDER* langBarInfo);
typedef UINT(*pcRailServerExecuteResult)(RailClientContext* context,
        const RAIL_EXEC_RESULT_ORDER* execResult);
typedef UINT(*pcRailClientGetAppIdRequest)(RailClientContext* context,
        const RAIL_GET_APPID_REQ_ORDER* getAppIdReq);
typedef UINT(*pcRailServerGetAppIdResponse)(RailClientContext* context,
        const RAIL_GET_APPID_RESP_ORDER* getAppIdResp);

struct _rail_client_context
{
	void* handle;
	void* custom;

	pcRailClientExecute ClientExecute;
	pcRailClientActivate ClientActivate;
	pcRailClientSystemParam ClientSystemParam;
	pcRailServerSystemParam ServerSystemParam;
	pcRailClientSystemCommand ClientSystemCommand;
	pcRailClientHandshake ClientHandshake;
	pcRailServerHandshake ServerHandshake;
	pcRailClientHandshakeEx ClientHandshakeEx;
	pcRailServerHandshakeEx ServerHandshakeEx;
	pcRailClientNotifyEvent ClientNotifyEvent;
	pcRailClientWindowMove ClientWindowMove;
	pcRailServerLocalMoveSize ServerLocalMoveSize;
	pcRailServerMinMaxInfo ServerMinMaxInfo;
	pcRailClientInformation ClientInformation;
	pcRailClientSystemMenu ClientSystemMenu;
	pcRailClientLanguageBarInfo ClientLanguageBarInfo;
	pcRailServerLanguageBarInfo ServerLanguageBarInfo;
	pcRailServerExecuteResult ServerExecuteResult;
	pcRailClientGetAppIdRequest ClientGetAppIdRequest;
	pcRailServerGetAppIdResponse ServerGetAppIdResponse;
};

#endif /* FREERDP_CHANNEL_RAIL_CLIENT_RAIL_H */

