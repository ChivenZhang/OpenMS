#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/28 02:49:42.
*
* =================================================*/
#include "MS.h"
#include <coroutine>

template <class T>
struct ICoroutinePromise
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		using value_type = std::remove_reference_t<T>;
		value_type value = value_type();
		std::exception_ptr error = nullptr;

		ICoroutinePromise get_return_object() { return ICoroutinePromise(handle_type::from_promise(*this)); }
		static std::suspend_always initial_suspend() { return {}; }
		static std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { error = std::current_exception(); }
		void return_value(T val) { value = std::move(val); }
		std::suspend_always yield_value(T val) { value = std::move(val); return {}; }
	};

	bool await_ready() const { return handle.done(); }
	void await_suspend(std::coroutine_handle<>) const { handle.resume(); }
	auto await_resume() { return handle.promise().value; }

	explicit ICoroutinePromise(handle_type h) : handle(h) {}
	~ICoroutinePromise() { if (handle) handle.destroy(); }
	ICoroutinePromise(ICoroutinePromise const& other) noexcept = delete;
	ICoroutinePromise& operator = (ICoroutinePromise const&) noexcept = delete;
	ICoroutinePromise(ICoroutinePromise&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	ICoroutinePromise& operator = (ICoroutinePromise&& other) noexcept
	{
		if (this != &other)
		{
			if (handle) handle.destroy();
			handle = other.handle;
			other.handle = nullptr;
		}
		return *this;
	}

	bool good() const { return handle.operator bool(); }
	bool done() const { return handle.done(); }
	void resume() const { handle.resume(); }
	explicit operator T() const { return handle.promise().value; }

private:
	handle_type handle;
};

template <>
struct ICoroutinePromise<void>
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		std::exception_ptr error = nullptr;

		ICoroutinePromise get_return_object() { return ICoroutinePromise(handle_type::from_promise(*this)); }
		static std::suspend_always initial_suspend() { return {}; }
		static std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { error = std::current_exception(); }
		static void return_void() {}
		static std::suspend_always yield_value(nullptr_t) { return {}; }
	};

	bool await_ready() const { return handle.done(); }
	void await_suspend(std::coroutine_handle<>) const { handle.resume(); }
	static void await_resume() {}

	ICoroutinePromise() = default;
	~ICoroutinePromise() { if (handle) handle.destroy(); }
	ICoroutinePromise(handle_type h) : handle(h) {}
	ICoroutinePromise(ICoroutinePromise const& other) noexcept = delete;
	ICoroutinePromise& operator = (ICoroutinePromise const&) noexcept = delete;
	ICoroutinePromise(ICoroutinePromise&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	ICoroutinePromise& operator = (ICoroutinePromise&& other) noexcept
	{
		if (this != &other)
		{
			if (handle) handle.destroy();
			handle = other.handle;
			other.handle = nullptr;
		}
		return *this;
	}

	bool good() const { return handle.operator bool(); }
	bool done() const { return handle.done(); }
	void resume() const { handle.resume(); }

private:
	handle_type handle;
};