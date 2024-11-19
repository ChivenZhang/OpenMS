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

void Service::startup()
{
	if (IService::Instance()) return;
	m_Instance = this;
}

void Service::shutdown()
{
	if (IService::Instance() == nullptr) return;
	m_Instance = nullptr;
}

TString Service::property(TString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return TString();
	return source->property(name);
}