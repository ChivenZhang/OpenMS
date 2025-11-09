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

// Await<T>

template<class T>
class MSAwait;
template<class T>
struct MSAwaitHandle
{
	MSAwaitHandle(std::coroutine_handle<> handle, T& value) : m_Handle(handle), m_Value(value) {}
	MSAwaitHandle(MSAwaitHandle const& value) = default;
	MSAwaitHandle(MSAwaitHandle&& value) noexcept = default;
	void setValue(T&& value)
	{
		m_Value = std::move(value);
		m_Handle.resume();
	}
private:
	std::coroutine_handle<> m_Handle;
	T& m_Value;
};
template<class T>
class MSAwait
{
public:
	using handle_t = MSAwaitHandle<T>;

public:
	explicit MSAwait(MSLambda<void(handle_t)> lambda, T const& value = T()) : m_Lambda(lambda), m_Value(value) {}
	bool await_ready() { return false; }
	T await_resume() { return m_Value; }
	void await_suspend(std::coroutine_handle<> handle)
	{
		if (m_Lambda) m_Lambda({ handle, m_Value });
		else handle.resume();
	}
private:
	MSLambda<void(handle_t)> m_Lambda;
	T m_Value;
};

// Await<void>

template<>
class MSAwait<void>;
template<>
struct MSAwaitHandle<void>
{
	MSAwaitHandle(std::coroutine_handle<> handle) : m_Handle(handle) {}
	MSAwaitHandle(MSAwaitHandle const& value) = default;
	MSAwaitHandle(MSAwaitHandle&& value) noexcept = default;
	void setValue() { m_Handle.resume(); }
private:
	std::coroutine_handle<> m_Handle;
};
template<>
class MSAwait<void>
{
public:
	using handle_t = MSAwaitHandle<void>;

public:
	explicit MSAwait(MSLambda<void(handle_t)> lambda) : m_Lambda(lambda) {}
	bool await_ready() { return false; }
	void await_resume() {}
	void await_suspend(std::coroutine_handle<> handle)
	{
		if (m_Lambda) m_Lambda({ handle });
		else handle.resume();
	}
private:
	MSLambda<void(handle_t)> m_Lambda;
};

// =================== Async ===================

// Async<T>

template<class T>
class MSAsync;
template<class T>
struct MSAsyncHandle
{
	MSAsync<T> get_return_object();
	std::suspend_always initial_suspend() { m_Value = T(); return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	std::suspend_always yield_value(T value) { m_Value = std::move(value); return {}; }
	void return_value(T value) { m_Value = std::move(value); }
	void unhandled_exception() {}
	T m_Value;
};
template<class T>
class MSAsync : public std::coroutine_handle<MSAsyncHandle<T>>
{
public:
	using promise_type = MSAsyncHandle<T>;
	T const& getValue() const { return this->promise().m_Value; }
};
template<class T>
inline MSAsync<T> MSAsyncHandle<T>::get_return_object() { return { MSAsync<T>::from_promise(*this) }; }

// Async<void>

template<>
class MSAsync<void>;
template<>
struct MSAsyncHandle<void>
{
	MSAsync<void> get_return_object();
	std::suspend_always initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	std::suspend_always yield_value(std::nullptr_t) { return {}; }
	void return_void() {}
	void unhandled_exception() {}
};
template<>
class MSAsync<void> : public std::coroutine_handle<MSAsyncHandle<void>>
{
public:
	using promise_type = MSAsyncHandle<void>;
};
inline MSAsync<void> MSAsyncHandle<void>::get_return_object() { return { MSAsync<void>::from_promise(*this) }; }

#endif