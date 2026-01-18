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
#include "Reactor/Private/ChannelAddress.h"
#include <format>
#include <mysql/jdbc.h>

MySQLClient::MySQLClient(config_t const& config)
	:
	m_Config(config)
{
}

void MySQLClient::startup()
{
	auto config = m_Config;
	
	auto driver = sql::mysql::get_mysql_driver_instance();
	MS_INFO("MySQL Driver： %d.%d.%d", driver->getMajorVersion(), driver->getMinorVersion(), driver->getPatchVersion());
	auto hostName = std::format("tcp://{}:{}", config.IP, config.PortNum);
	m_Context.reset(driver->connect(hostName, config.UserName, config.Password));
	m_Context->setSchema(config.Database);

	m_Address = IPv4Address::New(MSStringView(config.IP), config.PortNum);
	MS_INFO("accepted from %s:%d", config.IP.c_str(), config.PortNum);
}

void MySQLClient::shutdown()
{
	if (m_Context) m_Context->close();
	m_Context = nullptr;

	MS_INFO("rejected from %s:%d", m_Address->getAddress().c_str(), m_Address->getPort());
	m_Address = nullptr;
}

bool MySQLClient::running() const
{
	if (m_Context == nullptr) return false;
	return true;
}

bool MySQLClient::connect() const
{
	if (m_Context == nullptr) return false;
	return m_Context->isValid() && m_Context->isClosed() == false;
}

MSHnd<IChannelAddress> MySQLClient::address() const
{
	return m_Address;
}

uint64_t MySQLClient::execute(MSString const &sql, MSStringList &output)
{
	try
	{
		if (connect() == false) return -1;
		MSRef<sql::Statement> statement(m_Context->createStatement());
		uint64_t result = 0;
		if (statement->execute(sql))
		{
			if (auto resultSet = statement->getResultSet())
			{
				result += resultSet->rowsCount();
				auto columnCount = resultSet->getMetaData()->getColumnCount();
				while (resultSet->next())
				{
					for (auto k = 0; k < columnCount; ++k)
					{
						output.push_back(resultSet->getString(k + 1));
					}
				}
				resultSet->close();
			}
			else
			{
				result += statement->getUpdateCount();
			}
		}
		statement->close();
		return result;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s: %s", sql.c_str(), ex.what());
	}
	return -1;
}

uint64_t MySQLClient::prepare(MSString const &sql, MSStringList const& params, MSStringList& output)
{
	try
	{
		if (connect() == false) return -1;
		MSRef<sql::PreparedStatement> statement(m_Context->prepareStatement(sql));
		uint64_t result = 0;
		size_t paramCount = statement->getParameterMetaData()->getParameterCount();
		for (size_t i = 0; paramCount && i + paramCount <= params.size(); i += paramCount)
		{
			for (size_t k = 0; k < paramCount; ++k)
			{
				statement->setString(k + 1, params[i + k]);
			}
			if (statement->execute() == false) break;

			auto columnCount = statement->getMetaData()->getColumnCount();
			if (auto resultSet = statement->getResultSet())
			{
				result += resultSet->rowsCount();
				while (resultSet->next())
				{
					for (auto k = 0; k < columnCount; ++k)
					{
						output.push_back(resultSet->getString(k + 1));
					}
				}
				resultSet->close();
			}
			else
			{
				result += statement->getUpdateCount();
			}
		}
		statement->close();
		return result;
	}
	catch (MSError& ex)
	{
		MS_ERROR("%s: %s", sql.c_str(), ex.what());
	}
	return -1;
}
