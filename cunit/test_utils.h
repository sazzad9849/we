/**
 * FreeRDP: A Remote Desktop Protocol Client
 * Utils Unit Tests
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

#include "test_freerdp.h"

int init_utils_suite(void);
int clean_utils_suite(void);
int add_utils_suite(void);

void test_mutex(void);
void test_semaphore(void);
void test_load_plugin(void);
void test_wait_obj(void);
void test_args(void);
void test_passphrase_read(void);
void test_handle_signals(void);
/* Modeline for vim. Don't delete */
/* vim: cindent:noet:sw=8:ts=8 */
