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
#include "Timer.h"

Timer::Timer()
	:
	m_TimerID(0),
	m_Loop(nullptr),
	m_AsyncExit(nullptr),
	m_AsyncStop(nullptr),
	m_AsyncStart(nullptr)
{
	MSPromise<void> promise;
	auto future = promise.get_future();

	m_Thread = MSThread([=, &promise]() {
		uv_loop_t loop;
		uv_async_t asyncExit, asyncStart, asyncStop;
		uv_loop_init(&loop);
		loop.data = this;

		uv_async_init(&loop, &asyncExit, [](uv_async_t* handle) {
			auto _this = (Timer*)handle->loop->data;

			uv_close((uv_handle_t*)_this->m_AsyncStart, nullptr);
			uv_close((uv_handle_t*)_this->m_AsyncStop, nullptr);
			uv_close((uv_handle_t*)_this->m_AsyncExit, nullptr);
			uv_loop_close(handle->loop);
			});

		uv_async_init(&loop, &asyncStart, [](uv_async_t* handle) {
			auto _this = (Timer*)handle->loop->data;
			auto clause = (clause_t*)handle->data;
			auto timer = &clause->Timer;

			uv_timer_init(handle->loop, &timer->Handle);
			timer->Handle.data = timer;
			uv_timer_start(&timer->Handle, [](uv_timer_t* handle) {
				auto _this = (Timer*)handle->loop->data;
				auto timer = (timer_t*)handle->data;
				if (timer->Method) timer->Method(timer->ID);
				if (handle->repeat == 0)
				{
					uv_timer_stop(handle);
					uv_close((uv_handle_t*)&timer->Handle, [](uv_handle_t* handle) {
						auto _this = (Timer*)handle->loop->data;
						auto timer = (timer_t*)handle->data;
						_this->m_Timers.erase(timer->ID);
						});
				}
				}, timer->Timeout, timer->Repeat);

			clause->Promise.set_value();
			});

		uv_async_init(&loop, &asyncStop, [](uv_async_t* handle) {
			auto clause = (clause_t*)handle->data;
			auto timer = &clause->Timer;

			uv_timer_stop(&timer->Handle);
			clause->Promise.set_value();
			});

		m_Loop = &loop;
		m_AsyncExit = &asyncExit;
		m_AsyncStop = &asyncStop;
		m_AsyncStart = &asyncStart;
		promise.set_value();
		uv_run(&loop, UV_RUN_DEFAULT);
		m_AsyncExit = nullptr;
		m_AsyncStop = nullptr;
		m_AsyncStart = nullptr;
		m_Loop = nullptr;
		});

	future.wait();
}

Timer::~Timer()
{
	MSVector<uint32_t> handles;
	for (auto& timer : m_Timers) handles.push_back(timer.first);
	for (auto& timer : handles) stop(timer);
	uv_async_send(m_AsyncExit);
	if (m_Thread.joinable()) m_Thread.join();
	m_Timers.clear();
}

uint32_t Timer::start(uint64_t timeout, uint64_t repeat, task_t task)
{
	auto timerID = ++m_TimerID;
	auto& timer = m_Timers[timerID];
	timer.ID = timerID;
	timer.Method = task;
	timer.Repeat = repeat;
	timer.Timeout = timeout;
	timer.Handle.data = &timer;

	MSPromise<void> promise;
	auto future = promise.get_future();
	struct clause_t
	{
		timer_t& Timer;
		MSPromise<void>& Promise;
	} clause{ timer, promise };

	m_AsyncStart->data = &clause;
	if (uv_async_send(m_AsyncStart) == 0) future.wait();
	return timerID;
}

bool Timer::stop(uint32_t handle)
{
	auto result = m_Timers.find(handle);
	if (result == m_Timers.end()) return false;
	auto& timer = result->second;

	MSPromise<void> promise;
	auto future = promise.get_future();
	clause_t clause{ timer, promise };

	m_AsyncStop->data = &clause;
	if (uv_async_send(m_AsyncStop) == 0) future.wait();
	future.wait();
	m_Timers.erase(result);
	return true;
}