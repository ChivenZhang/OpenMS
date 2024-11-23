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
	TPromise<void> promise;
	auto future = promise.get_future();

	m_Thread = TThread([=, &promise]() {
		uv_loop_init(&m_Loop);
		m_Loop.data = this;

		uv_async_init(&m_Loop, &m_AsyncStart, [](uv_async_t* handle) {
			auto _this = (Timer*)handle->loop->data;
			auto clause = (clause_t*)handle->data;
			auto timer = &clause->Timer;

			uv_timer_init(handle->loop, &timer->Handle);
			auto result = uv_timer_start(&timer->Handle, [](uv_timer_t* handle) {
				auto _this = (Timer*)handle->loop->data;
				auto timer = (timer_t*)handle->data;
				if (timer->Method) timer->Method();
				if (handle->repeat == 0)
				{
					uv_timer_stop(handle);
					uv_close((uv_handle_t*)&timer->Handle, nullptr);
					_this->m_Timers.erase(timer->ID);
				}
				}, timer->Timeout, timer->Repeat);

			clause->Promise.set_value();
			});

		uv_async_init(&m_Loop, &m_AsyncStop, [](uv_async_t* handle) {
			auto clause = (clause_t*)handle->data;
			auto timer = &clause->Timer;

			uv_timer_stop(&timer->Handle);
			clause->Promise.set_value();
			});

		uv_async_init(&m_Loop, &m_AsyncExit, [](uv_async_t* handle) {
			auto _this = (Timer*)handle->loop->data;

			uv_close((uv_handle_t*)&_this->m_AsyncStart, nullptr);
			uv_close((uv_handle_t*)&_this->m_AsyncStop, nullptr);
			uv_close((uv_handle_t*)&_this->m_AsyncExit, nullptr);
			uv_loop_close(handle->loop);
			});

		promise.set_value();
		uv_run(&m_Loop, UV_RUN_DEFAULT);
		});

	future.wait();
}

Timer::~Timer()
{
	TVector<uint32_t> handles;
	for (auto& timer : m_Timers) handles.push_back(timer.first);
	for (auto& timer : handles) stop(timer);
	uv_async_send(&m_AsyncExit);
	if (m_Thread.joinable()) m_Thread.join();
	m_Timers.clear();
}

uint32_t Timer::start(uint64_t timeout, uint64_t repeat, task_t task)
{
	auto timerID = ++m_TimerID;
	auto& timer = m_Timers[timerID];
	timer.ID = timerID;
	timer.Repeat = repeat;
	timer.Timeout = timeout;
	timer.Method = task;
	timer.Handle.data = &timer;

	TPromise<void> promise;
	auto future = promise.get_future();
	struct clause_t
	{
		timer_t& Timer;
		TPromise<void>& Promise;
	} clause{ timer, promise };

	m_AsyncStart.data = &clause;
	if (uv_async_send(&m_AsyncStart) == 0)
	{
		future.wait();
		return timerID;
	}
	return uint32_t();
}

bool Timer::stop(uint32_t handle)
{
	auto result = m_Timers.find(handle);
	if (result == m_Timers.end()) return false;
	auto& timer = result->second;

	TPromise<void> promise;
	auto future = promise.get_future();
	clause_t clause{ timer, promise };

	m_AsyncStop.data = &clause;
	if (uv_async_send(&m_AsyncStop) == 0)
	{
		future.wait();
		m_Timers.erase(timer.ID);
		return true;
	}
	return false;
}