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
#include "BusinessServer.h"
#include "LogicService.h"
#include <OpenMS/Mailbox/Private/Mail.h>
#include <OpenMS/Server/Private/Service.h>

#include "LoginService.h"
#include "MatchService.h"

MSString BusinessServer::identity() const
{
	return "business";
}

void BusinessServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	auto loginService = MSNew<LoginService>();
	mailHub->create("login", loginService);

	auto logicService = MSNew<LogicService>();
	mailHub->create("logic", logicService);

	auto matchService = MSNew<MatchService>();
	mailHub->create("match", matchService);

	this->startTimer(0, 2000, [=, self = logicService.get()]()
	{
		self->call<void>("match", "updateMatch", "", 0, MSTuple{self->name()});

		self->call<void>(self->name(), "updateMatch", "", 0, MSTuple{});
	});
}

void BusinessServer::onExit()
{
	ClusterServer::onExit();
}