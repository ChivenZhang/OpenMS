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

struct User
{
	int id;
	int age;
	MSString name;
	float money;
	OPENMS_TYPE(User, id, age, name, money);
};

void MySQLService::onInit()
{
	ClusterService::onInit();

	MySQLPool::startup();

#if 0 // TEST
	for (size_t n = 0; n < 10; ++n)
	{
		MySQLPool::execute("select * from userinfo", [](uint64_t rows, MSStringList const& result)
		{
			if (rows && rows != -1)
			{
				auto cols = result.size() / rows;
				for (size_t i = 0; i + cols <= result.size(); i += cols)
				{
					MSString line;
					for (size_t k = 0; k < cols; ++k) line += result[i + k] + " ";
					MS_INFO("%s", line.c_str());
				}
			}
		});
	}
#endif
}

void MySQLService::onExit()
{
	ClusterService::onExit();

	MySQLPool::shutdown();
}

void MySQLService::configureEndpoint(MySQLPool::config_t& config)
{
	config.IP = "127.0.0.1";
	config.PortNum = 3306;
	config.UserName = "admin";
	config.Password = "123456";
	config.Database = "game";
	config.Instance = 2;
}
