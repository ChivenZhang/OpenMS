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
#include "Endpoint/IEndpoint.h"

namespace sql
{
	class Connection;
}

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
	explicit MySQLClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	uint64_t execute(MSString const& sql, MSStringList& result);
	uint64_t prepare(MSString const& sql, MSStringList const& vars, MSStringList& result);

protected:
	const config_t m_Config;
	MSRef<ISocketAddress> m_Address;
	MSRef<sql::Connection> m_Context;
};
