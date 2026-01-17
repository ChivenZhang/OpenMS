/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
* 
* 
* ====================History=======================
* Created by chivenzhang@gmail.com.
* 
* =================================================*/
#include "SQLiteClient.h"
#include "Reactor/Private/ChannelAddress.h"
#include <sqlite3.h>

SQLiteClient::SQLiteClient(config_t const& config)
	:
	m_Config(config),
	m_Context(nullptr)
{
}

void SQLiteClient::startup()
{
	auto config = m_Config;
	
	if (sqlite3_open(config.Database.c_str(), &m_Context) != SQLITE_OK)
	{
		MS_ERROR("cannot open sqlite database: %s", sqlite3_errmsg(m_Context));
	}
}

void SQLiteClient::shutdown()
{
	if (m_Context) sqlite3_close(m_Context);
	m_Context = nullptr;
}

bool SQLiteClient::running() const
{
	if (m_Context == nullptr) return false;
	return true;
}

bool SQLiteClient::connect() const
{
	if (m_Context == nullptr) return false;
	return true;
}

MSHnd<IChannelAddress> SQLiteClient::address() const
{
	return {};
}

uint64_t SQLiteClient::execute(MSString const &sql, MSStringList &output)
{
	if (connect() == false) return -1;
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(m_Context, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
	{
		uint64_t result = 0;
		int state = SQLITE_ERROR;
		while ((state = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			int columnCount = sqlite3_column_count(stmt);
			for (int k = 0; k < columnCount; ++k)
			{
				auto data = sqlite3_column_text(stmt, k);
				auto length = sqlite3_column_bytes(stmt, k);
				output.emplace_back((char*)data, length);
			}
			result += 1;
		}
		if (state == SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			return result;
		}
		sqlite3_finalize(stmt);
		MS_ERROR("cannot execute statement: %s as %s", sql.c_str(), sqlite3_errmsg(m_Context));
		return -1;
	}
	MS_ERROR("cannot prepare statement: %s", sqlite3_errmsg(m_Context));
	return -1;
}

uint64_t SQLiteClient::prepare(MSString const &sql, MSStringList const& params, MSStringList& output)
{
	if (connect() == false) return -1;
	sqlite3_stmt *stmt = nullptr;
	if (sqlite3_prepare_v2(m_Context, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
	{
		for (size_t k = 0; k < params.size(); ++k)
		{
			sqlite3_bind_text(stmt, static_cast<int>(k + 1), params[k].c_str(), -1, SQLITE_STATIC);
		}
		uint64_t result = 0;
		int state = SQLITE_ERROR;
		while ((state = sqlite3_step(stmt)) == SQLITE_ROW)
		{
			int columnCount = sqlite3_column_count(stmt);
			for (int k = 0; k < columnCount; ++k)
			{
				auto data = sqlite3_column_text(stmt, k);
				auto length = sqlite3_column_bytes(stmt, k);
				output.emplace_back((char*)data, length);
			}
			result += 1;
		}
		if (state == SQLITE_DONE)
		{
			sqlite3_finalize(stmt);
			return result;
		}
		sqlite3_finalize(stmt);
		MS_ERROR("cannot execute statement: %s as %s", sql.c_str(), sqlite3_errmsg(m_Context));
		return -1;
	}
	MS_ERROR("cannot prepare statement: %s", sqlite3_errmsg(m_Context));
	return -1;
}