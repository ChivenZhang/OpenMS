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
#include "Channel.h"
#include "ChannelReactor.h"

Channel::Channel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote)
	:
	m_Running(true),
	m_Context(this),
	m_Pipeline(this),
	m_Reactor(reactor),
	m_LocalAddr(local),
	m_RemoteAddr(remote)
{
}

Channel::~Channel()
{
	m_Running = false;
}

bool Channel::running() const
{
	return m_Running;
}

TRaw<const IChannelAddress> Channel::getLocal() const
{
	return m_LocalAddr.get();
}

TRaw<const IChannelAddress> Channel::getRemote() const
{
	return m_RemoteAddr.get();
}

TRaw<const IChannelContext> Channel::getContext() const
{
	return &m_Context;
}

TRaw<IChannelPipeline> Channel::getPipeline()
{
	return &m_Pipeline;
}

TRaw<const IChannelPipeline> Channel::getPipeline() const
{
	return &m_Pipeline;
}

void Channel::close()
{
	m_Running = false;
}

TFuture<bool> Channel::close(TPromise<bool>&& promise)
{
	auto result = promise.get_future();
	close();
	promise.set_value(true);
	return result;
}

void Channel::read(TRef<IChannelEvent> event)
{
	if (m_Running == false) return;

	auto inbounds = m_Pipeline.getInbounds();
	for (size_t i = 0; i < inbounds.size(); ++i)
	{
		try
		{
			inbounds[i].Handler->channelRead(&m_Context, event.get());
		}
		catch (TException ex)
		{
			inbounds[i].Handler->channelCatch(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = std::exception("unknown exception caught in " __FILE__ " [ " __FUNCTION__ " ]");
			inbounds[i].Handler->channelCatch(&m_Context, std::move(ex));
		}
	}

	auto outbounds = m_Pipeline.getOutbounds();
	for (size_t i = 0; i < outbounds.size(); ++i)
	{
		try
		{
			outbounds[i].Handler->channelWrite(&m_Context, event.get());
		}
		catch (TException ex)
		{
			outbounds[i].Handler->channelCatch(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = std::exception("unknown exception caught in " __FILE__ " [ " __FUNCTION__ " ]");
			outbounds[i].Handler->channelCatch(&m_Context, std::move(ex));
		}
	}
}

TFuture<bool> Channel::read(TRef<IChannelEvent> event, TPromise<bool>&& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	read(event);
	promise.set_value(true);
	return result;
}

void Channel::write(TRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	event->Channel = shared_from_this();
	m_Reactor->onOutbound(event, false);
}

TFuture<bool> Channel::write(TRef<IChannelEvent> event, TPromise<bool>&& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->UserData = &promise;
	write(event);
	return result;
}

void Channel::writeFlush(TRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	event->Channel = shared_from_this();
	m_Reactor->onOutbound(event, true);
}

TFuture<bool> Channel::writeFlush(TRef<IChannelEvent> event, TPromise<bool>&& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->UserData = &promise;
	writeFlush(event);
	return result;
}
