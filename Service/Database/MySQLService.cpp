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

#if 1 // TEST
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
	config.IP = property(identity() + ".mysql.ip", MSString("127.0.0.1"));
	config.PortNum  = property(identity() + ".mysql.port", 3306U);
	config.UserName = property(identity() + ".mysql.username", MSString());
	config.Password = property(identity() + ".mysql.password", MSString());
	config.Database = property(identity() + ".mysql.database", MSString());
	config.Instance = property(identity() + ".mysql.instance", 1U);
	config.Reconnect = property(identity() + ".mysql.reconnect", 1U);
}
