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
#include "SQLiteServer.h"

// ========================================================================================

MSString SQLiteServer::identity() const
{
	return "sqlite";
}

void SQLiteServer::onInit()
{
	ClusterServer::onInit();
	std::error_code error_code;
	std::filesystem::remove(property(identity() + ".sqlite.database", MSString()), error_code);

	m_SQLiteClient = MSNew<SQLiteClient>(SQLiteClient::config_t{
		.Database = property(identity() + ".sqlite.database", MSString()),
	});
	m_SQLiteClient->startup();

#if 0 // TEST
	MSStringList result;
	m_SQLiteClient->execute("delete from userinfo;", result);
	m_SQLiteClient->execute("create table if not exists userinfo (id integer primary key autoincrement, name text, age integer);", result);
	m_SQLiteClient->execute("insert into userinfo (name, age) values ('Alice', 30);", result);
	m_SQLiteClient->execute("insert into userinfo (name, age) values ('Bob', 25);", result);
	m_SQLiteClient->execute("insert into userinfo (name, age) values ('Charlie', 35);", result);
	result.clear();
	auto rows = m_SQLiteClient->execute("select * from userinfo;", result);
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
#endif
}

void SQLiteServer::onExit()
{
	ClusterServer::onExit();

	if (m_SQLiteClient) m_SQLiteClient->shutdown();
	m_SQLiteClient = nullptr;
}