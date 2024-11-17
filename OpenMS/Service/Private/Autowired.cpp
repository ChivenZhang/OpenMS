/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "../Autowired.h"

TRaw<BeanManager> BeanManager::Instance()
{
	static BeanManager s_Instance;
	return &s_Instance;
}