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
#include "Channel.h"
#include "ChannelReactor.h"
#include <cpptrace/cpptrace.hpp>

Channel::Channel(MSRaw<ChannelReactor> reactor, MSRef<IChannelAddress> local, MSRef<IChannelAddress> remote, uint32_t workID)
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

MSHnd<IChannelAddress> Channel::getLocal() const
{
	return m_LocalAddr;
}

MSHnd<IChannelAddress> Channel::getRemote() const
{
	return m_RemoteAddr;
}

MSRaw<IChannelContext> Channel::getContext() const
{
	return (MSRaw<IChannelContext>) & m_Context;
}

MSRaw<IChannelPipeline> Channel::getPipeline() const
{
	return (MSRaw<IChannelPipeline>) & m_Pipeline;
}

void Channel::close()
{
	m_Running = false;
}

MSFuture<bool> Channel::close(MSPromise<bool>& promise)
{
	auto result = promise.get_future();
	close();
	promise.set_value(true);
	return result;
}

void Channel::readChannel(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;

	auto result = true;
	auto inbounds = m_Pipeline.getInbounds();
	for (size_t i = 0; result && i < inbounds.size(); ++i)
	{
		try
		{
			result = inbounds[i].Handler->channelRead(&m_Context, event.get());
		}
		catch (MSError ex)
		{
			inbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = cpptrace::logic_error("unknown exception");
			inbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
	}

	auto outbounds = m_Pipeline.getOutbounds();
	for (size_t i = 0; result && i < outbounds.size(); ++i)
	{
		try
		{
			result = outbounds[i].Handler->channelWrite(&m_Context, event.get());
		}
		catch (MSError ex)
		{
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = cpptrace::logic_error("unknown exception");
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
	}
}

void Channel::writeChannel(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;

	auto result = true;
	auto outbounds = m_Pipeline.getOutbounds();
	for (size_t i = 0; result && i < outbounds.size(); ++i)
	{
		try
		{
			result = outbounds[i].Handler->channelWrite(&m_Context, event.get());
		}
		catch (MSError ex)
		{
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
		catch (...)
		{
			auto ex = cpptrace::logic_error("unknown exception");
			outbounds[i].Handler->channelError(&m_Context, std::move(ex));
		}
	}
}

void Channel::write(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	event->Channel = shared_from_this();
	m_Reactor->onOutbound(event, false);
}

MSFuture<bool> Channel::write(MSRef<IChannelEvent> event, MSPromise<bool>& promise)
{
	if (m_Running == false) return MSFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	write(event);
	return result;
}

void Channel::writeAndFlush(MSRef<IChannelEvent> event)
{
	if (m_Running == false) return;
	event->Channel = shared_from_this();
	m_Reactor->onOutbound(event, true);
}

MSFuture<bool> Channel::writeAndFlush(MSRef<IChannelEvent> event, MSPromise<bool>& promise)
{
	if (m_Running == false) return MSFuture<bool>();
	auto result = promise.get_future();
	event->Promise = &promise;
	writeAndFlush(event);
	return result;
}
