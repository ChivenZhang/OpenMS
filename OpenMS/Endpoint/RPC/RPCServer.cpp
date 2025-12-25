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
#include "RPCServer.h"

#include "RPCProtocol.h"
#include "Reactor/Private/ChannelHandler.h"

class RPCServerInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCServerInboundHandler(MSRaw<RPCServerBase> server);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCServerBase> m_Server;
};

RPCServerBase::RPCServerBase(config_t const& config)
	:
	m_Config(config)
{
}

void RPCServerBase::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<TCPServerReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Backlog,
		config.Workers,
		TCPServerReactor::callback_tcp_t
		{
			.OnOpen = [this](MSRef<IChannel> channel)
			{
				if (m_Config.Callback.OnOpen) m_Config.Callback.OnOpen(channel);
				channel->getPipeline()->addLast("default", MSNew<RPCServerInboundHandler>(this));
			},
			.OnClose = [this](MSRef<IChannel> channel)
			{
				if (m_Config.Callback.OnClose) m_Config.Callback.OnClose(channel);
			},
		}
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void RPCServerBase::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;

	MSMutexLock lock(m_LockSession);
	for (auto& session : m_Sessions)
	{
		auto callback = session.second;
		if (callback) callback({});
	}
	m_Sessions.clear();
	m_Session = 0;
}

bool RPCServerBase::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool RPCServerBase::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> RPCServerBase::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

bool RPCServerBase::unbind(MSStringView name)
{
	MSMutexLock lock(m_LockMethod);
	return m_Methods.erase(MSHash(name));
}

bool RPCServerBase::invoke(MSHnd<IChannel> client, uint32_t hash, MSStringView const& input, MSString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		MSMutexLock lock(m_LockMethod);
		auto result = m_Methods.find(hash);
		if (result == m_Methods.end()) return false;
		method = result->second;
	}
	if (method && method(client, input, output)) return true;
	return false;
}

bool RPCServerBase::bind(MSStringView name, method_t&& method)
{
	if (method == nullptr) return false;
	MSMutexLock lock(m_LockMethod);
	return m_Methods.emplace(MSHash(name), method).second;
}

bool RPCServerBase::call(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, MSStringView const& input, MSString& output)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

	// Convert request to string

	MSString buffer(sizeof(RPCRequestView) + input.size(), 0);
	auto& request = *(RPCRequestView*)buffer.data();
	request.Length = (uint32_t)buffer.size();
	request.Session = (++m_Session) | 0x80000000;
	request.Method = MSHash(name);
	if (input.empty() == false) ::memcpy(request.Buffer, input.data(), input.size());

	// Set promise to handle response

	MSPromise<MSString> promise;
	auto future = promise.get_future();
	{
		MSMutexLock lock(m_LockMethod);
		auto& session = m_Sessions[request.Session];
		session = [&](MSStringView const& response)
		{
			promise.set_value(MSString(response));
		};
	}

	// Send request to remote server

	m_Reactor->write(IChannelEvent::New(buffer, client));
	MS_INFO("服务端=>客户端：%u", (uint32_t)buffer.size());

	auto status = future.wait_for(std::chrono::milliseconds(timeout));
	{
		MSMutexLock lock(m_LockMethod);
		m_Sessions.erase(request.Session);
	}
	if (status == std::future_status::ready)
	{
		output = future.get();
		return true;
	}
	return false;
}

bool RPCServerBase::async(MSHnd<IChannel> client, MSStringView const& name, uint32_t timeout, MSStringView const& input, MSLambda<void(MSString&&)>&& callback)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

	// Convert request to string

	MSString buffer(sizeof(RPCRequestView) + input.size(), 0);
	auto& request = *(RPCRequestView*)buffer.data();
	request.Length = (uint32_t)buffer.size();
	request.Session = (++m_Session) | 0x80000000;
	request.Method = MSHash(name);
	if (input.empty() == false) ::memcpy(request.Buffer, input.data(), input.size());

	// Set timer to handle response

	{
		MSMutexLock lock(m_LockMethod);
		auto& session = m_Sessions[request.Session];
		session = [callback](MSStringView const& response)
		{
			if (callback) callback(MSString(response));
		};
	}

	m_Timer.start(timeout, 0, [sessionID = request.Session, this](uint32_t handle)
	{
		decltype(m_Sessions)::value_type::second_type callback;
		{
			MSMutexLock lock(m_LockMethod);
			auto result = m_Sessions.find(sessionID);
			if (result != m_Sessions.end())
			{
				callback = result->second;
				m_Sessions.erase(result);
			}
		}
		if (callback) callback({});
	});

	// Send request to remote server

	m_Reactor->write(IChannelEvent::New(buffer, client));
	MS_INFO("服务端=>客户端：%u", (uint32_t)buffer.size());
	return true;
}

RPCServerInboundHandler::RPCServerInboundHandler(MSRaw<RPCServerBase> server)
	:
	m_Server(server)
{
}

bool RPCServerInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	m_Buffer += event->Message;
	auto& package = *(RPCRequestBase*)m_Buffer.data();
	if (sizeof(RPCRequestBase) <= m_Buffer.size() && package.Length <= m_Buffer.size())
	{
		MS_INFO("服务端<=客户端:%u", package.Length);
		// Call from client
		if ((package.Session & 0X80000000) == 0)
		{
			auto& request = *(RPCRequestView*)m_Buffer.data();
			auto message = MSStringView(request.Buffer, request.Length - sizeof(RPCRequestView));
			if (message.size() <= m_Server->m_Config.Buffers)
			{
				MSString output;
				if (m_Server->invoke(event->Channel, request.Method, message, output))
				{
					MSString buffer(sizeof(RPCResponseView) + output.size(), 0);
					RPCResponseView& response = *(RPCResponseView*)buffer.data();
					response.Length = (uint32_t)buffer.size();
					response.Session = request.Session;
					if (output.empty() == false) ::memcpy(response.Buffer, output.data(), output.size());

					context->write(IChannelEvent::New(buffer));
				}
			}
		}
		else
		{
			auto& response = *(RPCResponseView*)m_Buffer.data();
			auto message = MSStringView(response.Buffer, response.Length - sizeof(RPCResponseView));
			if (message.size() <= m_Server->m_Config.Buffers)
			{
				decltype(m_Server->m_Sessions)::value_type::second_type callback;
				{
					MSMutexLock lock(m_Server->m_LockSession);
					auto result = m_Server->m_Sessions.find(response.Session);
					if (result != m_Server->m_Sessions.end())
					{
						callback = result->second;
						m_Server->m_Sessions.erase(result);
					}
				}
				if (callback) callback(message);
			}
		}
		m_Buffer = m_Buffer.substr(package.Length);
	}
	return false;
}
