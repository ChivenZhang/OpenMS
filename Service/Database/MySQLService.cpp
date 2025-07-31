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
#include "MySQLService.h"

// ========================================================================================

MSString MySQLService::identity() const
{
	return "mysql";
}

struct UserInfo
{
	int id;
	int age;
	MSString name;
	float money;
	OPENMS_TYPE(UserInfo, id, age, name, money);
};

void MySQLService::onInit()
{
	ClusterService::onInit();

	MySQLClient::startup();

	MSList<UserInfo> result;
	if (MySQLClient::query("select * from userinfo order by money", result))
	{
		for (auto& user : result)
		{
			MS_INFO("User Info: id=%d, name=%s, age=%d, money=%.2f", user.id, user.name.c_str(), user.age, user.money);
		}
	}
}

void MySQLService::onExit()
{
	ClusterService::onExit();

	MySQLClient::shutdown();
}

void MySQLService::configureEndpoint(MySQLClient::config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 3306;
	config.UserName = "admin";
	config.Password = "123456";
	config.Database = "game";
}
