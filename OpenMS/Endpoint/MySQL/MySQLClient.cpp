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
#include "MySQLClient.h"
#include <format>

void MySQLClient::startup()
{
	config_t config;
	configureEndpoint(config);

	auto driver = sql::mysql::get_mysql_driver_instance();
	MS_INFO("MySQL Driver： %d.%d.%d", driver->getMajorVersion(), driver->getMinorVersion(), driver->getPatchVersion());

	auto hostName = std::format("tcp://{}:{}", config.IP, config.PortNum);
	m_Connection.reset(driver->connect(hostName, config.UserName, config.Password));
	MS_INFO("accepted from %s:%d", config.IP.c_str(), config.PortNum);
	m_Connection->setSchema(config.Database);
}

void MySQLClient::shutdown()
{
	if (m_Connection) m_Connection->close();
	m_Connection = nullptr;
}

bool MySQLClient::running() const
{
	if (m_Connection == nullptr) return false;
	return true;
}

bool MySQLClient::connect() const
{
	if (m_Connection == nullptr) return false;
	return m_Connection->isValid() && m_Connection->isClosed() == false;
}

MSHnd<IChannelAddress> MySQLClient::address() const
{
	return MSHnd<IChannelAddress>();
}

bool MySQLClient::query(MSString const& sql, MSStringList& names, MSStringList& result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		MSRef<sql::ResultSet> resultSet(statement->executeQuery(sql));
		auto metaData = resultSet->getMetaData();
		auto columnCount = metaData->getColumnCount();
		for (auto i = 1; i <= columnCount; ++i)
		{
			names.push_back(metaData->getColumnName(i));
		}
		while (resultSet->next())
		{
			for (auto i = 1; i <= columnCount; ++i)
			{
				result.push_back(resultSet->getString(i));
			}
		}
		resultSet->close();
		statement->close();
		return true;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return false;
}

bool MySQLClient::query(MSString const& sql, MSStringList& names, MSList<type_t>& types, MSStringList& result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		MSRef<sql::ResultSet> resultSet(statement->executeQuery(sql));
		auto metaData = resultSet->getMetaData();
		auto columnCount = metaData->getColumnCount();
		for (auto i = 1; i <= columnCount; ++i)
		{
			names.push_back(metaData->getColumnName(i));
		}
		for (auto i = 1; i <= columnCount; ++i)
		{
			types.push_back((type_t)metaData->getColumnType(i));
		}
		while (resultSet->next())
		{
			for (auto i = 1; i <= columnCount; ++i)
			{
				result.push_back(resultSet->getString(i));
			}
		}
		resultSet->close();
		statement->close();
		return true;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return false;
}

bool MySQLClient::insert(MSString const& sql, MSStringList const& data)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		auto result = statement->executeUpdate(sql);
		statement->close();
		return true;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return false;
}

uint32_t MySQLClient::update(MSString const& sql)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		auto result = statement->executeUpdate(sql);
		statement->close();
		return true;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return false;
}
