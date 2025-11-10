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

// Async<T>

enum class MSAsyncState
{
    NONE = 0,
    PEND,
    AWAIT,
    DONE,
};

template <class T>
class MSAsync : public std::coroutine_handle<MSAsync<T>>
{
public:
    using promise_type = MSAsync;
	using handle_type = std::coroutine_handle<promise_type>;
    struct async_t
    {
        T m_Value{};
        MSAsyncState m_State = MSAsyncState::NONE;
        std::exception_ptr m_Error{};
        std::coroutine_handle<> m_Handle{};
    };

    MSAsync() = default;

    explicit MSAsync(handle_type handle) : m_Data(handle)
    {
    }

    ~MSAsync()
    {
        if(std::holds_alternative<handle_type>(m_Data))
        {
            std::get<handle_type>(m_Data).destroy();
        }
    }

    MSAsync get_return_object()
    {
        return MSAsync{handle_type::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
        async_data().m_State = MSAsyncState::PEND;
        return {};
    }

    auto final_suspend() noexcept
    {
        struct final_awaitable
        {
            bool await_ready() const noexcept { return false; }
            void await_resume() noexcept {}
            void await_suspend(std::coroutine_handle<MSAsync> handle) noexcept
            {
                if (handle.promise().async_data().m_Handle) handle.promise().async_data().m_Handle.resume();
            }
        };
        return final_awaitable{};
    }

    void return_value(T value) noexcept
    {
        async_data().m_Value = std::move(value);
        async_data().m_State = MSAsyncState::DONE;
    }

    std::suspend_always yield_value(T value) noexcept
    {
        async_data().m_Value = std::move(value);
        async_data().m_State = MSAsyncState::DONE;
        return {};
    }

    void unhandled_exception()
    {
        async_data().m_Error = std::current_exception();
        async_data().m_State = MSAsyncState::DONE;
    }

    bool await_ready() const noexcept
    {
        return async_data().m_State == MSAsyncState::DONE;
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        async_data().m_Handle = handle;
        async_data().m_State = MSAsyncState::AWAIT;
    }

    T await_resume()
    {
        if (async_data().m_Error) std::rethrow_exception(async_data().m_Error);
        async_data().m_State = MSAsyncState::PEND;
        return async_data().m_Value;
    }

    template<class U>
    auto await_transform(MSAsync<U> awaitable)
    {
        return awaitable;
    }

    template<class F>
    auto await_transform(F func)
    {
        struct transform_awaitable
        {
            decltype(func) method;
            std::coroutine_handle<MSAsync> handle;
            std::future<typename MSTraits<F>::return_type> future;
            std::promise<typename MSTraits<F>::return_type> promise;

            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<MSAsync> h)
            {
                handle = h;
                handle.promise().async_data().m_State = MSAsyncState::AWAIT;
                future = promise.get_future();
                std::thread([&]() mutable
                {
                    if constexpr(std::is_void_v<typename MSTraits<F>::return_type>)
                    {
                        method();
                        promise.set_value();
                    }
                    else
                    {
                        auto result = method();
                        promise.set_value(result);
                    }
                    h.resume();
                }).detach();
            }

            typename MSTraits<F>::return_type await_resume()
            {
                if (handle.promise().async_data().m_Error) std::rethrow_exception(handle.promise().async_data().m_Error);
                handle.promise().async_data().m_State = MSAsyncState::PEND;
                return future.get();
            }
        };
        return transform_awaitable{ func };
    }

    MSAsyncState state() const
    {
        return async_data().m_State;
    }

    T value()
    {
        if(async_data().m_State != MSAsyncState::DONE) throw std::runtime_error("pending promise");
        return await_resume();
    }

private:
    async_t& async_data()
    {
        if (std::holds_alternative<async_t>(m_Data)) return std::get<async_t>(m_Data);
        return std::get<handle_type>(m_Data).promise().async_data();
    }
    async_t const& async_data() const
    {
        if (std::holds_alternative<async_t>(m_Data)) return std::get<async_t>(m_Data);
        return std::get<handle_type>(m_Data).promise().async_data();
    }

private:
    std::variant<async_t, handle_type> m_Data;
};

// Async<void>

template <>
class MSAsync<void> : public std::coroutine_handle<MSAsync<void>>
{
public:
    using promise_type = MSAsync;
	using handle_type = std::coroutine_handle<promise_type>;

    struct data_t
    {
        MSAsyncState m_State = MSAsyncState::NONE;
        std::exception_ptr m_Error{};
        std::coroutine_handle<> m_Handle{};
    };

    MSAsync()
    {
    }

    explicit MSAsync(handle_type handle) : m_Data(handle)
    {
    }

    ~MSAsync()
    {
        if(std::holds_alternative<handle_type>(m_Data))
        {
            std::get<handle_type>(m_Data).destroy();
        }
    }

    MSAsync get_return_object()
    {
        return MSAsync{from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
        async_data().m_State = MSAsyncState::PEND;
        return {};
    }

    auto final_suspend() noexcept
    {
        struct final_awaitable
        {
            bool await_ready() const noexcept
            {
                return false;
            }
            void await_resume() noexcept
            {
            }
            void await_suspend(std::coroutine_handle<MSAsync> handle) noexcept
            {
                if (handle.promise().async_data().m_Handle) handle.promise().async_data().m_Handle.resume();
            }
        };
        return final_awaitable{};
    }

    void return_void() noexcept
    {
        async_data().m_State = MSAsyncState::DONE;
    }

    std::suspend_always yield_value(std::nullptr_t) noexcept
    {
        async_data().m_State = MSAsyncState::DONE;
        return {};
    }

    void unhandled_exception()
    {
        async_data().m_Error = std::current_exception();
        async_data().m_State = MSAsyncState::DONE;
    }

    bool await_ready() const noexcept
    {
        auto& handle = async_data().m_Handle;
        return bool(handle) == false || bool(handle) && handle.done();
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        async_data().m_Handle = handle;
    }

    void await_resume()
    {
        if (async_data().m_Error) std::rethrow_exception(async_data().m_Error);
    }

    template<class U>
    auto await_transform(MSAsync<U> awaitable)
    {
        return awaitable;
    }

    template<class F>
    auto await_transform(F func)
    {
        struct transform_awaitable
        {
            decltype(func) method;
            std::coroutine_handle<MSAsync> handle;
            std::future<typename MSTraits<F>::return_type> future;
            std::promise<typename MSTraits<F>::return_type> promise;

            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<MSAsync> h)
            {
                handle = h;
                handle.promise().async_data().m_State = MSAsyncState::AWAIT;
                future = promise.get_future();
                std::thread([&]() mutable
                {
                    if constexpr(std::is_void_v<typename MSTraits<F>::return_type>)
                    {
                        method();
                        promise.set_value();
                    }
                    else
                    {
                        promise.set_value(method());
                    }
                    h.resume();
                }).detach();
            }

            typename MSTraits<F>::return_type await_resume()
            {
                if (handle.promise().async_data().m_Error) std::rethrow_exception(handle.promise().async_data().m_Error);
                handle.promise().async_data().m_State = MSAsyncState::PEND;
                return future.get();
            }
        };
        return transform_awaitable{ func };
    }

    MSAsyncState state() const
    {
        return async_data().m_State;
    }

private:
    data_t& async_data()
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<std::coroutine_handle<MSAsync>>(m_Data).promise().async_data();
    }
    data_t const& async_data() const
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<std::coroutine_handle<MSAsync>>(m_Data).promise().async_data();
    }

private:
    std::variant<data_t, std::coroutine_handle<MSAsync>> m_Data;
};

#endif