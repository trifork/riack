/*
   Copyright 2012 Trifork A/S
   Author: Kaspar Pedersen

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef RIACK_MSG_H_
#define RIACK_MSG_H_

#include "riack_defines.h"

struct RIACK_PB_MSG
{
	uint8_t  msg_code;
	uint32_t msg_len;
	uint8_t* msg;
};

void dbg_print_message(struct RIACK_PB_MSG* pmsg);

int riack_send_message(struct RIACK_CLIENT *client, struct RIACK_PB_MSG* msg);
int riack_receive_message(struct RIACK_CLIENT *client, struct RIACK_PB_MSG** msg);

void riack_message_free(struct RIACK_CLIENT *client, struct RIACK_PB_MSG** ppMsg);

#endif /* RIACK_MSG_H_ */
