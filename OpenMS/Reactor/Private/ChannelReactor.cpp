#include "ChannelReactor.h"
#include "ChannelReactor.h"
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
#include "ChannelReactor.h"

ChannelReactor::ChannelReactor(size_t workerNum, callback_t callback)
	:
	m_Running(false),
	m_Sending(false),
	m_Connect(false),
	m_OnOpen(callback.OnOpen),
	m_OnClose(callback.OnClose),
	m_WorkerList(std::max<size_t>(1, workerNum)),
	m_WorkerThreads(std::max<size_t>(1, workerNum))
{
}

ChannelReactor::~ChannelReactor()
{
	shutdown();
}

void ChannelReactor::startup()
{
	m_Running = true;
	for (size_t i = 0; i < m_WorkerList.size(); ++i) m_WorkerList[i] = MSNew<ChannelWorker>(this);
	for (size_t i = 0; i < m_WorkerList.size(); ++i) m_WorkerThreads[i] = MSThread([=]() { m_WorkerList[i]->startup(); });
	for (size_t i = 0; i < m_WorkerList.size(); ++i) while (m_WorkerList[i]->running() == false);

#if 0 // test code
	m_EventThread = MSThread([=]() {
		auto channel = MSNew<Channel>(this, nullptr, nullptr);
		onConnect(channel);
		for (size_t i = 0; i < 10; ++i)
		{
			auto event = MSNew<IChannelEvent>(IChannelEvent { "A message!", channel });
			onInbound(event);
		}
		onDisconnect(channel);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		});
#endif
}

void ChannelReactor::shutdown()
{
	m_Running = false;
	for (size_t i = 0; i < m_WorkerList.size(); ++i) m_WorkerList[i]->shutdown();
	for (size_t i = 0; i < m_WorkerList.size(); ++i) if (m_WorkerThreads[i].joinable()) m_WorkerThreads[i].join();
	if (m_EventThread.joinable()) m_EventThread.join();
}

bool ChannelReactor::running() const
{
	return m_Running;
}

bool ChannelReactor::connect() const
{
	return m_Connect;
}

MSHnd<IChannelAddress> ChannelReactor::address() const
{
	return MSHnd<IChannelAddress>();
}

void ChannelReactor::write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address)
{
}

MSFuture<bool> ChannelReactor::write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address, MSPromise<bool>&& promise)
{
	if (m_Running == false) return MSFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	write(event, address);
	return result;
}

void ChannelReactor::onConnect(MSRef<Channel> channel)
{
	if (channel == nullptr) return;
	if (m_OnOpen) m_OnOpen(channel);
}

void ChannelReactor::onDisconnect(MSRef<Channel> channel)
{
	if (channel == nullptr) return;
	channel->close();
	if (m_OnClose) m_OnClose(channel);
}

void ChannelReactor::onInbound(MSRef<IChannelEvent> event)
{
	auto channel = event->Channel.lock();
	if (event == nullptr || channel == nullptr) return;
	m_WorkerList[channel->getWorkID()]->enqueue(event);
}

void ChannelReactor::onOutbound(MSRef<IChannelEvent> event, bool flush)
{
	if (event == nullptr || event->Channel.expired()) return;
	MSMutexLock lock(m_EventLock);
	m_EventQueue.push(event);
	m_Sending = true;
}
