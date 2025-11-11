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
    struct data_t
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

    MSAsync get_return_object()
    {
        return MSAsync{handle_type::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
        data().m_State = MSAsyncState::PEND;
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
                if (handle.promise().data().m_Handle) handle.promise().data().m_Handle.resume();
            }
        };
        return final_awaitable{};
    }

    void return_value(T value) noexcept
    {
        data().m_Value = std::move(value);
        data().m_State = MSAsyncState::DONE;
    }

    std::suspend_always yield_value(T value) noexcept
    {
        data().m_Value = std::move(value);
        data().m_State = MSAsyncState::DONE;
        return {};
    }

    void unhandled_exception()
    {
        data().m_Error = std::current_exception();
        data().m_State = MSAsyncState::DONE;
    }

    bool await_ready() const noexcept
    {
        return data().m_State == MSAsyncState::DONE;
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        data().m_Handle = handle;
        data().m_State = MSAsyncState::AWAIT;
    }

    T await_resume()
    {
        if (data().m_Error) std::rethrow_exception(data().m_Error);
        data().m_State = MSAsyncState::PEND;
        return data().m_Value;
    }

    template<class U>
    auto await_transform(MSAsync<U>&& awaitable)
    {
        return std::forward<MSAsync<U>>(awaitable);
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

            ~transform_awaitable() { MS_INFO("delete"); }
            bool await_ready() const noexcept { return false; }
            void await_suspend(std::coroutine_handle<MSAsync> h)
            {
                handle = h;
                handle.promise().data().m_State = MSAsyncState::AWAIT;
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
                    handle.resume();
                }).detach();
            }

            typename MSTraits<F>::return_type await_resume()
            {
                if (handle.promise().data().m_Error) std::rethrow_exception(handle.promise().data().m_Error);
                handle.promise().data().m_State = MSAsyncState::PEND;
                return future.get();
            }
        };
        return transform_awaitable{ func };
    }

    MSAsyncState state() const
    {
        return data().m_State;
    }

    T value()
    {
        if(data().m_State != MSAsyncState::DONE) throw std::runtime_error("pending promise");
        return await_resume();
    }

private:
    data_t& data()
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<handle_type>(m_Data).promise().data();
    }
    data_t const& data() const
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<handle_type>(m_Data).promise().data();
    }

private:
    std::variant<data_t, handle_type> m_Data;
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

    MSAsync get_return_object()
    {
        return MSAsync{from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
        data().m_State = MSAsyncState::PEND;
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
                if (handle.promise().data().m_Handle) handle.promise().data().m_Handle.resume();
            }
        };
        return final_awaitable{};
    }

    void return_void() noexcept
    {
        data().m_State = MSAsyncState::DONE;
    }

    std::suspend_always yield_value(std::nullptr_t) noexcept
    {
        data().m_State = MSAsyncState::DONE;
        return {};
    }

    void unhandled_exception()
    {
        data().m_Error = std::current_exception();
        data().m_State = MSAsyncState::DONE;
    }

    bool await_ready() const noexcept
    {
        auto& handle = data().m_Handle;
        return bool(handle) == false || bool(handle) && handle.done();
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        data().m_Handle = handle;
    }

    void await_resume()
    {
        if (data().m_Error) std::rethrow_exception(data().m_Error);
    }

    template<class U>
    auto await_transform(MSAsync<U>&& awaitable)
    {
        return std::forward<MSAsync<U>>(awaitable);
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
                handle.promise().data().m_State = MSAsyncState::AWAIT;
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
                    handle.resume();
                }).detach();
            }

            typename MSTraits<F>::return_type await_resume()
            {
                if (handle.promise().data().m_Error) std::rethrow_exception(handle.promise().data().m_Error);
                handle.promise().data().m_State = MSAsyncState::PEND;
                return future.get();
            }
        };
        return transform_awaitable{ func };
    }

    MSAsyncState state() const
    {
        return data().m_State;
    }

private:
    data_t& data()
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<std::coroutine_handle<MSAsync>>(m_Data).promise().data();
    }
    data_t const& data() const
    {
        if (std::holds_alternative<data_t>(m_Data)) return std::get<data_t>(m_Data);
        return std::get<std::coroutine_handle<MSAsync>>(m_Data).promise().data();
    }

private:
    std::variant<data_t, std::coroutine_handle<MSAsync>> m_Data;
};

#endif