/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Static Entry Point Tables
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

#include <freerdp/svc.h>

/* The 'entry' function pointers have variable arguments. */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-prototypes"
#endif
struct s_STATIC_ENTRY
{
	const char* name;
	UINT (*entry)();
};
typedef struct s_STATIC_ENTRY STATIC_ENTRY;

struct s_STATIC_ENTRY_TABLE
{
	const char* name;
	const STATIC_ENTRY* table;
};
typedef struct s_STATIC_ENTRY_TABLE STATIC_ENTRY_TABLE;

struct s_STATIC_SUBSYSTEM_ENTRY
{
	const char* name;
	const char* type;
	UINT (*entry)();
};
typedef struct s_STATIC_SUBSYSTEM_ENTRY STATIC_SUBSYSTEM_ENTRY;

struct s_STATIC_ADDIN_TABLE
{
	const char* name;
	const char* type;
	UINT (*entry)();
	const STATIC_SUBSYSTEM_ENTRY* table;
};
typedef struct s_STATIC_ADDIN_TABLE STATIC_ADDIN_TABLE;

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
