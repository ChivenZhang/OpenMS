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
#include "../IChannelReactor.h"
#include "Channel.h"
#include "ChannelWorker.h"

class ChannelReactor : public IChannelReactor
{
public:
	struct callback_t
	{
		TLambda<void(TRef<IChannel>)> Connected;
		TLambda<void(TRef<IChannel>)> Disconnect;
	};

public:
	ChannelReactor(size_t workerNum, callback_t callback);
	~ChannelReactor();
	void startup() override;
	void shutdown() override;
	bool running() const;
	void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	TFuture<bool> write(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) override;
	void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) override;

protected:
	virtual void onConnect(TRef<Channel> channel);
	virtual void onDisconnect(TRef<Channel> channel);
	virtual void onInbound(TRef<IChannelEvent> event);
	virtual void onOutbound(TRef<IChannelEvent> event, bool flush = false);

protected:
	TMutex m_EventLock;
	TThread m_EventThread;
	TAtomic<bool> m_Running;
	TAtomic<bool> m_Sending;
	TVector<TThread> m_WorkerThreads;
	TVector<TRef<ChannelWorker>> m_WorkerList;
	TQueue<TRef<IChannelEvent>> m_EventQueue;
	TLambda<void(TRef<IChannel>)> m_OnConnected;
	TLambda<void(TRef<IChannel>)> m_OnDisconnect;

private:
	friend class Channel;
};