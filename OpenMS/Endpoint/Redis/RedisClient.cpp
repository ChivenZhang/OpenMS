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

void RedisClient::startup()
{
}

void RedisClient::shutdown()
{
}

bool RedisClient::running() const
{
    return false;
}

bool RedisClient::connect() const
{
    return false;
}

MSHnd<IChannelAddress> RedisClient::address() const
{
    return MSHnd<IChannelAddress>();
}