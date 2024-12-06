#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include "MS.h"
#include <uv.h>

class Timer final
{
public:
	using task_t = MSLambda<void(uint32_t handle)>;

public:
	Timer();
	~Timer();
	uint32_t start(uint64_t timeout, uint64_t repeat, task_t task);
	bool stop(uint32_t handle);

private:
	struct timer_t
	{
		task_t Method;
		uv_timer_t Handle;
		uint32_t ID; uint64_t Timeout, Repeat;
	};
	struct clause_t
	{
		Timer::timer_t& Timer;
		MSPromise<void>& Promise;
	};
	MSThread m_Thread;
	uint32_t m_TimerID;
	MSMap<uint32_t, timer_t> m_Timers;
	MSRaw<uv_loop_t> m_Loop;
	MSRaw<uv_async_t> m_AsyncExit, m_AsyncStart, m_AsyncStop;
};