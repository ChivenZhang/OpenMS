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
#include "MS.h"

class OPENMS_API IServiceAddress
{
public:
	static MSString ToString(MSStringView ip, MSStringView port, MSStringView name)
	{
		return std::format("{}:{}:{}", ip, port, name);
	}

	static MSTuple<MSStringView, MSStringView, MSStringView> FromString(MSStringView address)
	{
		auto first_colon = address.find(':');
		if (first_colon == MSStringView::npos) return {};
		auto second_colon = address.find(':', first_colon + 1);
		if (second_colon == MSStringView::npos) return {};
		auto ip = address.substr(0, first_colon);
		auto port = address.substr(first_colon + 1, second_colon - first_colon - 1);
		auto name = address.substr(second_colon + 1);
		return { ip, port, name };
	}
};

/// @brief Interface for service
class OPENMS_API IService
{
	// TODO:
};