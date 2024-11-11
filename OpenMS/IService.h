#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "MS.h"

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

};

/// @brief Interface for provider service
class OPENMS_API IProviderService : public IService
{
public:

};

/// @brief Interface for comsumer service
class OPENMS_API IComsumerService : public IService
{
public:

};