/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "ChannelWorker.h"
#include "ChannelReactor.h"

ChannelWorker::ChannelWorker(MSRaw<ChannelReactor> reactor)
	:
	m_Reactor(reactor)
{
}

void ChannelWorker::startup()
{
	MS_DEBUG("startup worker");

	m_Running = true;
	while (m_Running == true && m_Reactor->running())
	{
		MSUniqueLock lock(m_EventLock);
		m_EventUnlock.wait(lock, [=]()
		{
			return m_EventQueue.size() || m_Running == false || m_Reactor->running() == false;
		});

		while (m_EventQueue.empty() == false && m_Running == true && m_Reactor->running() == true)
		{
			auto event = m_EventQueue.front();
			m_EventQueue.pop();
			auto channel = event->Channel.lock();
			if (channel) channel->readChannel(event);
		}
	}

	MS_DEBUG("shutdown worker");
}

void ChannelWorker::shutdown()
{
	m_Running = false;
	m_EventUnlock.notify_one();
}

bool ChannelWorker::running() const
{
	return m_Running;
}

void ChannelWorker::enqueue(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	MSMutexLock lock(m_EventLock);
	m_EventQueue.push(event);
	m_EventUnlock.notify_one();
}
