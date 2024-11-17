/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "ChannelWorker.h"
#include "ChannelReactor.h"

ChannelWorker::ChannelWorker(TRaw<ChannelReactor> reactor)
	:
	m_Reactor(reactor)
{
}

void ChannelWorker::startup()
{
	TDebug("Startup worker: %d", std::this_thread::get_id());

	m_Running = true;
	while (m_Running == true && m_Reactor->running())
	{
		TUniqueLock lock(m_EventLock);
		m_EventUnlock.wait(lock, [=]() {
			return m_EventQueue.size() || m_Running == false || m_Reactor->running() == false; });

		while (m_EventQueue.empty() == false && m_Running == true && m_Reactor->running() == true)
		{
			auto event = m_EventQueue.front();
			m_EventQueue.pop();
			auto channel = event->Channel.lock();
			if (channel) channel->read(event);
		}
	}

	TDebug("Shutdown worker: %d", std::this_thread::get_id());
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

void ChannelWorker::enqueue(TRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	TMutexLock lock(m_EventLock);
	m_EventQueue.push(event);
	m_EventUnlock.notify_one();
}
