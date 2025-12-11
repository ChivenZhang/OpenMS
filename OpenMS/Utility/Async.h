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
#include <variant>
#include <coroutine>

// ==================== Await<T> ====================

template <class T>
using MSAwait = MSLambda<void(T)>;

// =================== Promise<T> ===================

enum class MSAsyncState
{
	NONE = 0, PEND, AWAIT, YIELD, DONE,
};

struct MSPromiseBase
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
			(*m_ThisState) = MSAsyncState::PEND;
		}

		MSAsyncState* m_ThisState;
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
	MSAsyncState m_ThisState = MSAsyncState::NONE;
	MSAsyncState m_LastState = MSAsyncState::NONE;
	MSAsyncState* m_RootState = &m_ThisState;
	std::coroutine_handle<>* m_RootHandle = &m_ThisHandle;
};

template <class T>
class MSAsync;

template <class T>
struct MSAsyncPromise : MSPromiseBase
{
	MSAsync<T> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = MSAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_value(T value) noexcept
	{
		m_ThisState = MSAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Value = std::move(value);
	}

	auto yield_value(T value) noexcept
	{
		struct MSYieldAwaitable
		{
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::YIELD;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;
			}

			auto await_resume() noexcept
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;
				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
			}
		};
		m_Value = std::move(value);
		return MSYieldAwaitable{};
	}

	T& result()
	{
		if (m_Error) MSThrow(MSError("result()" " at " __FILE__));
		return m_Value;
	}

	template <class U>
	auto await_transform(MSAsync<U>&& async)
	{
		struct MSAsyncAwaitable
		{
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;
			std::coroutine_handle<MSAsyncPromise<U>> m_NextHandle;

			bool await_ready() const noexcept { return !m_NextHandle || m_NextHandle.done(); }

			auto await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::AWAIT;
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

				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
				return m_NextHandle.promise().result();
			}
		};
		return MSAsyncAwaitable{.m_NextHandle = async.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F lambda)
	{
		using return_type = MSTraits<typename MSTraits<F>::template argument_data<0>>::template argument_data<0>;

		struct MSLambdaAwaitable
		{
			F m_Lambda;
			std::future<return_type> m_Future;
			std::promise<return_type> m_Promise;
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				m_Future = m_Promise.get_future();
				MSAwait<return_type> promise = [this](return_type const& value)
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

				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
				return m_Future.get();
			}
		};
		return MSLambdaAwaitable{lambda};
	}

	T m_Value;
	std::exception_ptr m_Error;
};

template <>
struct MSAsyncPromise<void> : MSPromiseBase
{
	MSAsync<void> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = MSAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_void() noexcept
	{
		m_ThisState = MSAsyncState::DONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
	}

	auto yield_value(std::nullptr_t) noexcept
	{
		struct MSYieldAwaitable
		{
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::YIELD;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
			}

			auto await_resume() noexcept
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
			}
		};
		return MSYieldAwaitable{};
	}

	void result() const
	{
		if (m_Error) MSThrow(MSError("result()" " at " __FILE__));
	}

	template <class U>
	auto await_transform(MSAsync<U>&& async)
	{
		struct MSAsyncAwaitable
		{
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;
			std::coroutine_handle<MSAsyncPromise<U>> m_NextHandle;

			bool await_ready() const noexcept { return !m_NextHandle || m_NextHandle.done(); }

			auto await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::AWAIT;
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

				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
				return m_NextHandle.promise().result();
			}
		};
		return MSAsyncAwaitable{.m_NextHandle = async.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F lambda)
	{
		using return_type = MSTraits<typename MSTraits<F>::template argument_data<0>>::template argument_data<0>;

		struct MSLambdaAwaitable
		{
			F m_Lambda;
			std::future<return_type> m_Future;
			std::promise<return_type> m_Promise;
			std::coroutine_handle<MSAsyncPromise> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
			{
				m_ThisHandle = handle;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = MSAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;

				m_Future = m_Promise.get_future();
				MSAwait<return_type> promise = [this](return_type const& value)
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

				if (m_ThisHandle.promise().m_Error) MSThrow(MSError("await_resume()" " at " __FILE__));
				return m_Future.get();
			}
		};
		return MSLambdaAwaitable{lambda};
	}

	std::exception_ptr m_Error;
};

// ==================== Async<T> ====================

template <class T>
class MSAsync
{
public:
	using promise_type = MSAsyncPromise<T>;
	template <class U>
	using await_type = MSLambda<void(U)>;

	MSAsync() : m_ThisHandle(nullptr)
	{
	}

	explicit MSAsync(std::coroutine_handle<promise_type> handle) : m_ThisHandle(handle)
	{
	}

	MSAsync(MSAsync&& other) noexcept : m_ThisHandle(other.m_ThisHandle) { other.m_ThisHandle = nullptr; }

	~MSAsync() { if (m_ThisHandle) m_ThisHandle.destroy(); }

	MSAsync(MSAsync const&) = delete;

	MSAsync& operator=(const MSAsync&) = delete;

	MSAsync& operator=(MSAsync&& other) noexcept
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

	MSAsyncState state() const
	{
		return *m_ThisHandle.promise().m_RootState;
	}

	T value()
	{
		return m_ThisHandle.promise().result();
	}

private:
	template <class U>
	friend class MSAsyncPromise;
	std::coroutine_handle<promise_type> m_ThisHandle;
};

template <class T>
MSAsync<T> MSAsyncPromise<T>::get_return_object() noexcept
{
	return MSAsync<T>(std::coroutine_handle<MSAsyncPromise<T>>::from_promise(*this));
}

inline MSAsync<void> MSAsyncPromise<void>::get_return_object() noexcept
{
	return MSAsync(std::coroutine_handle<MSAsyncPromise>::from_promise(*this));
}

#endif
