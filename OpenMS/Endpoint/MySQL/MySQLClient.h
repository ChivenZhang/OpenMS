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

	uint64_t execute(MSString const& sql);

	uint64_t execute(MSString const& sql, MSStringList& names, MSStringList& result);

	uint64_t execute(MSString const& sql, MSStringList& names, MSList<type_t>& types, MSStringList& result);

	uint64_t prepare(MSString const& sql);

	uint64_t prepare(MSString const& sql, MSList<type_t> const& types, MSStringList const& data);

	uint64_t prepare(MSString const& sql, MSStringList& names, MSStringList& result);

	uint64_t prepare(MSString const& sql, MSStringList& names, MSList<type_t>& types, MSStringList& result);

	template<class T>
	uint64_t query(MSString const& sql, MSList<T>& result)
	{
		MSList<type_t> types;
		MSStringList names, output;
		auto updateCount = prepare(sql, names, types, output);
		if(updateCount == -1) return updateCount;
		auto columns = names.size();
		for(size_t i = 0; columns && i + columns <= output.size(); i += columns)
		{
			nlohmann::json json;
			for(size_t k=0; k < columns; ++k)
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
		return updateCount;
	}

	template<class T>
	uint64_t update(MSString const& sql, MSStringList const& names, MSList<type_t> const& types, MSList<T> const& data)
	{
		MSStringList input;
		auto columns = names.size();
		for(size_t i=0; i<data.size(); ++i)
		{
			nlohmann::json json = nlohmann::to_json(data[i]);
			for(size_t k = 0; k < columns; ++k)
			{
				input.emplace_back(json[names[k]].get<MSString>());
			}
		}
		return prepare(sql, types, input);
	}

protected:
	virtual void configureEndpoint(config_t& config) = 0;

protected:
	MSRef<ISocketAddress> m_Address;
	MSRef<sql::Connection> m_Connection;
};
