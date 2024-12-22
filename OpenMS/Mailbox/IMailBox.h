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
#include <variant>
#include <coroutine>
class IMailContext;

template <typename T>
struct IMailTask
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		using value_type = std::remove_reference_t<T>;
		value_type value = value_type();
		std::exception_ptr error = nullptr;

		IMailTask get_return_object() { return handle_type::from_promise(*this); }
		static std::suspend_always initial_suspend() { return {}; }
		static std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { error = std::current_exception(); }
		void return_value(T val) { value = std::move(val); }
		std::suspend_always yield_value(T val) { value = std::move(val); return {}; }
	};

	static bool await_ready() { return false; }
	auto await_suspend(std::coroutine_handle<>) { return handle; }
	auto await_resume() { return handle.promise().value; }

	IMailTask(handle_type h) : handle(h) {}
	~IMailTask() { if (handle) handle.destroy(); }
	IMailTask(IMailTask const& other) noexcept = delete;
	IMailTask& operator = (IMailTask const&) noexcept = delete;
	IMailTask(IMailTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	IMailTask& operator = (IMailTask&& other) noexcept
	{
		if (this != &other)
		{
			if (handle) handle.destroy();
			handle = other.handle;
			other.handle = nullptr;
		}
		return *this;
	}

	bool good() const { return handle.operator bool(); }
	bool done() const { return handle.done(); }
	void resume() const { handle.resume(); }
	operator T() const { return handle.promise().value; }

private:
	handle_type handle;
};

template <>
struct IMailTask<void>
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		std::exception_ptr error = nullptr;

		IMailTask get_return_object() { return handle_type::from_promise(*this); }
		static std::suspend_always initial_suspend() { return {}; }
		static std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() { error = std::current_exception(); }
		static void return_void() {}
		static std::suspend_always yield_value(nullptr_t) { return {}; }
	};

	static bool await_ready() { return false; }
	auto await_suspend(std::coroutine_handle<>) const { return handle; }
	static void await_resume() {}

	IMailTask() = default;
	~IMailTask() { if (handle) handle.destroy(); }
	IMailTask(handle_type h) : handle(h) {}
	IMailTask(IMailTask const& other) noexcept = delete;
	IMailTask& operator = (IMailTask const&) noexcept = delete;
	IMailTask(IMailTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	IMailTask& operator = (IMailTask&& other) noexcept
	{
		if (this != &other)
		{
			if (handle) handle.destroy();
			handle = other.handle;
			other.handle = nullptr;
		}
		return *this;
	}

	bool good() const { return handle.operator bool(); }
	bool done() const { return handle.done(); }
	void resume() const { handle.resume(); }

private:
	handle_type handle;
};
using IMailResult = IMailTask<void>;

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