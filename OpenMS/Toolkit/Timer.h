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
	using task_t = TLambda<void(uint32_t timer)>;

public:
	Timer();
	~Timer();
	uint32_t start(uint64_t timeout, uint64_t repeat, task_t task);
	bool stop(uint32_t timer);

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
		TPromise<void>& Promise;
	};
	TThread m_Thread;
	uint32_t m_TimerID;
	TMap<uint32_t, timer_t> m_Timers;
	TRaw<uv_loop_t> m_Loop;
	TRaw<uv_async_t> m_AsyncExit, m_AsyncStart, m_AsyncStop;
};