#pragma once
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
#include "OpenMS/Endpoint/IEndpoint.h"
#include <mysql/jdbc.h>

class MySQLClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum = 3306;
		MSString UserName;
		MSString Password;
		MSString Database;
	};

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool query(MSString const& sql, MSStringList& names, MSStringList& result);

	bool insert(MSString const& sql, MSStringList const& result)
	{
		return false;
	}

	uint32_t update(MSString const& sql)
	{
		return 0;
	}

protected:
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	MSRef<sql::Connection> m_Connection;
};
