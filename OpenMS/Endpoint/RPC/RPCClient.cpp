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
#include "RPCClient.h"
#include "Reactor/Private/ChannelHandler.h"

class RPCClientInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCClientInboundHandler(MSRaw<RPCClientBase> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCClientBase> m_Client;
};

RPCClientBase::RPCClientBase(config_t const& config)
	:
	m_Config(config)
{
}

void RPCClientBase::startup()
{
	auto config = m_Config;
	m_Reactor = MSNew<TCPClientReactor>(
		IPv4Address::New(config.IP, config.PortNum),
		config.Workers,
		TCPClientReactor::callback_tcp_t
		{
			.OnOpen = [this](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addLast("default", MSNew<RPCClientInboundHandler>(this));
			},
		}
	);
	m_Reactor->startup();
	if (m_Reactor->running() == false) MS_FATAL("failed to start reactor");
}

void RPCClientBase::shutdown()
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

bool RPCClientBase::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool RPCClientBase::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> RPCClientBase::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

bool RPCClientBase::unbind(MSStringView name)
{
	MSMutexLock lock(m_LockMethod);
	return m_Methods.erase(MSHash(name));
}

bool RPCClientBase::invoke(uint32_t hash, MSStringView const& input, MSString& output)
{
	decltype(m_Methods)::value_type::second_type method;
	{
		MSMutexLock lock(m_LockMethod);
		auto result = m_Methods.find(hash);
		if (result == m_Methods.end()) return false;
		method = result->second;
	}
	if (method && method(input, output)) return true;
	return false;
}

bool RPCClientBase::bind(MSStringView name, MSLambda<bool(MSStringView const& input, MSString& output)>&& method)
{
	if (method == nullptr) return false;
	MSMutexLock lock(m_LockMethod);
	return m_Methods.emplace(MSHash(name), method).second;
}

MSBinary<MSString, bool> RPCClientBase::call(MSStringView const& name, uint32_t timeout, MSStringView const& input)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return {{}, false};

	// Convert request to string

	MSString output(sizeof(RPCRequestView) + input.size(), 0);
	auto& request = *(RPCRequestView*)output.data();
	request.Length = (uint32_t)output.size();
	request.Session = (++m_Session) & 0x7FFFFFFF;
	request.Method = MSHash(name);
	if (input.empty() == false) ::memcpy(request.Buffer, input.data(), input.size());

	// Set promise to handle response

	MSPromise<MSString> promise;
	auto future = promise.get_future();
	{
		MSMutexLock lock(m_LockSession);
		auto& session = m_Sessions[request.Session];
		session = [&](MSStringView const& response) { promise.set_value(MSString(response)); };
	}

	// Send request to remote server

	m_Reactor->write(IChannelEvent::New(output), nullptr);

	auto status = future.wait_for(std::chrono::milliseconds(timeout));
	{
		MSMutexLock lock(m_LockSession);
		m_Sessions.erase(request.Session);
	}
	if (status == std::future_status::ready)
	{
		return {future.get(), true};
	}
	return {{}, false};
}

bool RPCClientBase::async(MSStringView const& name, uint32_t timeout, MSStringView const& input, MSLambda<void(MSString&&)>&& callback)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

	// Convert request to string

	MSString output(sizeof(RPCRequestView) + input.size(), 0);
	auto& requestView = *(RPCRequestView*)output.data();
	requestView.Length = (uint32_t)output.size();
	requestView.Session = ++m_Session;
	requestView.Method = MSHash(name);
	if (output.empty() == false) ::memcpy(requestView.Buffer, input.data(), input.size());

	// Set timer to handle response

	{
		MSMutexLock lock(m_LockSession);
		auto& session = m_Sessions[requestView.Session];
		session = [callback](MSStringView const& response)
		{
			if (callback) callback(MSString(response));
		};
	}

	m_Timer.start(timeout, 0, [sessionID = requestView.Session, this](uint32_t handle)
	{
		decltype(m_Sessions)::value_type::second_type callback;
		{
			MSMutexLock lock(m_LockSession);
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

	m_Reactor->write(IChannelEvent::New(output), nullptr);
	return true;
}

RPCClientInboundHandler::RPCClientInboundHandler(MSRaw<RPCClientBase> client)
	:
	m_Client(client)
{
}

bool RPCClientInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	m_Buffer += event->Message;
	auto& package = *(RPCRequestBase*)m_Buffer.data();
	if (sizeof(RPCRequestBase) <= m_Buffer.size() && package.Length <= m_Buffer.size())
	{
		// Call from server
		if (package.Session & 0X80000000)
		{
			auto& request = *(RPCRequestView*)m_Buffer.data();
			auto message = MSStringView(request.Buffer, request.Length - sizeof(RPCRequestView));
			if (message.size() <= m_Client->m_Config.Buffers)
			{
				MSString output;
				if (m_Client->invoke(request.Method, message, output))
				{
					MSString buffer(sizeof(RPCResponseView) + output.size(), 0);
					RPCResponseView& response = *(RPCResponseView*)buffer.data();
					response.Length = (uint32_t)buffer.size();
					response.Session = request.Session;
					if (output.empty() == false) ::memcpy(response.Buffer, output.data(), output.size());

					context->write(IChannelEvent::New(buffer));
				}
				else
				{
					context->close();
				}
			}
		}
		else
		{
			auto& response = *(RPCResponseView*)m_Buffer.data();
			auto message = MSStringView(response.Buffer, response.Length - sizeof(RPCResponseView));
			if (message.size() <= m_Client->m_Config.Buffers)
			{
				decltype(m_Client->m_Sessions)::value_type::second_type callback;
				{
					MSMutexLock lock(m_Client->m_LockSession);
					auto result = m_Client->m_Sessions.find(response.Session);
					if (result != m_Client->m_Sessions.end())
					{
						callback = result->second;
						m_Client->m_Sessions.erase(result);
					}
				}
				if (callback) callback(message);
			}
		}
		m_Buffer = m_Buffer.substr(package.Length);
	}
	return false;
}
