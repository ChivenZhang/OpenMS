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

template <class T>
class MSAsync : public std::coroutine_handle<MSAsync<T>>
{
public:
    using promise_type = MSAsync;

    MSAsync() : m_Data(data_t{})
    {
    }

    ~MSAsync()
    {
        if(std::holds_alternative<std::coroutine_handle<MSAsync>>(m_Data))
        {
            std::get<std::coroutine_handle<MSAsync>>(m_Data).destroy();
        }
    }

    MSAsync get_return_object()
    {
        return MSAsync{std::coroutine_handle<MSAsync>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
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

    void return_value(T value) noexcept
    {
        data().m_Value = std::move(value);
    }

    void unhandled_exception()
    {
        data().m_Error = std::current_exception();
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

    T await_resume()
    {
        if (data().m_Error) std::rethrow_exception(data().m_Error);
        return data().m_Value;
    }

    T value()
    {
        if(await_ready() == false) throw std::runtime_error("pending promise");
        return await_resume();
    }

private:
    struct data_t
    {
        T m_Value{};
        std::exception_ptr m_Error{};
        std::coroutine_handle<> m_Handle{};
    };
    explicit MSAsync(std::coroutine_handle<MSAsync> handle) : m_Data(handle)
    {
    }
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

// Async<void>

template <>
class MSAsync<void> : public std::coroutine_handle<MSAsync<void>>
{
public:
    using promise_type = MSAsync;

    MSAsync() : m_Data(data_t{})
    {
    }

    ~MSAsync()
    {
        if(std::holds_alternative<std::coroutine_handle<MSAsync>>(m_Data))
        {
            std::get<std::coroutine_handle<MSAsync>>(m_Data).destroy();
        }
    }

    MSAsync get_return_object()
    {
        return MSAsync{std::coroutine_handle<MSAsync>::from_promise(*this)};
    }

    std::suspend_never initial_suspend() noexcept
    {
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
    }

    void unhandled_exception()
    {
        data().m_Error = std::current_exception();
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

private:
    struct data_t
    {
        std::exception_ptr m_Error{};
        std::coroutine_handle<> m_Handle{};
    };
    explicit MSAsync(std::coroutine_handle<MSAsync> handle) : m_Data(handle)
    {
    }
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

// Await<T>

template<class T>
class MSAwait
{
public:
    struct handle_t
    {
        handle_t(std::coroutine_handle<> handle, T& value) : m_Handle(handle), m_Value(value) {}
        handle_t(handle_t const& other) : m_Handle(other.m_Handle), m_Value(other.m_Value) {}
        void setValue(T&& value)
        {
            MS_INFO("#3 %p", m_Handle.address());
            if (bool(m_Handle) && m_Handle.done() == false)
            {
                // m_Value = value;
                m_Handle.resume();
            }
        }

    private:
        std::coroutine_handle<> m_Handle;
        T& m_Value;
    };

public:
    MSAwait(MSLambda<void(handle_t)> callback)
        :
        m_Method(callback)
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        MS_INFO("#1 %p", handle.address());
        m_Handle = handle;
        m_Method({m_Handle, m_Value});
    }

    T await_resume()
    {
        MS_INFO("#2 %p", m_Handle.address());
        return m_Value;
    }

private:
    T m_Value = {};
    std::coroutine_handle<> m_Handle;
    MSLambda<void(handle_t)> m_Method;
};

// Await<void>

template<>
class MSAwait<void>
{
public:
    struct handle_t
    {
        handle_t(std::coroutine_handle<> handle)
            :
            m_Handle(handle)
        {
        }
        handle_t(handle_t&&) = default;
        handle_t(handle_t const&) = default;
        void setValue()
        {
            if (m_Handle && m_Handle.done() == false) m_Handle.resume();
        }

    private:
        std::coroutine_handle<> m_Handle;
    };

public:
    MSAwait(MSLambda<void(handle_t)> callback)
        :
        m_Method(callback)
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> handle)
    {
        m_Handle = handle;
        m_Method({m_Handle});
    }

    void await_resume() { }

private:
    std::coroutine_handle<> m_Handle;
    MSLambda<void(handle_t)> m_Method;
};

#endif