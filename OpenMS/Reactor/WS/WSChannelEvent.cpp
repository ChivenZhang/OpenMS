/*=================================================
* Copyright © 2020-2026 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "WSChannelEvent.h"

MSRef<WSChannelEvent> WSChannelEvent::New(MSStringView const& message, opcode_t opcode, MSHnd<IChannel> const& channel)
{
	auto event = MSNew<WSChannelEvent>();
	event->Message = message;
	event->Channel = channel;
	event->OpCode = opcode;
	return event;
}
