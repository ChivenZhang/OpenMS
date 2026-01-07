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
#include "Mailbox/Private/MailBox.h"
#include "Utility/Timer.h"

class Service : public MailBox
{
public:
	using MailBox::MailBox;
	using session_t = MSLambda<void(MSStringView)>;
	using method_t = MSLambda<MSAsync<MSString>(MSStringView)>;
	struct request_t
	{
		uint32_t Method;
		char Buffer[0];
	};

	void unbind();
	bool unbind(uint32_t method);
	bool bind(uint32_t method, method_t&& callback);
	bool call(uint32_t service, uint32_t method, uint32_t forward, uint32_t timeout, MSStringView request, MSString& response);
	bool async(uint32_t service, uint32_t method, uint32_t forward, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)>&& callback);
	MSAsync<MSString> async(uint32_t service, uint32_t method, uint32_t forward, uint32_t timeout, MSStringView request);

	bool unbind(MSStringView method);
	bool bind(MSStringView method, method_t&& callback);
	bool call(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSStringView request, MSString& response);
	bool async(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)>&& callback);
	MSAsync<MSString> async(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSStringView request);

	template<class F, std::enable_if_t<!std::is_same_v<typename TTraits<F>::return_data, TTraits<method_t>::return_data> || !std::is_same_v<typename TTraits<F>::argument_datas, TTraits<method_t>::argument_datas>, int> = 0>
	bool bind(MSStringView method, F&& callback)
	{
		return this->bind(method, [callback](MSStringView input)->MSAsync<MSString>
		{
			typename TTraits<F>::argument_datas request;
			if constexpr (TTraits<F>::argument_count)
			{
				if (MSTypeC(MSString(input), request) == false) co_return {};
			}
			if constexpr (std::is_same_v<typename TTraits<F>::return_data, MSAsync<void>>)
			{
				co_await std::apply(callback, request);
				co_return {};
			}
			else
			{
				auto response = co_await std::apply(callback, request);
				MSString result;
				MSTypeC(response, result);
				co_return result;
			}
		});
	}

	template<class T, class...Args>
	auto call(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSTuple<Args...>&& args)
	{
		if constexpr (std::is_void_v<T>)
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(args, request) == false) return false;
			}
			MSString response;
			return this->call(service, method, forward, timeout, request, response);
		}
		else
		{
			MSString request;
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(args, request) == false) return MSBinary{T(), false};
			}
			MSString response;
			if (this->call(service, method, forward, timeout, request, response) == false)
			{
				return MSBinary{T(), false};
			}
			T result;
			if (MSTypeC(response, result) == false) return MSBinary{T(), false};
			return MSBinary{result, true};
		}
	}

	template<class F, class...Args>
	bool async(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSTuple<Args...>&& args, F&& callback)
	{
		MSString request;
		if constexpr (sizeof...(Args) != 0)
		{
			if (MSTypeC(args, request) == false) return false;
		}
		return this->async(service, method, forward, timeout, request, [callback](MSStringView response)
		{
			typename TTraits<F>::argument_datas result;
			if constexpr (TTraits<F>::argument_count) MSTypeC(response, std::get<0>(result));
			std::apply(callback, result);
		});
	}

	template<class T, class...Args>
	MSAsync<T> async(MSStringView service, MSStringView method, MSStringView forward, uint32_t timeout, MSTuple<Args...>&& args)
	{
		if constexpr (std::is_void_v<T>)
		{
			co_return co_await [=, this](MSAwait<void> const& promise) mutable
			{
				this->async(service, method, forward, timeout, std::forward<MSTuple<Args...>>(args), [promise]()
				{
					promise();
				});
			};
		}
		else
		{
			co_return co_await [=, this](MSAwait<T> const& promise) mutable
			{
				this->async(service, method, forward, timeout, std::forward<MSTuple<Args...>>(args), [promise](T result)
				{
					promise(result);
				});
			};
		}
	}

protected:
	IMailTask read(IMail mail) override;

protected:
	Timer m_Timer;
	MSMutex m_LockMethod;
	MSMutex m_LockSession;
	MSAtomic<uint32_t> m_SessionID;
	MSMap<uint32_t, method_t> m_MethodMap;
	MSMap<uint32_t, session_t> m_SessionMap;
};
