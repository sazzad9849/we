/**
 * FreeRDP: A Remote Desktop Protocol Client
 * Input Interface API
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

#ifndef __INPUT_API_H
#define __INPUT_API_H

typedef struct rdp_input rdpInput;

#include <freerdp/api.h>
#include <freerdp/freerdp.h>
#include <freerdp/keyboard_scancode.h>

/* keyboard Flags */
#define KBD_FLAGS_EXTENDED		0x0100
#define KBD_FLAGS_DOWN			0x4000
#define KBD_FLAGS_RELEASE		0x8000

/* Pointer Flags */
#define PTR_FLAGS_WHEEL			0x0200
#define PTR_FLAGS_WHEEL_NEGATIVE	0x0100
#define PTR_FLAGS_MOVE			0x0800
#define PTR_FLAGS_DOWN			0x8000
#define PTR_FLAGS_BUTTON1		0x1000
#define PTR_FLAGS_BUTTON2		0x2000
#define PTR_FLAGS_BUTTON3		0x4000
#define WheelRotationMask		0x01FF

/* Extended Pointer Flags */
#define PTR_XFLAGS_DOWN			0x8000
#define PTR_XFLAGS_BUTTON1		0x0001
#define PTR_XFLAGS_BUTTON2		0x0002

/* Keyboard Toggle Flags */
#define KBD_SYNC_SCROLL_LOCK		0x00000001
#define KBD_SYNC_NUM_LOCK		0x00000002
#define KBD_SYNC_CAPS_LOCK		0x00000004
#define KBD_SYNC_KANA_LOCK		0x00000008

#define RDP_CLIENT_INPUT_PDU_HEADER_LENGTH	4

typedef void (*pSynchronizeEvent)(rdpInput* input, uint32 flags);
typedef void (*pKeyboardEvent)(rdpInput* input, uint16 flags, uint16 code);
typedef void (*pUnicodeKeyboardEvent)(rdpInput* input, uint16 flags, uint16 code);
typedef void (*pMouseEvent)(rdpInput* input, uint16 flags, uint16 x, uint16 y);
typedef void (*pExtendedMouseEvent)(rdpInput* input, uint16 flags, uint16 x, uint16 y);

struct rdp_input
{
	rdpContext* context; /* 0 */
	void* param1; /* 1 */
	uint32 paddingA[16 - 2]; /* 2 */

	pSynchronizeEvent SynchronizeEvent; /* 16 */
	pKeyboardEvent KeyboardEvent; /* 17 */
	pUnicodeKeyboardEvent UnicodeKeyboardEvent; /* 18 */
	pMouseEvent MouseEvent; /* 19 */
	pExtendedMouseEvent ExtendedMouseEvent; /* 20 */
	uint32 paddingB[32 - 21]; /* 21 */
};

FREERDP_API void freerdp_input_send_synchronize_event(rdpInput* input, uint32 flags);
FREERDP_API void freerdp_input_send_keyboard_event(rdpInput* input, uint16 flags, uint16 code);
FREERDP_API void freerdp_input_send_unicode_keyboard_event(rdpInput* input, uint16 flags, uint16 code);
FREERDP_API void freerdp_input_send_mouse_event(rdpInput* input, uint16 flags, uint16 x, uint16 y);
FREERDP_API void freerdp_input_send_extended_mouse_event(rdpInput* input, uint16 flags, uint16 x, uint16 y);

#define freerdp_input_send_keyboard_event_2(input, down, rdp_scancode) \
		freerdp_input_send_keyboard_event(input, \
			(rdp_scancode_extended(rdp_scancode) ? KBD_FLAGS_EXTENDED : 0) | \
			((down) ? KBD_FLAGS_DOWN : KBD_FLAGS_RELEASE), \
			rdp_scancode_code(rdp_scancode))

#endif /* __INPUT_API_H */
/* Modeline for vim. Don't delete */
/* vim: set cindent:noet:sw=8:ts=8 */
