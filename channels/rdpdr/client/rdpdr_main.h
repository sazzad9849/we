/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Device Redirection Virtual Channel
 *
 * Copyright 2010-2011 Vic Lee
 * Copyright 2010-2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 * Copyright 2015 Thincast Technologies GmbH
 * Copyright 2015 DI (FH) Martin Haimberger <martin.haimberger@thincast.com>
 * Copyright 2016 Inuvika Inc.
 * Copyright 2016 David PHAM-VAN <d.phamvan@inuvika.com>
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

#ifndef FREERDP_CHANNEL_RDPDR_CLIENT_MAIN_H
#define FREERDP_CHANNEL_RDPDR_CLIENT_MAIN_H

#include <winpr/crt.h>
#include <winpr/synch.h>
#include <winpr/thread.h>
#include <winpr/stream.h>
#include <winpr/collections.h>

#include <freerdp/api.h>
#include <freerdp/svc.h>
#include <freerdp/addin.h>

#include <freerdp/channels/rdpdr.h>
#include <freerdp/channels/log.h>

#ifdef __MACOSX__
#include <CoreServices/CoreServices.h>
#endif

#define TAG CHANNELS_TAG("rdpdr.client")

typedef struct rdpdr_plugin rdpdrPlugin;

enum RDPDR_CHANNEL_STATE
{
	RDPDR_CHANNEL_STATE_INITIAL = 0,
	RDPDR_CHANNEL_STATE_ANNOUNCE,
	RDPDR_CHANNEL_STATE_ANNOUNCE_REPLY,
	RDPDR_CHANNEL_STATE_NAME_REQUEST,
	RDPDR_CHANNEL_STATE_SERVER_CAPS,
	RDPDR_CHANNEL_STATE_CLIENT_CAPS,
	RDPDR_CHANNEL_STATE_CLIENTID_CONFIRM,
	RDPDR_CHANNEL_STATE_READY,
	RDPDR_CHANNEL_STATE_USER_LOGGEDON
};

struct rdpdr_plugin
{
	CHANNEL_DEF channelDef;
	CHANNEL_ENTRY_POINTS_FREERDP_EX channelEntryPoints;

	enum RDPDR_CHANNEL_STATE state;
	HANDLE thread;
	wStream* data_in;
	void* InitHandle;
	DWORD OpenHandle;
	wMessageQueue* queue;

	DEVMAN* devman;

	UINT16 versionMajor;
	UINT16 versionMinor;
	UINT16 clientID;
	char computerName[256];

	UINT32 sequenceId;

	/* hotplug support */
	HANDLE hotplugThread;
#ifdef _WIN32
	HWND hotplug_wnd;
#endif
#ifdef __MACOSX__
	CFRunLoopRef runLoop;
#endif
#ifndef _WIN32
	HANDLE stopEvent;
#endif
	rdpContext* rdpcontext;
};

BOOL rdpdr_state_advance(rdpdrPlugin* rdpdr, enum RDPDR_CHANNEL_STATE next);
UINT rdpdr_send(rdpdrPlugin* rdpdr, wStream* s);

#endif /* FREERDP_CHANNEL_RDPDR_CLIENT_MAIN_H */
