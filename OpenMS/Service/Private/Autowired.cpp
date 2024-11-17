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
#include "../Autowired.h"

TRaw<BeanManager> BeanManager::Instance()
{
	static BeanManager s_Instance;
	return &s_Instance;
}