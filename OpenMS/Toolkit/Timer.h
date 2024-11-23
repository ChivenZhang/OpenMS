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
#include "MS.h"
#include <uv.h>

class Timer final
{
public:
	using task_t = TLambda<void()>;

public:
	Timer();
	~Timer();
	void update();
	uint32_t start(uint64_t timeout, uint64_t repeat, task_t task);
	bool stop(uint32_t timer);

private:
	struct timer_t
	{
		uint32_t ID;
		task_t Method;
		uv_timer_t Handle;
	};
	uv_loop_t m_Loop;
	uint32_t m_TimerID;
	TMap<uint32_t, timer_t> m_Timers;
};