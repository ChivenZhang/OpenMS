#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "IChannelContext.h"
class Channel;

class ChannelContext : public IChannelContext
{
public:
	ChannelContext(TRaw<Channel> channel);

protected:
	TRaw<Channel> m_Channel;
};