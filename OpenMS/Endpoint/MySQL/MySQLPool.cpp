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
#include "MySQLPool.h"
#include "Reactor/Private/ChannelAddress.h"
#include <format>
#include <mysql/jdbc.h>

void MySQLPool::startup()
{
	config_t config;
	configureEndpoint(config);
	config.Instance = std::max<uint8_t>(1, config.Instance);
	config.Reconnect = std::max<uint8_t>(1, config.Reconnect);

	auto driver = sql::mysql::get_mysql_driver_instance();
	if (driver == nullptr)
	{
		MS_ERROR("cannot get mysql driver instance");
		return;
	}
	MS_INFO("MySQL Driver： %d.%d.%d", driver->getMajorVersion(), driver->getMinorVersion(), driver->getPatchVersion());

	m_Running = true;
	m_ThreadList.resize(config.Instance);
	for (size_t t = 0; t < m_ThreadList.size(); ++t)
	{
		MSThread thread([=]()
		{
			MSRef<sql::Connection> context;

			while (m_Running)
			{
				// Reconnect mysql database
				if (context == nullptr || context->isValid() == false || context->isClosed() == true)
				{
					if (context) context->close();
					context = nullptr;

					for (size_t i = 0; i < config.Reconnect; ++i)
					{
						try
						{
							auto hostName = std::format("tcp://{}:{}", config.IP, config.PortNum);
							context.reset(driver->connect(hostName, config.UserName, config.Password));
							context->setSchema(config.Database);
							if (context->isValid() && context->isClosed() == false) break;
						}
						catch (MSError& ex)
						{
							MS_ERROR("%s", ex.what());
						}
						context = nullptr;
						std::this_thread::sleep_for(std::chrono::seconds(1));
					}
				}
				if (context == nullptr)
				{
					// Notify other connection
					m_MutexUnlock.notify_one();
					continue;
				}

				execute_t execute;
				if (true)
				{
					MSUniqueLock lock(m_MutexLock);
					while (m_Running && m_ExecuteQueue.empty()) m_MutexUnlock.wait(lock);
					if (m_Running == false) break;
					execute = m_ExecuteQueue.front();
					m_ExecuteQueue.pop();
				}
				auto& sql = execute.Command;
				auto& vars = execute.Variables;
				auto& callback = execute.Callback;

				// Execute sql command

				uint64_t updateNum = 0;
				MSStringList output;
				try
				{
					if (vars.empty())
					{
						MSRef<sql::Statement> statement(context->createStatement());
						if (statement->execute(sql))
						{
							if (auto resultSet = statement->getResultSet())
							{
								updateNum += resultSet->rowsCount();
								auto columnCount = resultSet->getMetaData()->getColumnCount();
								while (resultSet->next())
								{
									for (auto k = 0; k < columnCount; ++k) output.push_back(resultSet->getString(k + 1));
								}
								resultSet->close();
							}
							else
							{
								updateNum += statement->getUpdateCount();
							}
						}
						statement->close();
					}
					else
					{
						MSRef<sql::PreparedStatement> statement(context->prepareStatement(sql));
						size_t paramCount = statement->getParameterMetaData()->getParameterCount();
						for (size_t i = 0; paramCount && i + paramCount <= vars.size(); i += paramCount)
						{
							for (size_t k = 0; k < paramCount; ++k)
							{
								statement->setString(k + 1, vars[i + k]);
							}
							if (statement->execute() == false) break;

							auto columnCount = statement->getMetaData()->getColumnCount();
							if (auto resultSet = statement->getResultSet())
							{
								updateNum += resultSet->rowsCount();
								while (resultSet->next())
								{
									for (auto k = 0; k < columnCount; ++k) output.push_back(resultSet->getString(k + 1));
								}
								resultSet->close();
							}
							else
							{
								updateNum += statement->getUpdateCount();
							}
						}
						statement->close();
					}
				}
				catch (MSError& ex)
				{
					MS_ERROR("%s", ex.what());

					updateNum = -1;
					output.clear();
				}

				// Return result set

				try
				{
					if (callback) callback(updateNum, output);
				}
				catch (MSError& ex)
				{
					MS_ERROR("%s", ex.what());
				}
			}

			if (context) context->close();
			context = nullptr;
		});
		m_ThreadList[t].swap(thread);
	}

	m_Address = IPv4Address::New(MSStringView(config.IP), config.PortNum);
	MS_INFO("accepted from %s:%d", config.IP.c_str(), config.PortNum);
}

void MySQLPool::shutdown()
{
	m_Running = false;
	m_MutexUnlock.notify_all();
	for (auto& thread : m_ThreadList) if (thread.joinable()) thread.join();
	m_ThreadList.clear();

	while (m_ExecuteQueue.size())
	{
		auto execute = m_ExecuteQueue.front();
		m_ExecuteQueue.pop();
		if (execute.Callback) execute.Callback(-1, {});
	}

	MS_INFO("rejected from %s:%d", m_Address->getAddress().c_str(), m_Address->getPort());
	m_Address = nullptr;
}

bool MySQLPool::running() const
{
	return m_Running;
}

bool MySQLPool::connect() const
{
	return m_Running && m_ThreadList.size();
}

MSHnd<IChannelAddress> MySQLPool::address() const
{
	return m_Address;
}

bool MySQLPool::execute(MSString const& sql, MSLambda<void(uint64_t update, MSStringList const& data)> result)
{
	if (connect() == false) return false;
	{
		MSMutexLock lock(m_MutexLock);
		m_ExecuteQueue.emplace(sql, MSStringList{}, result);
	}
	m_MutexUnlock.notify_one();
	return true;
}

bool MySQLPool::prepare(MSString const& sql, MSStringList const& vars, MSLambda<void(uint64_t update, MSStringList const& data)> result)
{
	if (connect() == false) return false;
	{
		MSMutexLock lock(m_MutexLock);
		m_ExecuteQueue.emplace(sql, vars, result);
	}
	m_MutexUnlock.notify_one();
	return true;
}