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
	explicit RPCClientInboundHandler(MSRaw<IRPCClient> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<IRPCClient> m_Client;
};

IRPCClient::IRPCClient(config_t const& config)
	:
	m_Config(config)
{
}

void IRPCClient::startup()
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

void IRPCClient::shutdown()
{
	if (m_Reactor) m_Reactor->shutdown();
	m_Reactor = nullptr;

	MSMutexLock lock(m_Locker);
	for (auto& session : m_Sessions)
	{
		auto callback = session.second;
		if (callback) callback({});
	}
	m_Sessions.clear();
}

bool IRPCClient::running() const
{
	return m_Reactor ? m_Reactor->running() : false;
}

bool IRPCClient::connect() const
{
	return m_Reactor ? m_Reactor->connect() : false;
}

MSHnd<IChannelAddress> IRPCClient::address() const
{
	return m_Reactor ? m_Reactor->address() : MSHnd<IChannelAddress>();
}

MSBinary<MSString, bool> IRPCClient::call(MSStringView const& name, uint32_t timeout, MSString const& input)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return {{}, false};

	// Convert request to string

	MSString output(sizeof(RPCRequestView) + input.size(), 0);
	auto& requestView = *(RPCRequestView*)output.data();
	requestView.ID = ++m_Session;
	requestView.Name = MSHash(name);
	requestView.Length = (uint32_t)input.size();
	if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

	// Set promise to handle response

	MSPromise<MSString> promise;
	auto future = promise.get_future();
	{
		MSMutexLock lock(m_Locker);
		auto& session = m_Sessions[requestView.ID];
		session = [&](MSStringView const& response) { promise.set_value(MSString(response)); };
	}

	// Send request to remote server

	auto length = (uint32_t)output.size();
	m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
	auto status = future.wait_for(std::chrono::milliseconds(timeout));
	{
		MSMutexLock lock(m_Locker);
		m_Sessions.erase(requestView.ID);
	}
	if (status == std::future_status::ready)
	{
		return {future.get(), true};
	}
	return {{}, false};
}

bool IRPCClient::async(MSStringView const& name, uint32_t timeout, MSString const& input, MSLambda<void(MSString&&)>&& callback)
{
	if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

	// Convert request to string

	MSString output(sizeof(RPCRequestView) + input.size(), 0);
	auto& requestView = *(RPCRequestView*)output.data();
	requestView.ID = ++m_Session;
	requestView.Name = MSHash(name);
	requestView.Length = (uint32_t)input.size();
	if (requestView.Length) ::memcpy(requestView.Buffer, input.data(), input.size());

	// Set timer to handle response

	{
		MSMutexLock lock(m_Locker);
		auto& session = m_Sessions[requestView.ID];
		session = [callback](MSStringView const& response)
		{
			if (callback) callback(MSString(response));
		};
	}

	m_Timer.start(timeout, 0, [sessionID = requestView.ID, this](uint32_t handle)
	{
		decltype(m_Sessions)::value_type::second_type callback;
		{
			MSMutexLock lock(m_Locker);
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

	auto length = (uint32_t)output.size();
	m_Reactor->writeAndFlush(IChannelEvent::New(MSString((char*)&length, sizeof(length)) + output), nullptr);
	return true;
}

RPCClientInboundHandler::RPCClientInboundHandler(MSRaw<IRPCClient> client)
	:
	m_Client(client)
{
}

bool RPCClientInboundHandler::channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)
{
	m_Buffer += event->Message;

	struct stream_t
	{
		uint32_t Length;
		char Buffer[0];
	};
	auto& stream = *(stream_t*)m_Buffer.data();

	if (sizeof(stream_t) <= m_Buffer.size() && sizeof(stream_t) + stream.Length <= m_Buffer.size())
	{
		auto message = MSStringView(stream.Buffer, stream.Length);
		if (message.size() <= m_Client->m_Config.Buffers)
		{
			auto& responseView = *(RPCResponseView*)message.data();
			if (sizeof(RPCResponseView) <= message.size() && sizeof(RPCResponseView) + responseView.Length == message.size())
			{
				decltype(m_Client->m_Sessions)::value_type::second_type callback;
				{
					MSMutexLock lock(m_Client->m_Locker);
					auto result = m_Client->m_Sessions.find(responseView.ID);
					if (result != m_Client->m_Sessions.end())
					{
						callback = result->second;
						m_Client->m_Sessions.erase(result);
					}
				}
				if (callback) callback(MSStringView(responseView.Buffer, responseView.Length));
			}
		}

		m_Buffer = m_Buffer.substr(sizeof(uint32_t) + stream.Length);
	}
	return false;
}
