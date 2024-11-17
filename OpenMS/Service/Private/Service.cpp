#include "Service.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "Service.h"
#include "../IEnvironment.h"

int IEnvironment::argc = 0;
char** IEnvironment::argv = nullptr;

TRaw<IService> IService::Instance()
{
	return m_Instance;
}
TRaw<IService> IService::m_Instance = nullptr;

void Service::startup(int argc, char** argv)
{
	if (IService::Instance()) return;
	m_Instance = this;

	// Initialize reactor

	auto address = property("registry.server.ip");
	auto portNum = property<uint16_t>("registry.server.port");
	auto backlog = property<uint32_t>("registry.server.backlog");
	auto workers = property<uint32_t>("registry.server.workers");
	m_Reactor = TNew<TCPServerReactor>(IPv4Address::New(address, portNum), backlog, workers, TCPServerReactor::callback_t{
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
	return AUTOWIRE(IPropertySource, "application")::get()->property(name);
}