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
#include "../IChannelReactor.h"
#include "Channel.h"
#include "ChannelWorker.h"

class ChannelReactor : public IChannelReactor
{
public:
	struct callback_t
	{
		MSLambda<void(MSRef<IChannel>)> OnOpen;
		MSLambda<void(MSRef<IChannel>)> OnClose;
	};

public:
	ChannelReactor(size_t workerNum, callback_t callback);
	~ChannelReactor();
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;
	void write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;
	MSFuture<bool> write(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address, MSPromise<bool>&& promise) override;
	void writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address) override;
	MSFuture<bool> writeAndFlush(MSRef<IChannelEvent> event, MSRef<IChannelAddress> address, MSPromise<bool>&& promise) override;

protected:
	virtual void onConnect(MSRef<Channel> channel);
	virtual void onDisconnect(MSRef<Channel> channel);
	virtual void onInbound(MSRef<IChannelEvent> event);
	virtual void onOutbound(MSRef<IChannelEvent> event, bool flush = false);

protected:
	MSMutex m_EventLock;
	MSThread m_EventThread;
	MSAtomic<bool> m_Running;
	MSAtomic<bool> m_Sending;
	MSAtomic<bool> m_Connect;
	MSVector<MSThread> m_WorkerThreads;
	MSVector<MSRef<ChannelWorker>> m_WorkerList;
	MSQueue<MSRef<IChannelEvent>> m_EventQueue;
	MSLambda<void(MSRef<IChannel>)> m_OnOnOpen;
	MSLambda<void(MSRef<IChannel>)> m_OnOnClose;

private:
	friend class Channel;
};