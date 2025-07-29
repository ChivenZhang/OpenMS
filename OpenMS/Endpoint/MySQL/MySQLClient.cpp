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
    auto url = std::format("tcp://{}:{}", config.IP, config.PortNum);
    auto connection = driver->connect(url, config.UserName.c_str(), config.Password.c_str());
    auto statement = connection->createStatement();
    auto resultSet = statement->executeQuery("select * from db");
    while(resultSet->next())
    {
        resultSet->getInt("age");
        resultSet->getString("name");
    }
    resultSet->close();
    statement->close();
    connection->close();

    delete connection;
}

void MySQLClient::shutdown()
{
}

bool MySQLClient::running() const
{
    return false;
}

bool MySQLClient::connect() const
{
    return false;
}

MSHnd<IChannelAddress> MySQLClient::address() const
{
    return MSHnd<IChannelAddress>();
}