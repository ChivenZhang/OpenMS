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
	NONE = 0, PEND, AWAIT, RESUME, YIELD, DONE,
};

template <class T>
class TAsync;

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
		std::coroutine_handle<> await_suspend(std::coroutine_handle<T> handle) noexcept
		{
			if (handle.promise().m_NextHandle) return handle.promise().m_NextHandle;
			return std::noop_coroutine();
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

	TPromiseBase()
	{
		static uint32_t s_ID = 0;
		m_ID = ++s_ID;
	}
	uint32_t m_ID = 0;
	std::coroutine_handle<> m_ThisHandle;
	std::coroutine_handle<> m_NextHandle;
	TAsyncState m_ThisState = TAsyncState::NONE;
	TAsyncState m_LastState = TAsyncState::NONE;
	TAsyncState* m_RootState = &m_ThisState;
	std::coroutine_handle<>* m_RootHandle = &m_ThisHandle;
};

template <class T>
struct TAsyncPromise;

template <class T>
struct TAsyncPromise : public TPromiseBase
{
	TAsync<T> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		m_LastState = TAsyncState::NONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_value(T value) noexcept
	{
		m_ThisState = TAsyncState::DONE;
		m_LastState = TAsyncState::NONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Value = std::move(value);
		m_Error = nullptr;
	}

	T& result()
	{
		if (m_Error) std::rethrow_exception(m_Error);
		return m_Value;
	}

	template <class U>
	auto await_transform(TAsync<U>&& sub)
	{
		struct TAsyncAwaitable
		{
			std::coroutine_handle<TAsyncPromise<U>> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			auto await_suspend(std::coroutine_handle<TAsyncPromise> caller)
			{
				m_ThisHandle.promise().m_NextHandle = caller;
				m_ThisHandle.promise().m_RootState = caller.promise().m_RootState;
				m_ThisHandle.promise().m_RootHandle = caller.promise().m_RootHandle;

				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;
				return m_ThisHandle;
			}

			U await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				m_ThisHandle.promise().m_LastState = TAsyncState::NONE;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				if constexpr (std::is_void_v<U>) m_ThisHandle.promise().result();
				else return m_ThisHandle.promise().result();
			}
		};
		return TAsyncAwaitable{.m_ThisHandle = sub.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F&& lambda)
	{
		using return_type = TFirstType<typename TTraits<typename TTraits<F>::template argument_data<0>>::argument_datas>::first_type;

		struct TLambdaAwaitable
		{
			F m_Lambda;
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;
			std::any m_AwaitValue;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> caller)
			{
				m_ThisHandle = caller;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				if constexpr (std::is_void_v<return_type>)
				{
					m_Lambda([this]()
					{
						m_ThisHandle.promise().m_ThisState = TAsyncState::RESUME;
						*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
					});
				}
				else
				{
					m_Lambda([this](return_type value)
					{
						m_AwaitValue = std::move(value);

						m_ThisHandle.promise().m_ThisState = TAsyncState::RESUME;
						*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
					});
				}
			}

			return_type await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				m_ThisHandle.promise().m_LastState = TAsyncState::NONE;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				m_ThisHandle = nullptr;
				if constexpr (std::is_void_v<return_type>) return;
				else return std::any_cast<return_type>(m_AwaitValue);
			}
		};
		return TLambdaAwaitable{lambda};
	}

	T m_Value;
	std::exception_ptr m_Error;
};

template <>
struct TAsyncPromise<void> : public TPromiseBase
{
	TAsync<void> get_return_object() noexcept;

	void unhandled_exception() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		m_LastState = TAsyncState::NONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = std::current_exception();
	}

	void return_void() noexcept
	{
		m_ThisState = TAsyncState::DONE;
		m_LastState = TAsyncState::NONE;
		*m_RootState = m_ThisState;
		*m_RootHandle = nullptr;
		m_Error = nullptr;
	}

	void result() const
	{
		if (m_Error) std::rethrow_exception(m_Error);
	}

	template <class U>
	auto await_transform(TAsync<U>&& sub)
	{
		struct TAsyncAwaitable
		{
			std::coroutine_handle<TAsyncPromise<U>> m_ThisHandle;

			bool await_ready() const noexcept { return false; }

			auto await_suspend(std::coroutine_handle<TAsyncPromise> caller)
			{
				m_ThisHandle.promise().m_NextHandle = caller;
				m_ThisHandle.promise().m_RootState = caller.promise().m_RootState;
				m_ThisHandle.promise().m_RootHandle = caller.promise().m_RootHandle;

				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;
				return m_ThisHandle;
			}

			U await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				m_ThisHandle.promise().m_LastState = TAsyncState::NONE;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				if constexpr (std::is_void_v<U>) m_ThisHandle.promise().result();
				else return m_ThisHandle.promise().result();
			}
		};
		return TAsyncAwaitable{.m_ThisHandle = sub.m_ThisHandle};
	}

	template <class F>
	auto await_transform(F&& lambda)
	{
		using return_type = TFirstType<typename TTraits<typename TTraits<F>::template argument_data<0>>::argument_datas>::first_type;

		struct TLambdaAwaitable
		{
			F m_Lambda;
			std::coroutine_handle<TAsyncPromise> m_ThisHandle;
			std::any m_AwaitValue;

			bool await_ready() const noexcept { return false; }

			void await_suspend(std::coroutine_handle<TAsyncPromise> caller)
			{
				m_ThisHandle = caller;
				m_ThisHandle.promise().m_LastState = m_ThisHandle.promise().m_ThisState;
				m_ThisHandle.promise().m_ThisState = TAsyncState::AWAIT;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = m_ThisHandle;

				if constexpr (std::is_void_v<return_type>)
				{
					m_Lambda([this]()
					{
						m_ThisHandle.promise().m_ThisState = TAsyncState::RESUME;
						*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
					});
				}
				else
				{
					m_Lambda([this](return_type value)
					{
						m_AwaitValue = std::move(value);

						m_ThisHandle.promise().m_ThisState = TAsyncState::RESUME;
						*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
					});
				}
			}

			return_type await_resume()
			{
				m_ThisHandle.promise().m_ThisState = m_ThisHandle.promise().m_LastState;
				m_ThisHandle.promise().m_LastState = TAsyncState::NONE;
				*m_ThisHandle.promise().m_RootState = m_ThisHandle.promise().m_ThisState;
				*m_ThisHandle.promise().m_RootHandle = nullptr;

				if (m_ThisHandle.promise().m_Error) std::rethrow_exception(m_ThisHandle.promise().m_Error);

				m_ThisHandle = nullptr;
				if constexpr (std::is_void_v<return_type>) return;
				else return std::any_cast<return_type>(m_AwaitValue);
			}
		};
		return TLambdaAwaitable{lambda};
	}

	std::exception_ptr m_Error;
};

// =================== Async<T> ====================

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

	TAsync(std::coroutine_handle<promise_type> handle) : m_ThisHandle(handle)
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

	uint32_t id() const
	{
		return m_ThisHandle ? m_ThisHandle.promise().m_ID : 0U;
	}

	bool done() const
	{
		return m_ThisHandle.done();
	}

	void resume()
	{
		if (bool(*m_ThisHandle.promise().m_RootHandle) && !m_ThisHandle.promise().m_RootHandle->done()) m_ThisHandle.promise().m_RootHandle->resume();
		else m_ThisHandle.resume();
	}

	void destroy()
	{
		if(m_ThisHandle) m_ThisHandle.destroy();
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
	template<class U>
	friend struct TAsyncPromise;
	std::coroutine_handle<promise_type> m_ThisHandle;
};

template <class T>
TAsync<T> TAsyncPromise<T>::get_return_object() noexcept
{
	return std::coroutine_handle<TAsyncPromise<T>>::from_promise(*this);
}

inline TAsync<void> TAsyncPromise<void>::get_return_object() noexcept
{
	return std::coroutine_handle<TAsyncPromise<void>>::from_promise(*this);
}