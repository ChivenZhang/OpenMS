#pragma once
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
#include "OpenMS/Service/Private/Property.h"
#include "OpenMS/Toolkit/Timer.h"
#include "AuthorityServer.h"

class AuthorityConfig :
	public RESOURCE2(Property, IProperty),
	public RESOURCE2N(Value, IValue, "iptable"),
	public RESOURCE2N(Value, IValue, "service"),
	public RESOURCE(AuthorityServer),
	public RESOURCE(AuthorityClient),
	public RESOURCE(Timer)
{
};