/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Remote Assistance Virtual Channel
 *
 * Copyright 2014 Marc-Andre Moreau <marcandre.moreau@gmail.com>
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

#ifndef FREERDP_CHANNEL_REMDESK_CLIENT_REMDESK_H
#define FREERDP_CHANNEL_REMDESK_CLIENT_REMDESK_H

#include <freerdp/channels/remdesk.h>

/**
 * Client Interface
 */

typedef struct s_remdesk_client_context RemdeskClientContext;

struct s_remdesk_client_context
{
	void* handle;
	void* custom;
};

#endif /* FREERDP_CHANNEL_REMDESK_CLIENT_REMDESK_H */
