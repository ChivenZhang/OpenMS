#pragma once
/*=================================================
* Copyright @ 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/28 02:49:42.
*
* =================================================*/
#include "../MS.h"
#include <coroutine>

template <class T>
struct IMailPromise
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		using value_type = std::remove_reference_t<T>;
		value_type value = value_type();

		IMailPromise get_return_object() { return ICoroutinePromise(handle_type::from_promise(*this)); }
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
		void return_value(T val) { value = std::move(val); }
		std::suspend_always yield_value(T val) { value = std::move(val); return {}; }
	};

	bool await_ready() const { return handle.done(); }
	void await_suspend(std::coroutine_handle<>) const { handle.resume(); }
	auto await_resume() { return handle.promise().value; }

	explicit IMailPromise(handle_type h) : handle(h) {}
	~IMailPromise() { if (handle) handle.destroy(); }
	IMailPromise(IMailPromise const& other) noexcept = delete;
	IMailPromise& operator = (IMailPromise const&) noexcept = delete;
	IMailPromise(IMailPromise&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	IMailPromise& operator = (IMailPromise&& other) noexcept
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
struct IMailPromise<void>
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		IMailPromise get_return_object() { return IMailPromise(handle_type::from_promise(*this)); }
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { std::rethrow_exception(std::current_exception()); }
		void return_void() {}
		std::suspend_always yield_value(std::nullptr_t) { return {}; }
	};

	bool await_ready() const { return handle.done(); }
	void await_suspend(std::coroutine_handle<>) const { handle.resume(); }
	void await_resume() {}

	IMailPromise() = default;
	~IMailPromise() { if (handle) handle.destroy(); }
	IMailPromise(handle_type h) : handle(h) {}
	IMailPromise(IMailPromise const& other) noexcept = delete;
	IMailPromise& operator = (IMailPromise const&) noexcept = delete;
	IMailPromise(IMailPromise&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	IMailPromise& operator = (IMailPromise&& other) noexcept
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
