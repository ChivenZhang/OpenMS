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

template<class T>
using MSAwait = MSLambda<void(T)>;

// =================== Promise<T> ===================

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
        m_ThisState = MSAsyncState::PEND;
        return std::suspend_never{};
    }

    auto final_suspend() noexcept
    {
        return FinalAwaitable{};
    }

    MSAsyncState m_ThisState = MSAsyncState::NONE;
    std::coroutine_handle<> m_NextHandle;
};

template<class T>
class MSAsync;

template<class T>
struct MSAsyncPromise : MSPromiseBase
{
    MSAsync<T> get_return_object() noexcept;

    void unhandled_exception() noexcept
    {
        m_ThisState = MSAsyncState::DONE;
        m_Error = std::current_exception();
    }

    void return_value(T value) noexcept
    {
        m_ThisState = MSAsyncState::DONE;
        m_Value = std::move(value);
    }

    auto yield_value(T value) noexcept
    {
        m_ThisState = MSAsyncState::DONE;
        m_Value = std::move(value);
        return std::suspend_always{};
    }

    T& result()
    {
        if (m_Error) MSThrowError(MSError("result()" " at " __FILE__));
        return m_Value;
    }

    template<class U>
    auto await_transform(MSAsync<U> async)
    {
        return async;
    }

    template<class F>
    auto await_transform(F lambda)
    {
        using return_type = MSTraits<F>::return_data;

        struct MSLambdaAwaitable
        {
            F m_Lambda;
            std::promise<return_type> m_Promise;
            std::future<return_type> m_Future;
            MSAsyncState m_LastState = MSAsyncState::NONE;
            std::coroutine_handle<MSAsyncPromise> m_NextHandle;

            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
            {
                m_NextHandle = handle;
                m_LastState = m_NextHandle.promise().m_ThisState;
                m_NextHandle.promise().m_ThisState = MSAsyncState::AWAIT;
                m_Future = m_Promise.get_future();

                MSAwait<return_type> promise = [&](return_type && value)
                {
                    try
                    {
                        m_Promise.set_value(value);
                    }
                    catch (...)
                    {
                        MSThrowError(MSError("MSLambdaAwaitable.await_suspend() at " __FILE__));
                    }
                    m_NextHandle.resume();
                };
                m_Lambda(promise);
            }

            return_type await_resume()
            {
                m_NextHandle.promise().m_ThisState = m_LastState;
                if (m_NextHandle.promise().m_Error) MSThrowError(MSError("await_resume()" " at " __FILE__));
                return m_Future.get();
            }
        };
        return MSLambdaAwaitable{ lambda };
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
        m_ThisState = MSAsyncState::DONE;
        m_Error = std::current_exception();
    }

    void return_void() noexcept
    {
        m_ThisState = MSAsyncState::DONE;
    }

    auto yield_value(std::nullptr_t) noexcept
    {
        m_ThisState = MSAsyncState::DONE;
        return std::suspend_always{};
    }

    void result() const
    {
        if (m_Error) MSThrowError(MSError("result()" " at " __FILE__));
    }

    template<class U>
    auto await_transform(MSAsync<U> async)
    {
        return async;
    }

    template<class F>
    auto await_transform(F lambda)
    {
        using return_type = MSTraits<F>::return_data;

        struct MSLambdaAwaitable
        {
            F m_Lambda;
            std::future<return_type> m_Future;
            std::promise<return_type> m_Promise;
            MSAsyncState m_LastState = MSAsyncState::NONE;
            std::coroutine_handle<MSAsyncPromise> m_NextHandle;

            bool await_ready() const noexcept { return false; }

            void await_suspend(std::coroutine_handle<MSAsyncPromise> handle)
            {
                m_NextHandle = handle;
                m_LastState = m_NextHandle.promise().m_ThisState;
                m_NextHandle.promise().m_ThisState = MSAsyncState::AWAIT;
                m_Future = m_Promise.get_future();

                MSAwait<return_type> promise = [&](return_type && value)
                {
                    try
                    {
                        m_Promise.set_value(value);
                    }
                    catch (...)
                    {
                        MSThrowError(MSError("MSLambdaAwaitable.await_suspend() at " __FILE__));
                    }
                    m_NextHandle.resume();
                };
                m_Lambda(promise);
            }

            return_type await_resume()
            {
                m_NextHandle.promise().m_ThisState = m_LastState;
                if (m_NextHandle.promise().m_Error) MSThrowError(MSError("await_resume()" " at " __FILE__));
                return m_Future.get();
            }
        };
        return MSLambdaAwaitable{ lambda };
    }

    std::exception_ptr m_Error;
};

// ==================== Async<T> ====================

template <class T>
class MSAsync
{
public:
    using promise_type = MSAsyncPromise<T>;
    template<class U>
    using await_type = MSLambda<void(U)>;

    MSAsync() : m_ThisHandle(nullptr) {}

    explicit MSAsync(std::coroutine_handle<promise_type> handle) : m_ThisHandle(handle) {}

    MSAsync(MSAsync&& other) noexcept : m_ThisHandle(other.m_ThisHandle) { other.m_ThisHandle = nullptr; }

    ~MSAsync() { if (m_ThisHandle) m_ThisHandle.destroy(); }

    MSAsync(MSAsync const&) = delete;

    MSAsync& operator=(const MSAsync&) = delete;

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
            m_LastState = m_NextHandle.promise().m_ThisState;
            m_NextHandle.promise().m_ThisState = MSAsyncState::AWAIT;
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
            auto await_resume()
            {
                this->m_NextHandle.promise().m_ThisState = this->m_LastState;
                if (!this->m_NextHandle) throw std::runtime_error("invalid coroutine");
                return this->m_NextHandle.promise().result();
            }
        };
        return MSAsyncAwaitable { {MSAsyncState::NONE, m_ThisHandle} };
    }

    auto operator co_await() const && noexcept
    {
        struct MSAsyncAwaitable : MSAwaitBase
        {
            auto await_resume()
            {
                this->m_NextHandle.promise().m_ThisState = this->m_LastState;
                if (!this->m_NextHandle) throw std::runtime_error("invalid coroutine");
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
        return m_ThisHandle.promise().m_ThisState;
    }

    T value()
    {
        return m_ThisHandle.promise().result();
    }

private:
    std::coroutine_handle<promise_type> m_ThisHandle;
};

template<class T>
MSAsync<T> MSAsyncPromise<T>::get_return_object() noexcept
{
    return MSAsync<T>(std::coroutine_handle<MSAsyncPromise>::from_promise(*this));
}

inline MSAsync<void> MSAsyncPromise<void>::get_return_object() noexcept
{
    return MSAsync(std::coroutine_handle<MSAsyncPromise>::from_promise(*this));
}

#endif