/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "GatewayServer.h"

#include "ForwardService.h"
#include "Endpoint/TCP/TCPServer.h"
#include "Handler/AES/AESHandler.h"
#include "Server/Private/Service.h"

#define AES256_KEY 0x50, 0x71, 0x47, 0x55, 0x6f, 0x4c, 0x36, 0x7a, 0x55, 0x50, 0x38, 0x43, 0x30, 0x42, 0x38, 0x43, 0x2f, 0x55, 0x45, 0x4d, 0x50, 0x49, 0x73, 0x36, 0x4f, 0x63, 0x42, 0x4c, 0x79, 0x32, 0x72, 0x35

MSString GatewayServer::identity() const
{
	return "gateway";
}

void GatewayServer::onInit()
{
	ClusterServer::onInit();

	auto hub = AUTOWIRE(IMailHub)::bean();

	auto service = MSNew<Service>();
	service->bind("login", [=](MSString user, MSString pass)->MSAsync<uint32_t>
	{
		co_return co_await [=](MSAwait<uint32_t> promise)
		{
			service->async("logic", "login", 100, MSTuple{user, pass}, [=](uint32_t response)
			{
				promise(response);
			});
		};
	});
	service->bind("logout", [=](uint32_t userID)->MSAsync<bool>
	{
		co_return co_await [=](MSAwait<bool> promise)
		{
			service->async("logic", "logout", 100, MSTuple{userID}, [=](bool response)
			{
				promise(response);
			});
		};
	});
	service->bind("signup", [=](MSString user, MSString pass)->MSAsync<bool>
	{
		co_return co_await [=](MSAwait<bool> promise)
		{
			service->async("logic", "signup", 100, MSTuple{user, pass}, [=](bool response)
			{
				promise(response);
			});
		};
	});

	hub->create("gateway", service);

	m_TCPServer = MSNew<TCPServer>(TCPServer::config_t{
		.IP = property(identity() + ".client.ip", MSString("127.0.0.1")),
		.PortNum = (uint16_t)property(identity() + ".client.port", 0U),
		.Backlog = property(identity() + ".client.backlog", 0U),
		.Workers = property(identity() + ".client.workers", 0U),
		.Callback = ChannelReactor::callback_t
		{
			.OnOpen = [=, this](MSRef<IChannel> channel)
			{
				channel->getPipeline()->addFirst("decrypt", MSNew<AESInboundHandler>(AESInboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("input", IChannelPipeline::handler_in
				{
					.OnHandle = [=, this](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						struct message_t
						{
							uint32_t Service;
							uint32_t Method;
							char Request[0];
						};
						if (event->Message.size() < sizeof(message_t)) return false;
						auto message = reinterpret_cast<message_t*>(event->Message.data());
						auto content = MSStringView(message->Request, event->Message.size() - sizeof(message_t));
						// TODO: 检查接口访问权限
						switch (message->Service)
						{
						case MSHash("gateway"):
							{
								switch (message->Method)
								{
								case MSHash("login"):
									{
										MSTuple<MSString, MSString> request;
										MSTypeC(content, request);
										service->async("logic", "login", 100, std::move(request), [=](uint32_t userID)
										{
											context->write(IChannelEvent::New(std::to_string(userID)));
											if (userID)
											{
												auto serviceName = "client:" + std::to_string(userID);
												auto clientService = MSNew<ForwardService>(channel);
												hub->create(serviceName, clientService);
												channel->getContext()->userdata() = userID;
											}
										});
									}
									break;
								case MSHash("logout"):
									{
										MSTuple<uint32_t> request;
										MSTypeC(content, request);
										service->async("logic", "logout", 100, std::move(request), [=](bool result)
										{
											context->write(IChannelEvent::New(std::to_string(result)));
											auto userID = (uint32_t)channel->getContext()->userdata();
											if (userID && result)
											{
												auto serviceName = "client:" + std::to_string(userID);
												hub->cancel(serviceName);
												channel->getContext()->userdata() = 0;
											}
										});
									}
									break;
								case MSHash("signup"):
									{
										MSTuple<MSString, MSString> request;
										MSTypeC(content, request);
										service->async("logic", "signup", 100, std::move(request), [=](bool result)
										{
											context->write(IChannelEvent::New(std::to_string(result)));
										});
									}
									break;
								}
							} break;
						default:
							{
								auto userID = channel->getContext()->userdata();
								if (userID)
								{

								}
							} break;
						}
						return false;
					},
				});
				channel->getPipeline()->addFirst("encrypt", MSNew<AESOutboundHandler>(AESOutboundHandler::config_t
				{
					.Key = { AES256_KEY },
				}));
				channel->getPipeline()->addLast("output", IChannelPipeline::handler_out
				{
					.OnHandle = [](MSRaw<IChannelContext> context, MSRaw<IChannelEvent> event)->bool
					{
						context->write(IChannelEvent::New(event->Message));
						return false;
					},
				});
			},
			.OnClose = [=, this](MSRef<IChannel> channel)
			{
				auto userID = (uint32_t)channel->getContext()->userdata();
				if (userID)
				{
					auto serviceName = "client:" + std::to_string(userID);
					hub->cancel(serviceName);
					channel->getContext()->userdata() = 0;
				}
			},
		}
	});
	m_TCPServer->startup();

	m_KeepAlive = startTimer(0, OPENMS_HEARTBEAT * 1000, [this](uint32_t handle)
	{
		if (m_TCPServer->connect() == false)
		{
			m_TCPServer->shutdown();
			m_TCPServer->startup();
		}
	});
}

void GatewayServer::onExit()
{
	if (m_KeepAlive) stopTimer(m_KeepAlive);
	m_KeepAlive = 0;

	if (m_TCPServer) m_TCPServer->shutdown();
	m_TCPServer = nullptr;

	ClusterServer::onExit();
}