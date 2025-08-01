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
#include "RedisClient.h"
#include "Reactor/Private/ChannelAddress.h"
#include <hiredis/hiredis.h>

void RedisClient::startup()
{
	config_t config;
	configureEndpoint(config);

	auto context = redisConnect(config.IP.c_str(), config.PortNum);
	if(context == nullptr)
	{
		MS_ERROR("cannot connect redis");
		return;
	}
	if(context->err)
	{
		redisFree(context);
		MS_ERROR("%s", context->errstr);
		return;
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
			redisFree(context);
			return;
		}
		if(result->type == REDIS_REPLY_ERROR)
		{
			MS_ERROR("%s", result->str);
			freeReplyObject(result);
			redisFree(context);
			return;
		}
	}
	else
	{
		// Do Nothing
	}

	m_Context = context;
	m_Address = IPv4Address::New(MSStringView(config.IP), config.PortNum);

	MS_INFO("accepted from %s:%d", m_Address->getAddress().c_str(), m_Address->getPort());
}

void RedisClient::shutdown()
{
	if(m_Context) redisFree(m_Context);
	m_Context = nullptr;

	MS_INFO("rejected from %s:%d", m_Address->getAddress().c_str(), m_Address->getPort());
	m_Address = nullptr;
}

bool RedisClient::running() const
{
	if(m_Context == nullptr) return false;
	return true;
}

bool RedisClient::connect() const
{
	if(m_Context == nullptr) return false;
	if(m_Context->err) return false;
	return true;
}

MSHnd<IChannelAddress> RedisClient::address() const
{
	return m_Address;
}

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

bool RedisClient::execute(MSString const &cmd, MSString& output)
{
	if(connect() == false) return false;
	auto result = (redisReply*)redisCommand(m_Context, cmd.c_str());
	if(result == nullptr)
	{
		return false;
	}
	if(result->type == REDIS_REPLY_ERROR)
	{
		MS_ERROR("%s", result->str);
		return false;
	}
	output = RedisParseResult(result);
	return true;
}