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
#include "MySQLServer.h"

// ========================================================================================

MSString MySQLServer::identity() const
{
	return "mysql";
}

void MySQLServer::onInit()
{
	ClusterServer::onInit();

	m_MysqlPool = MSNew<MySQLPool>(MySQLPool::config_t{
		.IP = property(identity() + ".mysql.ip", MSString("127.0.0.1")),
		.PortNum  = (uint16_t)property(identity() + ".mysql.port", 3306U),
		.UserName = property(identity() + ".mysql.username", MSString()),
		.Password = property(identity() + ".mysql.password", MSString()),
		.Database = property(identity() + ".mysql.database", MSString()),
		.Instance = (uint8_t)property(identity() + ".mysql.instance", 1U),
		.Reconnect = (uint8_t)property(identity() + ".mysql.reconnect", 1U),
	});
	m_MysqlPool->startup();

#if 0 // TEST
	for (size_t n = 0; n < 10; ++n)
	{
		m_MysqlPool->execute("select * from userinfo", [](uint64_t rows, MSStringList const& result)
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

void MySQLServer::onExit()
{
	ClusterServer::onExit();

	if (m_MysqlPool) m_MysqlPool->shutdown();
	m_MysqlPool = nullptr;
}