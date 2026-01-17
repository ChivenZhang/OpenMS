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
#include "StorageServer.h"
#include "UserService.h"

// ========================================================================================

MSString StorageServer::identity() const
{
	return "storage";
}

void StorageServer::onInit()
{
	ClusterServer::onInit();
	auto mailHub = AUTOWIRE(IMailHub)::bean();

	m_SQLiteClient = MSNew<SQLiteClient>(SQLiteClient::config_t{
		.Database = property(identity() + ".sqlite.database", MSString()),
	});
	m_SQLiteClient->startup();

	// ========================================================================================
	{
		MSStringList result;
		m_SQLiteClient->execute("create table if not exists user(id integer primary key autoincrement, username text unique, password text, created_at datetime default current_timestamp);", result);
		m_SQLiteClient->execute("insert into user(username, password) values('admin', '123456')", result);
	}
	// ========================================================================================

	auto userService = MSNew<UserService>();
	userService->bind("loginDB", [=, this](MSString username, MSString password)->MSAsync<uint32_t>
	{
		MS_INFO("查询用户：%s", username.data());

		MSStringList result;
		auto rowNum = m_SQLiteClient->prepare("select id from user where username = ? and password = ? limit 1;", { username, password }, result);
		if (rowNum != -1 && rowNum > 0) co_return std::stoul(result[0]);
		co_return 0U;
	});
	mailHub->create("userdb", userService);
}

void StorageServer::onExit()
{
	ClusterServer::onExit();

	if (m_SQLiteClient) m_SQLiteClient->shutdown();
	m_SQLiteClient = nullptr;
}