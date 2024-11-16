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
#define OPENMS_CONFIG_FILE "application.json"

/// @brief Interface for service
class OPENMS_API IService
{
public:
	virtual ~IService() = default;

	virtual void startup(int argc, char** argv) = 0;

	virtual void shutdown() = 0;
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