#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../IChannelContext.h"
class Channel;

class ChannelContext : public IChannelContext
{
public:
	explicit ChannelContext(MSRaw<Channel> channel);
	void close() override;
	MSFuture<bool> close(MSPromise<bool>& promise) override;
	void write(MSRef<IChannelEvent> event) override;
	MSFuture<bool> write(MSRef<IChannelEvent> event, MSPromise<bool>& promise) override;
	size_t& userdata() override;
	MSStringMap<MSAny>& attribs() override;

protected:
	size_t m_Userdata;
	MSRaw<Channel> m_Channel;
	MSStringMap<MSAny> m_AttribMap;
};