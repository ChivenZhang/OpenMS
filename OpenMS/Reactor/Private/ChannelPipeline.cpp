#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
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
#include "ChannelPipeline.h"
#include "Channel.h"
#include "ChannelHandler.h"

ChannelPipeline::ChannelPipeline(TRaw<Channel> channel)
	:
	m_Channel(channel)
{
}

TArrayView<const IChannelPipeline::inbound_t> ChannelPipeline::getInbounds() const
{
	return m_Inbounds;
}

TArrayView<const IChannelPipeline::outbound_t> ChannelPipeline::getOutbounds() const
{
	return m_Outbounds;
}

bool ChannelPipeline::addFirst(TStringView name, TRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(name.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Inbounds.end()) return false;
	m_Inbounds.insert(m_Inbounds.begin(), { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addLast(TStringView name, TRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(name.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Inbounds.end()) return false;
	m_Inbounds.insert(m_Inbounds.end(), { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addBefore(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(which.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Inbounds.end()) return false;
	hashName = THash(name.data());
	auto result2 = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Inbounds.end()) return false;
	m_Inbounds.insert(result, { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addAfter(TStringView which, TStringView name, TRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(which.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Inbounds.end()) return false;
	hashName = THash(name.data());
	auto result2 = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Inbounds.end()) return false;
	m_Inbounds.insert(result + 1, { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addFirst(TStringView name, TRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(name.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Outbounds.end()) return false;
	m_Outbounds.insert(m_Outbounds.begin(), { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addLast(TStringView name, TRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(name.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Outbounds.end()) return false;
	m_Outbounds.insert(m_Outbounds.end(), { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addBefore(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(which.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Outbounds.end()) return false;
	hashName = THash(name.data());
	auto result2 = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Outbounds.end()) return false;
	m_Outbounds.insert(result, { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addAfter(TStringView which, TStringView name, TRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = THash(which.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Outbounds.end()) return false;
	hashName = THash(name.data());
	auto result2 = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Outbounds.end()) return false;
	m_Outbounds.insert(result + 1, { handler, THash(name.data()) });
	return true;
}

bool ChannelPipeline::addFirst(TStringView name, inconfig_t config)
{
	return addFirst(name, TNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ config.OnRead, config.OnError, }));
}

bool ChannelPipeline::addLast(TStringView name, inconfig_t config)
{
	return addLast(name, TNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ config.OnRead, config.OnError, }));
}

bool ChannelPipeline::addBefore(TStringView which, TStringView name, inconfig_t config)
{
	return addBefore(which, name, TNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ config.OnRead, config.OnError, }));
}

bool ChannelPipeline::addAfter(TStringView which, TStringView name, inconfig_t config)
{
	return addAfter(which, name, TNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ config.OnRead, config.OnError, }));
}

bool ChannelPipeline::addFirst(TStringView name, outconfig_t config)
{
	return addFirst(name, TNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ config.OnWrite, config.OnError, }));
}

bool ChannelPipeline::addLast(TStringView name, outconfig_t config)
{
	return addLast(name, TNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ config.OnWrite, config.OnError, }));
}

bool ChannelPipeline::addBefore(TStringView which, TStringView name, outconfig_t config)
{
	return addBefore(which, name, TNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ config.OnWrite, config.OnError, }));
}

bool ChannelPipeline::addAfter(TStringView which, TStringView name, outconfig_t config)
{
	return addAfter(which, name, TNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ config.OnWrite, config.OnError, }));
}
