#pragma once
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
#include "OpenMS/Endpoint/IEndpoint.h"
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Endpoint/RPC/RPCProtocol.h"
#include "OpenMS/Reactor/TCP/TCPClientReactor.h"
#include "OpenMS/Reactor/Private/ChannelHandler.h"
#include "OpenMS/Toolkit/Timer.h"

class RPCClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 0;
		uint32_t Workers = 0;
		uint32_t Buffers = UINT16_MAX;
		TCPClientReactor::callback_tcp_t Callback;
	};

public:
	explicit RPCClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	template <class T, class... ARGS, OPENMS_NOT_SAME(T, void)>
	T call(MSStringView name, uint32_t timeout, ARGS... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return T();

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return T();
		}
		RPCRequest request;
		request.ID = ++m_Session;
		request.Name = name;
		request.Args = input;
		if (TTypeC(request, input) == false) return T();

		// Setup promise and callback

		if (timeout)
		{
			MSString output;
			auto promise = MSNew<MSPromise<void>>();
			auto future = promise->get_future();
			{
				MSMutexLock lock(m_Lock);
				auto& session = m_Sessions[request.ID];
				session.OnResult = [promise, &output](MSString&& response)
				{
					output = response;
					promise->set_value();
				};
			}

			// Send input to remote server

			m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);

			// Wait for result and return

			auto status = future.wait_for(std::chrono::milliseconds(timeout));
			{
				MSMutexLock lock(m_Lock);
				m_Sessions.erase(request.ID);
			}
			if (status == std::future_status::ready)
			{
				T result;
				if (TTypeC(output, result) == true) return result;
			}
		}
		else
		{
			// Send input to remote server

			m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);
		}
		return T();
	}

	template <class T, class... ARGS, OPENMS_IS_SAME(T, void)>
	bool call(MSStringView name, uint32_t timeout, ARGS... args)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (TTypeC(std::make_tuple(args...), input) == false) return false;
		}
		RPCRequest request;
		request.ID = ++m_Session;
		request.Name = name;
		request.Args = input;
		if (TTypeC(request, input) == false) return false;

		// Set up promise and callback

		if (timeout)
		{
			auto promise = MSNew<MSPromise<void>>();
			auto future = promise->get_future();
			{
				MSMutexLock lock(m_Lock);
				auto& session = m_Sessions[request.ID];
				session.OnResult = [promise](MSString&& response) { promise->set_value(); };
			}

			// Send input to remote server

			m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);

			// Wait for result and return

			future.wait_for(std::chrono::milliseconds(timeout));
			{
				MSMutexLock lock(m_Lock);
				m_Sessions.erase(request.ID);
			}
		}
		else
		{
			// Send input to remote server

			m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);
		}
		return true;
	}

	template <class T, class... ARGS, OPENMS_NOT_SAME(T, void)>
	bool async(MSStringView name, uint32_t timeout, MSTuple<ARGS...> args, MSLambda<void(T&&)> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCRequest request;
		request.ID = ++m_Session;
		request.Name = name;
		request.Args = input;
		if (TTypeC(request, input) == false) return false;

		// Setup promise and callback

		if (timeout)
		{
			{
				MSMutexLock lock(m_Lock);
				auto& session = m_Sessions[request.ID];
				session.OnResult = [callback](MSString&& response)
				{
					T result;
					TTypeC(response, result);
					if (callback) callback(std::move(result));
				};
			}

			m_Timer.start(timeout, 0, [=, sessionID = request.ID](uint32_t handle)
			{
				MSMutexLock lock(m_Lock);
				auto result = m_Sessions.find(sessionID);
				if (result != m_Sessions.end())
				{
					result->second.OnResult(MSString());
					m_Sessions.erase(result);
				}
			});
		}

		// Send input to remote server

		m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);
		return true;
	}

	template <class T, class... ARGS, OPENMS_IS_SAME(T, void)>
	bool async(MSStringView name, uint32_t timeout, MSTuple<ARGS...> args, MSLambda<void()> callback)
	{
		if (m_Reactor == nullptr || m_Reactor->connect() == false) return false;

		// Convert arguments to string

		MSString input;
		if (std::is_same_v<MSTuple<ARGS...>, MSTuple<>> == false)
		{
			if (TTypeC(args, input) == false) return false;
		}
		RPCRequest request;
		request.ID = ++m_Session;
		request.Name = name;
		request.Args = input;
		if (TTypeC(request, input) == false) return false;

		// Setup promise and callback

		if (timeout)
		{
			{
				MSMutexLock lock(m_Lock);
				auto& session = m_Sessions[request.ID];
				session.OnResult = [callback](MSString&& response)
				{
					if (callback) callback();
				};
			}

			m_Timer.start(timeout, 0, [=, sessionID = request.ID](uint32_t handle)
			{
				MSMutexLock lock(m_Lock);
				auto result = m_Sessions.find(sessionID);
				if (result != m_Sessions.end())
				{
					result->second.OnResult(MSString());
					m_Sessions.erase(result);
				}
			});
		}

		// Send input to remote server

		m_Reactor->writeAndFlush(IChannelEvent::New(input + char()), nullptr);
		return true;
	}

protected:
	friend class RPCClientInboundHandler;
	Timer m_Timer;
	config_t m_Config;
	MSMutex m_Lock;
	uint32_t m_Session = 0;
	uint32_t m_Buffers = UINT32_MAX;
	MSRef<TCPClientReactor> m_Reactor;

	struct invoke_t
	{
		MSLambda<void(MSString&&)> OnResult;
	};

	MSMap<uint32_t, invoke_t> m_Sessions;
};

class RPCClientInboundHandler : public ChannelInboundHandler
{
public:
	explicit RPCClientInboundHandler(MSRaw<RPCClient> client);
	bool channelRead(MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event) override;

protected:
	MSString m_Buffer;
	MSRaw<RPCClient> m_Client;
};
