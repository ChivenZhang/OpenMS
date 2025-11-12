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

// Await<T>

template<class T>
using MSAwait = std::function<void(T)>;

// Async<T>

enum class MSAsyncState
{
    NONE = 0, PEND, AWAIT, DONE,
};

struct MSPromiseBase
{
    struct FinalAwaitable
    {
        bool await_ready() const noexcept { return false; }
        template<class T>
        void await_suspend(std::coroutine_handle<T> handle) noexcept
        {
            if (handle.promise().m_NextHandle)
            {
                handle.promise().m_NextHandle.resume();
            }
        }
        void await_resume() noexcept {}
    };

    auto initial_suspend() noexcept
    {
        m_State = MSAsyncState::PEND;
        return std::suspend_never{};
    }

    auto final_suspend() noexcept
    {
        return FinalAwaitable{};
    }

    MSAsyncState m_State = MSAsyncState::NONE;
    std::coroutine_handle<> m_NextHandle;
};

template<class T>
class MSAsync;

template<class T>
struct MSAsyncPromise : MSPromiseBase
{
    MSAsync<T> get_return_object() noexcept { return MSAsync<T>(std::coroutine_handle<MSAsyncPromise>::from_promise(*this)); }

    void unhandled_exception() noexcept
    {
        m_State = MSAsyncState::DONE;
        m_Error = std::current_exception();
    }

    void return_value(T value) noexcept
    {
        m_State = MSAsyncState::DONE;
        m_Value = std::move(value);
    }

    auto yield_value(T value) noexcept
    {
        m_State = MSAsyncState::DONE;
        m_Value = std::move(value);
        return std::suspend_always{};
    }

    T& result()
    {
        if (m_Error) std::rethrow_exception(m_Error);
        return m_Value;
    }

    template<class U>
    auto await_transform(MSAsync<U> awaitable)
    {
        return awaitable;
    }

    template<class F>
    auto await_transform(F func)
    {
        using return_type = MSTraits<F>::return_data;

        struct MSTransformAwaitable
        {
            decltype(func) m_Callback;
            MSAsyncState m_LastState = MSAsyncState::NONE;
            std::coroutine_handle<MSAsyncPromise> m_NextHandle;
            std::future<return_type> m_Future;
            std::promise<return_type> m_Promise;

            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
            {
                m_NextHandle = handle;
                m_LastState = m_NextHandle.promise().m_State;
                m_NextHandle.promise().m_State = MSAsyncState::AWAIT;
                m_Future = m_Promise.get_future();

                if constexpr (std::is_void_v<return_type>)
                {
                    MSAwait<return_type> promise_func = [&](return_type && value)
                    {
                        m_Promise.set_value(value);
                        m_NextHandle.resume();
                    };
                    m_Callback(promise_func);
                }
                else
                {
                    MSAwait<return_type> promise_func = [&](return_type && value)
                    {
                        m_Promise.set_value(value);
                        m_NextHandle.resume();
                    };
                    m_Callback(promise_func);
                }
            }

            return_type await_resume()
            {
                if (m_NextHandle.promise().m_Error) std::rethrow_exception(m_NextHandle.promise().m_Error);
                m_NextHandle.promise().m_State = m_LastState;
                return m_Future.get();
            }
        };
        return MSTransformAwaitable{ func };
    }

    std::exception_ptr m_Error;
    T m_Value;
};

template<>
struct MSAsyncPromise<void> : MSPromiseBase
{
    MSAsync<void> get_return_object() noexcept;

    void unhandled_exception() noexcept
    {
        m_State = MSAsyncState::DONE;
        m_Error = std::current_exception();
    }

    void return_void() noexcept
    {
        m_State = MSAsyncState::DONE;
    }

    auto yield_value(std::nullptr_t) noexcept
    {
        m_State = MSAsyncState::DONE;
        return std::suspend_always{};
    }

    void result() const
    {
        if (m_Error) std::rethrow_exception(m_Error);
    }

    std::exception_ptr m_Error;
};

template <class T>
class MSAsync
{
public:
    using promise_type = MSAsyncPromise<T>;
    template<class U>
    using await_type = std::function<void(U)>;

    MSAsync() : m_ThisHandle(nullptr)
    {
    }

    explicit MSAsync(std::coroutine_handle<promise_type> handle) : m_ThisHandle(handle)
    {
    }

    MSAsync(MSAsync const&) = delete;
    MSAsync& operator=(const MSAsync&) = delete;

    MSAsync(MSAsync&& other) noexcept : m_ThisHandle(other.m_ThisHandle)
    {
        other.m_ThisHandle = nullptr;
    }

    ~MSAsync()
    {
        if (m_ThisHandle) m_ThisHandle.destroy();
    }

    MSAsync& operator=(MSAsync&& other) noexcept
    {
        if (std::addressof(other) != this)
        {
            if (m_ThisHandle) m_ThisHandle.destroy();
            m_ThisHandle = other.m_ThisHandle;
            other.m_ThisHandle = nullptr;
        }
        return *this;
    }

    struct MSAwaitBase
    {
        bool await_ready() const noexcept { return !m_NextHandle || m_NextHandle.done(); }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept
        {
            m_LastState = m_NextHandle.promise().m_State;
            m_NextHandle.promise().m_State = MSAsyncState::AWAIT;
            m_NextHandle.promise().m_NextHandle = handle;
            return m_NextHandle;
        }

        MSAsyncState m_LastState;
        std::coroutine_handle<promise_type> m_NextHandle;
    };

    auto operator co_await() const & noexcept
    {
        struct MSAsyncAwaitable : MSAwaitBase
        {
            decltype(auto) await_resume()
            {
                if (!this->m_NextHandle) throw std::runtime_error("invalid coroutine");
                this->m_NextHandle.promise().m_State = this->m_LastState;
                return this->m_NextHandle.promise().result();
            }
        };
        return MSAsyncAwaitable { {MSAsyncState::NONE, m_ThisHandle} };
    }

    auto operator co_await() const && noexcept
    {
        struct MSAsyncAwaitable : MSAwaitBase
        {
            decltype(auto) await_resume()
            {
                if (!this->m_NextHandle) throw std::runtime_error("invalid coroutine");
                this->m_NextHandle.promise().m_State = this->m_LastState;
                return this->m_NextHandle.promise().result();
            }
        };
        return MSAsyncAwaitable { {MSAsyncState::NONE, m_ThisHandle} };
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
        m_ThisHandle.resume();
    }

    void destroy()
    {
        m_ThisHandle.destroy();
    }

    MSAsyncState state() const
    {
        return m_ThisHandle.promise().m_State;
    }

    T value()
    {
        return m_ThisHandle.promise().result();
    }

private:
    std::coroutine_handle<promise_type> m_ThisHandle;
};

inline MSAsync<void> MSAsyncPromise<void>::get_return_object() noexcept
{
    return MSAsync(std::coroutine_handle<MSAsyncPromise>::from_promise(*this));
}

#endif