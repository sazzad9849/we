/**
 * FreeRDP: A Remote Desktop Protocol client.
 * RDP Server Listener
 *
 * Copyright 2011 Vic Lee
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

#ifndef __LISTENER_H
#define __LISTENER_H

typedef struct rdp_listener rdpListener;

#include "rdp.h"
#include <freerdp/listener.h>

struct rdp_listener
{
	freerdp_listener* instance;

 	int sockfds[5];
	int num_sockfds;
};

#endif

/* Modeline for vim. Don't delete */
/* vim: set cindent:noet:sw=8:ts=8 */
