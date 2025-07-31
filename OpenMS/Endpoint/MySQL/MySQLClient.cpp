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
#include <mysql/jdbc.h>

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
	MS_INFO("rejected from %s:%d", config.IP.c_str(), config.PortNum);
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

uint64_t MySQLClient::execute(MSString const &sql)
{
	try
	{
		if (connect() == false) return 0;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		if(statement->execute(sql) == false) return false;
		auto result = statement->getUpdateCount();
		statement->close();
		return result;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::execute(MSString const &sql, MSStringList &names, MSStringList &result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		MSRef<sql::ResultSet> resultSet(statement->executeQuery(sql));
		auto metaData = resultSet->getMetaData();
		auto updateCount = resultSet->rowsCount();
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
		return updateCount;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::execute(MSString const &sql, MSStringList &names, MSList<type_t> &types, MSStringList &result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::Statement> statement(m_Connection->createStatement());
		MSRef<sql::ResultSet> resultSet(statement->executeQuery(sql));
		auto metaData = resultSet->getMetaData();
		auto updateCount = resultSet->rowsCount();
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
		return updateCount;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::prepare(MSString const &sql)
{
	try
	{
		if (connect() == false) return 0;
		MSRef<sql::PreparedStatement> statement(m_Connection->prepareStatement(sql));
		if(statement->execute() == false) return false;
		auto result = statement->getUpdateCount();
		statement->close();
		return result;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::prepare(MSString const &sql, MSList<type_t> const& types, MSStringList const &data)
{
	try
	{
		if (connect() == false) return 0;
		MSRef<sql::PreparedStatement> statement(m_Connection->prepareStatement(sql));
		uint64_t result = 0;
		size_t columns = types.size();
		for(size_t i = 0; columns && i + columns <= types.size(); i+=columns)
		{
			for(size_t k = 0; k < columns; ++k)
			{
				switch(types[k])
				{
				case sql::DataType::BIT:
				case sql::DataType::TINYINT:
				case sql::DataType::SMALLINT:
				case sql::DataType::MEDIUMINT:
				case sql::DataType::INTEGER:
					statement->setInt(k+1, std::stoi(data[i+k]));
					break;
				case sql::DataType::BIGINT:
					statement->setInt64(k+1, std::stoll(data[i+k]));
					break;
				case sql::DataType::REAL:
				case sql::DataType::DOUBLE:
				case sql::DataType::DECIMAL:
				case sql::DataType::NUMERIC:
					statement->setDouble(k+1, std::stod(data[i+k]));
					break;
				case sql::DataType::CHAR:
				case sql::DataType::VARCHAR:
				case sql::DataType::LONGVARCHAR:
					statement->setString(k+1, data[i+k]);
					break;
				case sql::DataType::BINARY:
				case sql::DataType::VARBINARY:
				case sql::DataType::LONGVARBINARY:
					statement->setBlob(k+1, std::make_shared<std::istringstream>(data[i+k]));
					break;
				default:
					statement->setString(k+1, data[i+k]);
					break;
				}
			}
			result += statement->executeUpdate();
		}
		statement->close();
		return result;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::prepare(MSString const& sql, MSStringList& names, MSStringList& result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::PreparedStatement> statement(m_Connection->prepareStatement(sql));
		MSRef<sql::ResultSet> resultSet(statement->executeQuery());
		auto metaData = resultSet->getMetaData();
		auto updateCount = resultSet->rowsCount();
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
		return updateCount;
	}
	catch (sql::SQLException& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}

uint64_t MySQLClient::prepare(MSString const& sql, MSStringList& names, MSList<type_t>& types, MSStringList& result)
{
	try
	{
		if (connect() == false) return false;
		MSRef<sql::PreparedStatement> statement(m_Connection->prepareStatement(sql));
		MSRef<sql::ResultSet> resultSet(statement->executeQuery());
		auto metaData = resultSet->getMetaData();
		auto updateCount = resultSet->rowsCount();
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
		return updateCount;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s", ex.what());
	}
	return -1;
}