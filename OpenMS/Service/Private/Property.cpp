#include "Property.h"
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "Property.h"
#include "../IEnvironment.h"
#include <fstream>
#include <filesystem>
#define OPENMS_CONFIG_FILE "application.json"

TString Value::value() const
{
	return m_Value;
}

void Value::setValue(TString const& value)
{
	m_Value = value;
}

Property::Property()
{
	auto argc = IEnvironment::argc;
	auto argv = IEnvironment::argv;

	// Get config file path

	auto directory = std::filesystem::path(argv[0]).parent_path().generic_string();
	auto configFile = directory + "/" + OPENMS_CONFIG_FILE;
	if (std::filesystem::exists(configFile) == false) TFatal("config file not found: %s", configFile.c_str());

	// Load config file

	std::ifstream ifs(configFile);
	if (!ifs.is_open()) TFatal("Failed to open config file: %s", configFile.c_str());
	auto text = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();

	// Parse config file

	TLambda<void(TString const&, uint32_t, nlohmann::ordered_json&)> parse_func;
	parse_func = [&](TString const& name, uint32_t depth, nlohmann::ordered_json& value) {
		TString _value;
		switch (value.type())
		{
		case nlohmann::ordered_json::value_t::array:
			m_PropertyMap[name] = value.dump();
			for (size_t i = 0, n = value.size(); i < n; ++i)
			{
				auto& raw = value.at(i);
				if (depth == 0)
					parse_func("[" + std::to_string(i) + "]", depth + 1, raw);
				else
					parse_func(name + "[" + std::to_string(i) + "]", depth + 1, raw);
			}
			break;
		case nlohmann::ordered_json::value_t::object:
			m_PropertyMap[name] = value.dump();
			for (auto field : value.items())
			{
				auto& raw = field.value();
				if (depth == 0)
					parse_func(field.key(), depth + 1, raw);
				else
					parse_func(name + "." + field.key(), depth + 1, raw);
			}
			break;
		case nlohmann::ordered_json::value_t::boolean:
			m_PropertyMap[name] = TTextC<bool>::to_string(value.get<bool>());
			break;
		case nlohmann::ordered_json::value_t::string:
			m_PropertyMap[name] = value.get<std::string>();
			break;
		case nlohmann::ordered_json::value_t::number_float:
			m_PropertyMap[name] = TTextC<float>::to_string(value.get<float>());
			break;
		case nlohmann::ordered_json::value_t::number_integer:
			m_PropertyMap[name] = TTextC<int32_t>::to_string(value.get<int32_t>());
			break;
		case nlohmann::ordered_json::value_t::number_unsigned:
			m_PropertyMap[name] = TTextC<uint32_t>::to_string(value.get<uint32_t>());
			break;
		}
		};
	auto document = nlohmann::ordered_json::parse(text, nullptr, false, true);
	parse_func(TString(), 0, document);

	for (auto& value : m_PropertyMap)
	{
		RESOURCE2_DATA(Value, IValue, value.first)->setValue(value.second);
	}
}

TString Property::property(TStringView name) const
{
	auto result = m_PropertyMap.find(TString(name));
	if (result == m_PropertyMap.end()) return TString();
	return result->second;
}