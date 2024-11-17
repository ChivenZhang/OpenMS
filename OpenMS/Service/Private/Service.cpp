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
#include "Service.h"
#include "../IEnvironment.h"

int IEnvironment::argc = 0;
char** IEnvironment::argv = nullptr;
TRaw<IService> IService::m_Instance = nullptr;
TRaw<IService> IService::Instance() { return m_Instance; }

struct ServerConfig
{
	std::string ip;
	uint16_t port;
	uint32_t backlog;
	uint32_t workers;
	OPENMS_TYPE(ServerConfig, ip, port, backlog, workers)
};

void Service::startup(int argc, char** argv)
{
	if (IService::Instance()) return;
	m_Instance = this;

	auto config = AUTOWIRE2_THIS(IValue, "registry.server").bean()->value<ServerConfig>();
	m_Reactor = TNew<TCPServerReactor>(
		IPv4Address::New(config.ip, config.port), config.backlog, config.workers,
		TCPServerReactor::callback_t{
		[=](TRef<IChannel> channel) {
			TPrint("New connection!");
		},
		[=](TRef<IChannel> channel) {
			TPrint("Disconnection !");
		},
		});
	m_Reactor->startup();
	if (m_Reactor->running() == false) TFatal("Failed to start reactor");
}

void Service::shutdown()
{
	if (IService::Instance() == nullptr) return;
	m_Instance = nullptr;
	m_Reactor->shutdown();
}

TString Service::property(TStringView name) const
{
	auto source = AUTOWIRE(IProperty)::bean();
	if (source == nullptr) return TString();
	return source->property(name);
}