#pragma once
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
#include "../IChannelReactor.h"
#include "Channel.h"
#include "ChannelWorker.h"

class ChannelReactor : public IChannelReactor
{
public:
	struct callback_t
	{
		TLambda<void(TRef<IChannel>)> OnOpen;
		TLambda<void(TRef<IChannel>)> OnClose;
	};

public:
	ChannelReactor(size_t workerNum, callback_t callback);
	~ChannelReactor();
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	THnd<IChannelAddress> address() const override;
	void write(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	TFuture<bool> write(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) override;
	void writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address) override;
	TFuture<bool> writeAndFlush(TRef<IChannelEvent> event, TRef<IChannelAddress> address, TPromise<bool>&& promise) override;

protected:
	virtual void onConnect(TRef<Channel> channel);
	virtual void onOnClose(TRef<Channel> channel);
	virtual void onInbound(TRef<IChannelEvent> event);
	virtual void onOutbound(TRef<IChannelEvent> event, bool flush = false);

protected:
	TMutex m_EventLock;
	TThread m_EventThread;
	TAtomic<bool> m_Running;
	TAtomic<bool> m_Sending;
	TAtomic<bool> m_Connect;
	TVector<TThread> m_WorkerThreads;
	TVector<TRef<ChannelWorker>> m_WorkerList;
	TQueue<TRef<IChannelEvent>> m_EventQueue;
	TLambda<void(TRef<IChannel>)> m_OnOnOpen;
	TLambda<void(TRef<IChannel>)> m_OnOnClose;

private:
	friend class Channel;
};