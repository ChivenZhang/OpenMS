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
#include "RedisPool.h"
#include "Reactor/Private/ChannelAddress.h"
#include <hiredis/hiredis.h>

static MSString RedisParseResult(MSRaw<redisReply> reply)
{
	switch(reply->type)
	{
	case REDIS_REPLY_STATUS:
		{
			return MSString(reply->str, reply->len);
		} break;
	case REDIS_REPLY_ERROR:
		{
			return MSString(reply->str, reply->len);
		} break;
	case REDIS_REPLY_INTEGER:
		{
			return std::to_string(reply->integer);
		} break;
	case REDIS_REPLY_STRING:
		{
			return MSString(reply->str, reply->len);
		} break;
	case REDIS_REPLY_ARRAY:
		{
			nlohmann::json json;
			for(size_t i=0; i<reply->elements; ++i)
			{
				json[i] = RedisParseResult(reply->element[i]);
			}
			return json.dump();
		} break;
	case REDIS_REPLY_DOUBLE:
		{
			return MSString(reply->str, reply->len);
		} break;
	case REDIS_REPLY_BOOL:
		{
			return std::to_string(reply->integer);
		} break;
	case REDIS_REPLY_MAP:
		{
			nlohmann::json json;
			for(size_t i=0; i+2<=reply->elements; i+=2)
			{
				json[RedisParseResult(reply->element[i])] = RedisParseResult(reply->element[i+1]);
			}
			return json.dump();
		} break;
	case REDIS_REPLY_SET:
		{
			nlohmann::json json;
			for(size_t i=0; i<reply->elements; ++i)
			{
				json[i] = RedisParseResult(reply->element[i]);
			}
			return json.dump();
		} break;
	case REDIS_REPLY_PUSH:
		{
			nlohmann::json json;
			for(size_t i=0; i<reply->elements; ++i)
			{
				json[i] = RedisParseResult(reply->element[i]);
			}
			return json.dump();
		} break;
	case REDIS_REPLY_BIGNUM:
		{
			return MSString(reply->str, reply->len);
		} break;
	case REDIS_REPLY_VERB:
		{
			return MSString(reply->vtype, 3) + ":" + MSString(reply->str, reply->len);
		} break;
	default:
		{
			return MSString(reply->str, reply->len);
		} break;
	}
}

RedisPool::RedisPool(config_t const& config)
	:
	m_Config(config)
{
}

void RedisPool::startup()
{
	auto config = m_Config;
	config.Instance = std::max<uint8_t>(1, config.Instance);
	config.Reconnect = std::max<uint8_t>(1, config.Reconnect);

	m_Running = true;
	m_ThreadList.resize(config.Instance);
	for (size_t t = 0; t < m_ThreadList.size(); ++t)
	{
		MSThread thread([=]()
		{
			MSRaw<redisContext> context = nullptr;

			while (m_Running)
			{
				// Reconnect redis database
				if (context == nullptr || context->err)
				{
					if (context) redisFree(context);
					context = nullptr;

					for (size_t i = 0; i < config.Reconnect; ++i)
					{
						try
						{
							do
							{
								context = redisConnect(config.IP.c_str(), config.PortNum);
								if(context == nullptr)
								{
									MS_ERROR("cannot connect redis");
									break;
								}
								if(context->err)
								{
									MS_ERROR("%s", context->errstr);
									redisFree(context); context = nullptr;
									break;
								}
								if(config.UserName.size())
								{
									// TODO: Auth by username
								}
								else if(config.Password.size())
								{
									auto result = (redisReply*)redisCommand(context, "AUTH %s", config.Password.c_str());
									if(result == nullptr)
									{
										MS_ERROR("cannot to auth");
										redisFree(context); context = nullptr;
										break;
									}
									if(result->type == REDIS_REPLY_ERROR)
									{
										MS_ERROR("%s", result->str);
										freeReplyObject(result);
										redisFree(context); context = nullptr;
										break;
									}
									freeReplyObject(result);
								}
								else
								{
									// Do Nothing
								}
							} while (false);

							if (context && context->err == REDIS_OK) break;
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
				auto& command = execute.Command;
				auto& callback = execute.Callback;

				// Execute redis command

				bool updateNum = false;
				MSString output;
				try
				{
					if(connect() == false) return false;
					auto result = (redisReply*)redisCommand(context, command.c_str());
					if(result == nullptr)
					{
						updateNum = false;
					}
					else if(result->type == REDIS_REPLY_ERROR)
					{
						MS_ERROR("%s", result->str);
						freeReplyObject(result);
						updateNum = false;
					}
					else
					{
						updateNum = true;
						output = RedisParseResult(result);
					}
					freeReplyObject(result);
				}
				catch (MSError& ex)
				{
					MS_ERROR("%s", ex.what());

					updateNum = false;
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

			if (context) redisFree(context);
			context = nullptr;
		});
		m_ThreadList[t].swap(thread);
	}

	m_Address = IPv4Address::New(MSStringView(config.IP), config.PortNum);
	MS_INFO("accepted from %s:%d", config.IP.c_str(), config.PortNum);
}

void RedisPool::shutdown()
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

bool RedisPool::running() const
{
	return m_Running;
}

bool RedisPool::connect() const
{
	return m_Running && m_ThreadList.size();
}

MSHnd<IChannelAddress> RedisPool::address() const
{
	return m_Address;
}

bool RedisPool::execute(MSString const& command, MSLambda<void(bool result, MSString const& data)> result)
{
	if (connect() == false) return false;
	{
		MSMutexLock lock(m_MutexLock);
		m_ExecuteQueue.emplace(command, result);
	}
	m_MutexUnlock.notify_one();
	return true;
}