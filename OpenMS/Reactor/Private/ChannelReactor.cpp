#include "ChannelReactor.h"
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
#include "ChannelReactor.h"

ChannelReactor::ChannelReactor(size_t workerNum, callback_t callback)
	:
	m_Running(false),
	m_Sending(false),
	m_Connect(false),
	m_OnConnected(callback.Connected),
	m_OnDisconnect(callback.Disconnect),
	m_WorkerList(std::max(1ULL, workerNum)),
	m_WorkerThreads(std::max(1ULL, workerNum))
{
}

ChannelReactor::~ChannelReactor()
{
	shutdown();
}

void ChannelReactor::startup()
{
	m_Running = true;
	for (size_t i = 0; i < m_WorkerList.size(); ++i) m_WorkerList[i] = TNew<ChannelWorker>(this);
	for (size_t i = 0; i < m_WorkerList.size(); ++i) m_WorkerThreads[i] = TThread([=]() { m_WorkerList[i]->startup(); });
	for (size_t i = 0; i < m_WorkerList.size(); ++i) while (m_WorkerList[i]->running() == false);

#if 0 // test code
	m_EventThread = TThread([=]() {
		auto channel = TNew<Channel>(this, nullptr, nullptr);
		onConnect(channel);
		for (size_t i = 0; i < 10; ++i)
		{
			auto event = TNew<IChannelEvent>(IChannelEvent { "A message!", channel });
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

void ChannelReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

TFuture<bool> ChannelReactor::write(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	write(event, address);
	return result;
}

void ChannelReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address)
{
}

TFuture<bool> ChannelReactor::writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	writeAndFlush(event, address);
	return result;
}

void ChannelReactor::onConnect(TRef<Channel> channel)
{
	if (m_OnConnected) m_OnConnected(channel);
}

void ChannelReactor::onDisconnect(TRef<Channel> channel)
{
	channel->close();
	if (m_OnDisconnect) m_OnDisconnect(channel);
}

void ChannelReactor::onInbound(TRef<IChannelEvent> event)
{
	auto channel = event->Channel.lock();
	if (event == nullptr || channel == nullptr) return;
	m_WorkerList[channel->getWorkID()]->enqueue(event);
}

void ChannelReactor::onOutbound(TRef<IChannelEvent> event, bool flush)
{
	if (event == nullptr || event->Channel.expired()) return;
	TMutexLock lock(m_EventLock);
	m_EventQueue.push(event);
	m_Sending = true;
}
