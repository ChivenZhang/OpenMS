#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
#include "ChannelPipeline.h"
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
#include "ChannelPipeline.h"
#include "Channel.h"
#include "ChannelHandler.h"

ChannelPipeline::ChannelPipeline(MSRaw<Channel> channel)
	:
	m_Channel(channel)
{
}

MSArrayView<const IChannelPipeline::inbound_t> ChannelPipeline::getInbounds() const
{
	return m_Inbounds;
}

MSArrayView<const IChannelPipeline::outbound_t> ChannelPipeline::getOutbounds() const
{
	return m_Outbounds;
}

bool ChannelPipeline::addFirst(MSStringView name, MSRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(name.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Inbounds.end()) return false;
	m_Inbounds.insert(m_Inbounds.begin(), { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addLast(MSStringView name, MSRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(name.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Inbounds.end()) return false;
	m_Inbounds.insert(m_Inbounds.end(), { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addBefore(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(which.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Inbounds.end()) return false;
	hashName = MSHash(name.data());
	auto result2 = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Inbounds.end()) return false;
	m_Inbounds.insert(result, { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addAfter(MSStringView which, MSStringView name, MSRef<IChannelInboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(which.data());
	auto result = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Inbounds.end()) return false;
	hashName = MSHash(name.data());
	auto result2 = std::find_if(m_Inbounds.begin(), m_Inbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Inbounds.end()) return false;
	m_Inbounds.insert(result + 1, { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addFirst(MSStringView name, MSRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(name.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Outbounds.end()) return false;
	m_Outbounds.insert(m_Outbounds.begin(), { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addLast(MSStringView name, MSRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(name.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result != m_Outbounds.end()) return false;
	m_Outbounds.insert(m_Outbounds.end(), { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addBefore(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(which.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Outbounds.end()) return false;
	hashName = MSHash(name.data());
	auto result2 = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Outbounds.end()) return false;
	m_Outbounds.insert(result, { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addAfter(MSStringView which, MSStringView name, MSRef<IChannelOutboundHandler> handler)
{
	if (m_Channel->running() == false || handler == nullptr) return false;
	auto hashName = MSHash(which.data());
	auto result = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result == m_Outbounds.end()) return false;
	hashName = MSHash(name.data());
	auto result2 = std::find_if(m_Outbounds.begin(), m_Outbounds.end(), [hashName](auto& e) { return e.HashName == hashName; });
	if (result2 != m_Outbounds.end()) return false;
	m_Outbounds.insert(result + 1, { handler, MSHash(name.data()) });
	return true;
}

bool ChannelPipeline::addFirst(MSStringView name, handler_in handler)
{
	return addFirst(name, MSNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addLast(MSStringView name, handler_in handler)
{
	return addLast(name, MSNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addBefore(MSStringView which, MSStringView name, handler_in handler)
{
	return addBefore(which, name, MSNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addAfter(MSStringView which, MSStringView name, handler_in handler)
{
	return addAfter(which, name, MSNew<LambdaInboundHandler>(LambdaInboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addFirst(MSStringView name, handler_out handler)
{
	return addFirst(name, MSNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addLast(MSStringView name, handler_out handler)
{
	return addLast(name, MSNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addBefore(MSStringView which, MSStringView name, handler_out handler)
{
	return addBefore(which, name, MSNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}

bool ChannelPipeline::addAfter(MSStringView which, MSStringView name, handler_out handler)
{
	return addAfter(which, name, MSNew<LambdaOutboundHandler>(LambdaOutboundHandler::callback_t{ handler.OnHandle, handler.OnError, }));
}
