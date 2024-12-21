#pragma once
/*=================================================
* Copyright @ 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang at 2024/12/20 15:57:08.
*
* =================================================*/
#include "MS.h"
#include <coroutine>
class IMailContext;

template <class T>
struct IMailPromise
{
	struct IPromiseType
	{
		using value_type = std::remove_reference_t<T>;
		using reference_type = std::conditional_t<std::is_reference_v<T>, T, T&>;
		using pointer_type = value_type*;
		using handle_type = std::coroutine_handle<IPromiseType>;

		IMailPromise get_return_object() { return handle_type::from_promise(*this); }
		static auto initial_suspend() { return std::suspend_always{}; }
		auto final_suspend() noexcept { m_Done = true; return std::suspend_never{}; }
		static void unhandled_exception() {}
		static void return_void() {}
		template<typename U = T, std::enable_if_t<!std::is_rvalue_reference_v<U>, int> = 0>
		auto yield_value(std::remove_reference_t<T>& value) noexcept
		{
			m_Value = std::addressof(value);
			return std::suspend_always{};
		}
		auto yield_value(std::remove_reference_t<T>&& value) noexcept
		{
			m_Value = std::addressof(value);
			return std::suspend_always{};
		}
		reference_type value() const noexcept
		{
			return static_cast<reference_type>(*m_Value);
		}

		bool is_done() const { return m_Done; }

	private:
		bool m_Done = false;
		pointer_type m_Value = nullptr;
	};
	using promise_type = IPromiseType;

	IMailPromise() = default;
	IMailPromise(IMailPromise const&) = delete;
	IMailPromise(IMailPromise&& other) noexcept
		: m_Handle(other.m_Handle)
	{
		other.m_Handle = nullptr;
	}
	IMailPromise& operator = (IMailPromise const&) = delete;
	IMailPromise& operator = (IMailPromise&& other) noexcept
	{
		if (this != &other)
		{
			m_Handle = other.m_Handle.from_promise(other.m_Handle.promise());
			other.m_Handle = nullptr;
		}
		return *this;
	}

	static bool await_ready() { return false; }
	static void await_suspend(std::coroutine_handle<>) {}
	static void await_resume() {}

	operator T() const { return m_Handle.promise().value(); }

	constexpr operator std::coroutine_handle<>() const noexcept
	{
		return m_Handle.operator std::coroutine_handle<>();
	}

	bool valid() const noexcept
	{
		return m_Handle.operator bool();
	}

	bool done() const noexcept
	{
		return valid() && m_Handle.promise().is_done();
	}

	void resume() const
	{
		return m_Handle.resume();
	}

	void destroy() const noexcept
	{
		return m_Handle.destroy();
	}

private:
	IMailPromise(typename promise_type::handle_type&& handle) : m_Handle(handle){}
	typename promise_type::handle_type m_Handle;
};

template <>
struct IMailPromise<void>
{
	struct IPromiseType
	{
		using handle_type = std::coroutine_handle<IPromiseType>;
		IMailPromise get_return_object() { return handle_type::from_promise(*this); }
		static auto initial_suspend() { return std::suspend_always{}; }
		auto final_suspend() noexcept { m_Done = true; return std::suspend_never{}; }
		static void unhandled_exception() {}
		static void return_void() {}
		static auto yield_value(std::nullptr_t) { return std::suspend_always{}; }
		bool is_done() const { return m_Done; }

	private:
		bool m_Done = false;
	};
	using promise_type = IPromiseType;

	IMailPromise() = default;
	IMailPromise(IMailPromise const&) = delete;
	IMailPromise(IMailPromise&& other) noexcept
		: m_Handle(other.m_Handle)
	{
		other.m_Handle = nullptr;
	}
	IMailPromise& operator = (IMailPromise const&) = delete;
	IMailPromise& operator = (IMailPromise&& other) noexcept
	{
		if (this != &other)
		{
			m_Handle = other.m_Handle.from_promise(other.m_Handle.promise());
			other.m_Handle = nullptr;
		}
		return *this;
	}

	static bool await_ready() { return false; }
	static void await_suspend(std::coroutine_handle<>) {}
	static void await_resume() {}

	constexpr operator std::coroutine_handle<>() const noexcept
	{
		return m_Handle.operator std::coroutine_handle<>();
	}

	bool valid() const noexcept
	{
		return m_Handle.operator bool();
	}

	bool done() const noexcept
	{
		return valid() && m_Handle.promise().is_done();
	}

	void resume() const
	{
		return m_Handle.resume();
	}

	void destroy() const noexcept
	{
		return m_Handle.destroy();
	}

private:
	IMailPromise(promise_type::handle_type&& handle) : m_Handle(handle){}
	promise_type::handle_type m_Handle;
};

using IMailResult = IMailPromise<void>;

/// @brief Interface for mail
struct IMail
{
	MSString From, To, Data;
	uint32_t SID; // Session
};

/// @brief Interface for mailbox
class OPENMS_API IMailBox
{
public:
	virtual ~IMailBox() = default;

	virtual bool send(IMail&& mail) = 0;

	// virtual bool send(IMail&& mail, IMail& response) = 0;

	virtual IMailResult sign(IMail&& mail) = 0;

	virtual bool create(MSString address, MSLambda<MSRef<IMailBox>(MSRaw<IMailContext>)> factory) = 0;

	virtual bool cancel(MSString address) = 0;

	virtual bool exist(MSString address) const = 0;

	template<class T, class... Args>
	bool create(MSString address, Args... args)
	{
		static_assert(std::is_base_of_v<IMailBox, T>);
		return create(address, [&](MSRaw<IMailContext> context){ return MSNew<T>(context, std::forward<Args>(args)...); });
	}
};