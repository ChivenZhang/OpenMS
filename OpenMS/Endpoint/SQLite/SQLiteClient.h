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
struct sqlite3;

/// @brief SQLite Client Endpoint
class SQLiteClient : public IEndpoint
{
public:
	struct config_t
	{
		MSString Database;
	};

public:
	explicit SQLiteClient(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	uint64_t execute(MSString const& sql, MSStringList& result);
	uint64_t prepare(MSString const& sql, MSStringList const& params, MSStringList& result);

protected:
	const config_t m_Config;
	MSRaw<sqlite3> m_Context;
};
