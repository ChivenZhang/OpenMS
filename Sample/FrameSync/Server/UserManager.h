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
#include <OpenMS/MS.h>
#include "User.h"

class UserManager
{
public:
	THnd<User> getUser(uint32_t uid) const
	{
		auto result = m_Users.find(uid);
		if (result == m_Users.end()) return THnd<User>();
		return result->second;
	}

	void setUser(uint32_t uid, TRef<User> user)
	{
		if (user) m_Users[uid] = user;
		else m_Users.erase(uid);
	}

protected:
	TMap<uint32_t, TRef<User>> m_Users;
};