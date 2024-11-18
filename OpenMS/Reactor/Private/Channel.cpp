#include "Channel.h"
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
#include "Channel.h"
#include "ChannelReactor.h"

Channel::Channel(TRaw<ChannelReactor> reactor, TRef<IChannelAddress> local, TRef<IChannelAddress> remote, uint32_t workID)
	:
	m_Running(true),
	m_WorkID(workID),
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

uint32_t Channel::getWorkID() const
{
	return m_WorkID;
}

TRaw<IChannelAddress> Channel::getLocal() const
{
	return m_LocalAddr.get();
}

TRaw<IChannelAddress> Channel::getRemote() const
{
	return m_RemoteAddr.get();
}

TRaw<IChannelContext> Channel::getContext() const
{
	return (TRaw<IChannelContext>) & m_Context;
}

TRaw<IChannelPipeline> Channel::getPipeline() const
{
	return (TRaw<IChannelPipeline>) & m_Pipeline;
}

void Channel::close()
{
	m_Running = false;
}

TFuture<bool> Channel::close(TPromise<bool>& promise)
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
			auto result = inbounds[i].Handler->channelRead(&m_Context, event.get());
			if (result == false) break;
		}
		catch (TException ex)
		{
			inbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = std::exception("unknown exception caught in " __FILE__ " [ " __FUNCTION__ " ]");
			inbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
	}

	auto outbounds = m_Pipeline.getOutbounds();
	for (size_t i = 0; i < outbounds.size(); ++i)
	{
		try
		{
			auto result = outbounds[i].Handler->channelWrite(&m_Context, event.get());
			if (result == false) break;
		}
		catch (TException ex)
		{
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = std::exception("unknown exception caught in " __FILE__ " [ " __FUNCTION__ " ]");
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
	}
}

TFuture<bool> Channel::read(TRef<IChannelEvent> event, TPromise<bool>& promise)
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

TFuture<bool> Channel::write(TRef<IChannelEvent> event, TPromise<bool>& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	write(event);
	return result;
}

void Channel::writeAndFlush(TRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	event->Channel = shared_from_this();
	m_Reactor->onOutbound(event, true);
}

TFuture<bool> Channel::writeAndFlush(TRef<IChannelEvent> event, TPromise<bool>& promise)
{
	if (m_Running == false) return TFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	writeAndFlush(event);
	return result;
}
