#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "../IChannelWorker.h"
class ChannelReactor;

class ChannelWorker : public IChannelWorker
{
public:
	ChannelWorker(MSRaw<ChannelReactor> reactor);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	void enqueue(MSRef<IChannelEvent> event) override;

protected:
	MSMutex m_EventLock;
	MSMutexUnlock m_EventUnlock;
	MSAtomic<bool> m_Running;
	MSRaw<ChannelReactor> m_Reactor;
	MSQueue<MSRef<IChannelEvent>> m_EventQueue;
};