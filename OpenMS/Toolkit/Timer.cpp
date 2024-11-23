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
#include "Timer.h"

Timer::Timer()
	:
	m_TimerID(0)
{
	uv_loop_init(&m_Loop);
	m_Loop.data = this;
}

Timer::~Timer()
{
	uv_loop_close(&m_Loop);
	m_Timers.clear();
}

void Timer::update()
{
	uv_run(&m_Loop, UV_RUN_NOWAIT);
}

uint32_t Timer::start(uint64_t timeout, uint64_t repeat, task_t task)
{
	auto timerID = ++m_TimerID;
	auto& timer = m_Timers[timerID];
	uv_timer_init(&m_Loop, &timer.Handle);
	timer.ID = timerID;
	timer.Method = task;
	timer.Handle.data = &timer;
	auto reuslt = uv_timer_start(&timer.Handle, [](uv_timer_t* handle) {
		auto _this = (Timer*)handle->loop->data;
		auto timer = (timer_t*)handle->data;
		if (timer->Method) timer->Method();
		if (handle->repeat == 0)
		{
			uv_timer_stop(handle);
			_this->m_Timers.erase(timer->ID);
		}
		}, timeout, repeat);
	return timerID;
}

bool Timer::stop(uint32_t handle)
{
	auto result = m_Timers.find(handle);
	if (result == m_Timers.end()) return false;
	auto& timer = result->second;
	uv_timer_stop(&timer.Handle);
	return m_Timers.erase(timer.ID);
}