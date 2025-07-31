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

	enum type_t
	{
		UNKNOWN = 0,
		BIT,
		TINYINT,
		SMALLINT,
		MEDIUMINT,
		INTEGER,
		BIGINT,
		REAL,
		DOUBLE,
		DECIMAL,
		NUMERIC,
		CHAR,
		BINARY,
		VARCHAR,
		VARBINARY,
		LONGVARCHAR,
		LONGVARBINARY,
		TIMESTAMP,
		DATE,
		TIME,
		YEAR,
		GEOMETRY,
		ENUM,
		SET,
		SQLNULL,
		JSON,
		VECTOR,
	};

public:
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	bool query(MSString const& sql, MSStringList& names, MSStringList& result);

	bool query(MSString const& sql, MSStringList& names, MSList<type_t>& types, MSStringList& result);

	bool insert(MSString const& sql, MSStringList const& data);

	uint32_t update(MSString const& sql);

	template<class T>
	bool query(MSString const& sql, MSList<T>& result)
	{
		MSList<type_t> types;
		MSStringList names, output;
		if (query(sql, names, types, output) == false) return false;
		auto columns = names.size();
		for(size_t i = 0; columns && i + columns <= output.size(); i += columns)
		{
			nlohmann::json json;
			for(size_t k=0; k<columns; ++k)
			{
				switch(types[k])
				{
				case sql::DataType::BIT:
				case sql::DataType::TINYINT:
				case sql::DataType::SMALLINT:
				case sql::DataType::MEDIUMINT:
				case sql::DataType::INTEGER:
				{
					int32_t value = 0;
					TTypeC(output[i+k], value);
					json[names[k]] = value;
				} break;
				case sql::DataType::BIGINT:
				{
					int64_t value = 0;
					TTypeC(output[i+k], value);
					json[names[k]] = value;
				} break;
				case sql::DataType::REAL:
				case sql::DataType::DOUBLE:
				case sql::DataType::DECIMAL:
				case sql::DataType::NUMERIC:
				{
					double value = 0;
					TTypeC(output[i+k], value);
					json[names[k]] = value;
				} break;
				case sql::DataType::CHAR:
				case sql::DataType::VARCHAR:
				case sql::DataType::LONGVARCHAR:
				{
					json[names[k]] = output[i+k];
				} break;
				case sql::DataType::BINARY:
				case sql::DataType::VARBINARY:
				case sql::DataType::LONGVARBINARY:
				{
					json[names[k]] = MSList<uint8_t>(output[i+k].begin(), output[i+k].end());
				} break;
				default:
				{
					json[names[k]] = output[i+k];
				} break;
				}
			}

			auto& object = result.emplace_back();
			json.get_to(object);
		}
		return true;
	}

protected:
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	MSRef<sql::Connection> m_Connection;
};
