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
#include <future>
#include <variant>
#include <coroutine>
#include <functional>
#include "TraitsUtility.h"

// ==================== Await<T> ====================

template <class T>
using TAwait = std::function<void(T)>;

// =================== Promise<T> ===================

enum class TAsyncState
{
	NONE = 0, PEND, AWAIT, YIELD, DONE,
};

struct TPromiseBase
{
	struct InitAwaitable
	{
		bool await_ready() const noexcept { return false; }

		template <class T>
		void await_suspend(std::coroutine_handle<T> handle) noexcept
		{
		}

		void await_resume() noexcept
		{
			(*m_ThisState) = TAsyncState::PEND;
		}

		TAsyncState* m_ThisState;
	};

	struct FinalAwaitable
	{
		bool await_ready() const noexcept { return false; }

		template <class T>
		auto await_suspend(std::coroutine_handle<T> handle) noexcept
		{
			if (handle.promise().m_NextHandle) handle.promise().m_NextHandle.resume();
		}

		void await_resume() noexcept
		{
		}
	};

	auto initial_suspend() noexcept
	{
		return InitAwaitable{&m_ThisState};
	}

	auto final_suspend() noexcept
	{
		return FinalAwaitable{};
	}

	std::coroutine_handle<> m_ThisHandle;
	std::coroutine_handle<> m_NextHandle;
	TAsyncState m_ThisState = TAsyncState::NONE;
	TAsyncState m_LastState = TAsyncState::NONE;
	TAsyncState* m_RootState = &m_ThisState;
	std::coroutine_handle<>* m_RootHandle = &m_ThisHandle;
};

template <class T>
class TAsync;

template <class T>
struct TAsyncPromise : TPromiseBase
{
	TAsync<T> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_value(T value) noexcept
	{
		m_ThisState = TAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Value = std::move(value);
		m_Error = nullptr;
	}

	auto yield_value(T value) noexcept
	{
		struct TYieldAwaitable
		{
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::YIELD;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;
			}

			auto await_resume() noexcept
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);
			}
		};
		m_Value = std::move(value);
		return TYieldAwaitable{};
	}

	T& result()
	{
		if (m_Error) std::rethrow_exception(m_Error);
		return m_Value;
	}

	template <class U>
	auto await_transform(TAsync<U>&& async)
	{
		struct TAsyncAwaitable
		{
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;
			std::coroutine_handle<TAsyncPromise<U>> m_NextHandle;

			bool await_ready() const noexcept { return !m_NextHandle || m_NextHandle.done(); }

			auto await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				m_NextHandle.promise().m_NextHandle = m_ThisHandle;
				m_NextHandle.promise().m_RootState = m_ThisHandle.promise().m_RootState;
				m_NextHandle.promise().m_RootHandle = m_ThisHandle.promise().m_RootHandle;
				return m_NextHandle;
			}

			U await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				return m_NextHandle.promise().result();
			}
		};
		return TAsyncAwaitable{.m_NextHandle = async.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F lambda)
	{
		using return_type = TTraits<typename TTraits<F>::template argument_data<0>>::template argument_data<0>;

		struct TLambdaAwaitable
		{
			F m_Lambda;
			std::future<return_type> m_Future;
			std::promise<return_type> m_Promise;
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				m_Future = m_Promise.get_future();
				TAwait<return_type> promise = [this](return_type const& value)
				{
					m_Promise.set_value(value);
					if (m_ThisHandle) m_ThisHandle.resume();
				};
				m_Lambda(promise);
			}

			return_type await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				return m_Future.get();
			}
		};
		return TLambdaAwaitable{lambda};
	}

	T m_Value;
	std::exception_ptr m_Error;
};

template <>
struct TAsyncPromise<void> : TPromiseBase
{
	TAsync<void> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_void() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = nullptr;
	}

	auto yield_value(std::nullptr_t) noexcept
	{
		struct TYieldAwaitable
		{
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::YIELD;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
			}

			auto await_resume() noexcept
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);
			}
		};
		return TYieldAwaitable{};
	}

	void result() const
	{
		if (m_Error) std::rethrow_exception(m_Error);
	}

	template <class U>
	auto await_transform(TAsync<U>&& async)
	{
		struct TAsyncAwaitable
		{
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;
			std::coroutine_handle<TAsyncPromise<U>> m_NextHandle;

			bool await_ready() const noexcept { return !m_NextHandle || m_NextHandle.done(); }

			auto await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				m_NextHandle.promise().m_NextHandle = m_ThisHandle;
				m_NextHandle.promise().m_RootState = m_ThisHandle.promise().m_RootState;
				m_NextHandle.promise().m_RootHandle = m_ThisHandle.promise().m_RootHandle;
				return m_NextHandle;
			}

			U await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				return m_NextHandle.promise().result();
			}
		};
		return TAsyncAwaitable{.m_NextHandle = async.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F lambda)
	{
		using return_type = TTraits<typename TTraits<F>::template argument_data<0>>::template argument_data<0>;

		struct TLambdaAwaitable
		{
			F m_Lambda;
			std::future<return_type> m_Future;
			std::promise<return_type> m_Promise;
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;

				m_Future = m_Promise.get_future();
				TAwait<return_type> promise = [this](return_type const& value)
				{
					m_Promise.set_value(value);
					if (m_ThisHandle) m_ThisHandle.resume();
				};
				m_Lambda(promise);
			}

			return_type await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				return m_Future.get();
			}
		};
		return TLambdaAwaitable{lambda};
	}

	std::exception_ptr m_Error;
};

// ==================== Async<T> ====================

template <class T>
class TAsync
{
public:
	using promise_type = TAsyncPromise<T>;
	template <class U>
	using await_type = std::function<void(U)>;

	TAsync() : m_ThisHandle(nullptr)
	{
	}

	explicit TAsync(std::coroutine_handle<promise_type> handle) : m_ThisHandle(handle)
	{
	}

	TAsync(TAsync&& other) noexcept : m_ThisHandle(other.m_ThisHandle) { other.m_ThisHandle = nullptr; }

	~TAsync() { if (m_ThisHandle) m_ThisHandle.destroy(); }

	TAsync(TAsync const&) = delete;

	TAsync& operator=(const TAsync&) = delete;

	TAsync& operator=(TAsync&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			if (m_ThisHandle) m_ThisHandle.destroy();
			m_ThisHandle = other.m_ThisHandle;
			other.m_ThisHandle = nullptr;
		}
		return *this;
	}

	explicit operator bool() const noexcept
	{
		return bool(m_ThisHandle);
	}

	bool done() const
	{
		return m_ThisHandle.done();
	}

	void resume()
	{
		if (bool(*m_ThisHandle.promise().m_RootHandle) && m_ThisHandle.promise().m_RootHandle->done() == false) m_ThisHandle.promise().m_RootHandle->resume();
		else m_ThisHandle.resume();
	}

	void destroy()
	{
		m_ThisHandle.destroy();
	}

	TAsyncState state() const
	{
		return *m_ThisHandle.promise().m_RootState;
	}

	T value()
	{
		return m_ThisHandle.promise().result();
	}

private:
	template <class U>
	friend struct TAsyncPromise;
	std::coroutine_handle<promise_type> m_ThisHandle;
};

template <class T>
TAsync<T> TAsyncPromise<T>::get_return_object() noexcept
{
	return TAsync<T>(std::coroutine_handle<TAsyncPromise<T>>::from_promise(*this));
}

inline TAsync<void> TAsyncPromise<void>::get_return_object() noexcept
{
	return TAsync(std::coroutine_handle<TAsyncPromise>::from_promise(*this));
}