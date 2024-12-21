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
	struct promise_type
	{
		IMailPromise<T> get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
		static auto initial_suspend() { return std::suspend_never{}; }
		auto final_suspend() noexcept { m_Finished = true; return std::suspend_never{}; }
		static void return_void() {}
		static void unhandled_exception() {}
		static auto yield_value(std::nullptr_t) { return std::suspend_always{}; }
		bool is_done() const { return m_Finished; }

	private:
		bool m_Finished = false;
	};

	IMailPromise(std::coroutine_handle<promise_type>&& handle = {})
		: m_Handle(handle)
	{
	}

	IMailPromise(IMailPromise&& value) noexcept
		: m_Handle(value.m_Handle.from_promise(value.m_Handle.promise()))
	{
		value.m_Handle = nullptr;
	}

	IMailPromise& operator = (IMailPromise&& value) noexcept
	{
		m_Handle = value.m_Handle.from_promise(value.m_Handle.promise());
		value.m_Handle = nullptr;
		return *this;
	}

	static bool await_ready() { return false; }
	static void await_suspend(std::coroutine_handle<>) {}
	static void await_resume() {}
	std::coroutine_handle<promise_type> const& get_handle() const { return m_Handle; }

private:
	std::coroutine_handle<promise_type> m_Handle;
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