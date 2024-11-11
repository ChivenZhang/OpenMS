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
#include "IChannelReactor.h"
#include "Channel.h"
#include "ChannelWorker.h"

class ChannelReactor : public IChannelReactor
{
public:
	ChannelReactor(size_t workerNum, callback_t callback);
	~ChannelReactor();
	void startup() override;
	void shutdown() override;
	bool running() const;

protected:
	void onConnect(TRef<Channel> channel);
	void onDisconnect(TRef<Channel> channel);
	void onInbound(TRef<IChannelEvent> event);
	void onOutbound(TRef<IChannelEvent> event, bool flush = false);

protected:
	callback_t m_Callback;
	TThread m_EventThread;
	TAtomic<bool> m_Running;
	TVector<TThread> m_WorkerThreads;
	TVector<TRef<ChannelWorker>> m_WorkerList;
	TMutex m_EventLock;
	TQueue<TRef<IChannelEvent>> m_EventQueue;

private:
	friend class Channel;
};