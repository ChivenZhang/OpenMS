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

// =================== Await ===================

template<class T>
class MSAwait;
template<class T>
struct MSAwaitHandle
{
	MSAwaitHandle(std::coroutine_handle<> handle, T& value) : m_Async(handle), m_Value(value) {}
	MSAwaitHandle(MSAwaitHandle const& value) = default;
	MSAwaitHandle(MSAwaitHandle&& value) noexcept = default;
	void setValue(T&& value)
	{
		m_Value = std::move(value);
		m_Async.resume();
	}
private:
	std::coroutine_handle<> m_Async;
	T& m_Value;
};
template<class T>
class MSAwait
{
public:
	explicit MSAwait(MSLambda<void(MSAwaitHandle<T>)> lambda, T const& value = T()) : m_Lambda(lambda), m_Value(value) {}
	bool await_ready() { return false; }
	T await_resume() { return m_Value; }
	void await_suspend(std::coroutine_handle<> handle)
	{
		if (m_Lambda) m_Lambda({ handle, m_Value });
		else handle.resume();
	}
private:
	MSLambda<void(MSAwaitHandle<T>)> m_Lambda;
	T m_Value;
};

template<>
class MSAwait<void>;
template<>
struct MSAwaitHandle<void>
{
	MSAwaitHandle(std::coroutine_handle<> handle) : m_Async(handle) {}
	MSAwaitHandle(MSAwaitHandle const& value) = default;
	MSAwaitHandle(MSAwaitHandle&& value) noexcept = default;
	void setValue() { m_Async.resume(); }
private:
	std::coroutine_handle<> m_Async;
};
template<>
class MSAwait<void>
{
public:
	explicit MSAwait(MSLambda<void(MSAwaitHandle<void>)> lambda) : m_Lambda(lambda) {}
	bool await_ready() { return false; }
	void await_resume() {}
	void await_suspend(std::coroutine_handle<> handle)
	{
		if (m_Lambda) m_Lambda({ handle });
		else handle.resume();
	}
private:
	MSLambda<void(MSAwaitHandle<void>)> m_Lambda;
};

// =================== Async ===================

template<class T>
class MSAsync;
template<class T>
struct MSAsyncHandle;
template<class T>
class MSAsync : public std::coroutine_handle<MSAsyncHandle<T>>
{
public:
	using promise_type = MSAsyncHandle<T>;
	T const& getValue() const { return this->promise().m_Value; }
};
template<class T>
struct MSAsyncHandle
{
	MSAsync<T> get_return_object() { return { MSAsync<T>::from_promise(*this) }; }
	void return_value(T value) { m_Value = value; }
	std::suspend_always yield_value(T value) { m_Value = value; return {}; }
	std::suspend_never initial_suspend() { m_Value = T(); return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	void unhandled_exception() {}
	T m_Value;
};

template<>
class MSAsync<void>;
template<>
struct MSAsyncHandle<void>;
template<>
class MSAsync<void> : public std::coroutine_handle<MSAsyncHandle<void>>
{
public:
	using promise_type = MSAsyncHandle<void>;
};
template<>
struct MSAsyncHandle<void>
{
	MSAsync<void> get_return_object() { return { MSAsync<void>::from_promise(*this) }; }
	void return_void() {}
	std::suspend_always yield_value() { return {}; }
	std::suspend_never initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	void unhandled_exception() {}
};

#endif