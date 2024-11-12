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
#include "ChannelReactor.h"

ChannelReactor::ChannelReactor(size_t workerNum, callback_t callback)
	:
	m_Running(false),
	m_Callback(callback),
	m_WorkerList(std::max(1ULL, workerNum)),
	m_WorkerThreads(std::max(1ULL, workerNum))
{
}

ChannelReactor::~ChannelReactor()
{
	this->shutdown();
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

void ChannelReactor::onConnect(TRef<Channel> channel)
{
	if (m_Callback.Connected) m_Callback.Connected(channel);
}

void ChannelReactor::onDisconnect(TRef<Channel> channel)
{
	if (m_Callback.Disconnect) m_Callback.Disconnect(channel);
}

void ChannelReactor::onInbound(TRef<IChannelEvent> event)
{
	auto index = (rand() % m_WorkerList.size());
	if (event) m_WorkerList[index]->enqueue(event);
}

void ChannelReactor::onOutbound(TRef<IChannelEvent> event, bool flush)
{
	TMutexLock lock(m_EventLock);
	m_EventQueue.push(event);
	m_Sending = true;
}
