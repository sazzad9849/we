/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Popup browser for AAD authentication
 *
 * Copyright 2023 Isaac Klein <fifthdegree@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <freerdp/freerdp.h>

#ifdef __cplusplus
extern "C"
{
#endif

	BOOL sdl_webview_get_avd_access_token(freerdp* instance, char** token);
	BOOL sdl_webview_get_rdsaad_access_token(freerdp* instance, const char* scope,
	                                         const char* req_cnf, char** token);

#ifdef __cplusplus
}
#endif
