#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by chivenzhang@gmail.com.
*
* =================================================*/
#include "MS.h"
#if 20 <= OPENMS_CPP_VERSION
#include <coroutine>

// =================== Async ===================

// Async<T>

struct MSAsyncBase
{
	bool m_Await = false;
};

template<class T>
class MSAsync;
template<class T>
struct MSAsyncHandle : MSAsyncBase
{
	MSAsync<T> get_return_object();
	std::suspend_always initial_suspend() { m_Value = T(); return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	std::suspend_always yield_value(T value) { m_Value = std::move(value); return {}; }
	void return_value(T value) { m_Value = std::move(value); }
	void unhandled_exception() {}
	T m_Value;
	std::coroutine_handle<> m_Handle;
};
template<class T>
class MSAsync : public std::coroutine_handle<MSAsyncHandle<T>>
{
public:
	using promise_type = MSAsyncHandle<T>;
	bool awaitable() const { return this->promise().m_Await; }
	T const& value() const { return this->promise().m_Value; }
};
template<class T>
inline MSAsync<T> MSAsyncHandle<T>::get_return_object()
{
	return { MSAsync<T>::from_promise(*this) };
}

// Async<void>

template<>
class MSAsync<void>;
template<>
struct MSAsyncHandle<void> : MSAsyncBase
{
	MSAsync<void> get_return_object();
	std::suspend_always initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	std::suspend_always yield_value(std::nullptr_t) { return {}; }
	void return_void() {}
	void unhandled_exception() {}

	std::coroutine_handle<> m_Handle;
};
template<>
class MSAsync<void> : public std::coroutine_handle<MSAsyncHandle<void>>
{
public:
	using promise_type = MSAsyncHandle<void>;
	bool awaitable() const { return this->promise().m_Await; }
};
inline MSAsync<void> MSAsyncHandle<void>::get_return_object()
{
	return { MSAsync<void>::from_promise(*this) };
}

// =================== Await ===================

// Await<T>

template<class T, class...Args>
class MSAwait
{
public:
	explicit MSAwait(MSLambda<MSAsync<T>(Args &&...)>&& method, Args... args)
		:
		m_Method(method),
		m_Args(std::forward<Args>(args)...)
	{
	}
	bool await_ready() { return false; }
	T await_resume() { return m_Handle.promise().m_Value; }
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
	{
		m_Handle = std::apply(m_Method, m_Args);
		m_Handle.promise().m_Handle = handle;
		return m_Handle;
	}
private:
	MSAsync<T> m_Handle;
	MSTuple<Args &&...> m_Args;
	MSLambda<MSAsync<T>(Args...)> m_Method;
};

// Await<void>

template<class...Args>
class MSAwait<void, Args...>
{
public:
	explicit MSAwait(MSLambda<MSAsync<void>(Args &&...)>&& method, Args... args)
		:
		m_Method(method),
		m_Args(std::make_tuple(std::forward<Args>(args)...))
	{
	}
	bool await_ready() { return false; }
	void await_resume() {}
	std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
	{
		m_Handle = std::apply(m_Method, m_Args);
		m_Handle.promise().m_Handle = handle;
		return m_Handle;
	}
private:
	MSAsync<void> m_Handle;
	MSTuple<Args &&...> m_Args;
	MSLambda<MSAsync<void>(Args...)> m_Method;
};

#endif