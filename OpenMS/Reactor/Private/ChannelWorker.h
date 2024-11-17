#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../IChannelWorker.h"
class ChannelReactor;

class ChannelWorker : public IChannelWorker
{
public:
	ChannelWorker(TRaw<ChannelReactor> reactor);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	void enqueue(TRef<IChannelEvent> event) override;

protected:
	TMutex m_EventLock;
	TMutexUnlock m_EventUnlock;
	TAtomic<bool> m_Running;
	TRaw<ChannelReactor> m_Reactor;
	TQueue<TRef<IChannelEvent>> m_EventQueue;
};