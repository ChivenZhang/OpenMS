#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/MS.h>
#include "User.h"

class UserManager
{
public:
	MSHnd<User> getUser(uint32_t uid) const
	{
		auto result = m_Users.find(uid);
		if (result == m_Users.end()) return MSHnd<User>();
		return result->second;
	}

	void setUser(uint32_t uid, MSRef<User> user)
	{
		if (user) m_Users[uid] = user;
		else m_Users.erase(uid);
	}

protected:
	MSMap<uint32_t, MSRef<User>> m_Users;
};