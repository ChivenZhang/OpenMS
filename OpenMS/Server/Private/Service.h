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
	using method_t = MSLambda<MSAsync<MSString>(MSStringView)>;
	using session_t = MSLambda<void(MSStringView)>;
	using MailBox::MailBox;
	bool bind(MSStringView method, method_t&& callback);
	bool call(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSString& response);
	bool async(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)> callback);

	template<class F, std::enable_if_t<!std::is_same_v<typename MSTraits<F>::return_data, MSTraits<method_t>::return_data> || !std::is_same_v<typename MSTraits<F>::argument_datas, MSTraits<method_t>::argument_datas>, int> = 0>
	bool bind(MSStringView method, F callback)
	{
		return this->bind(method, [callback](MSStringView input)->MSAsync<MSString>
		{
			typename MSTraits<F>::argument_datas request;
			if constexpr (MSTraits<F>::argument_count)
			{
				if (MSTypeC(MSString(input), request) == false) co_return {};
			}
			if constexpr (std::is_same_v<typename MSTraits<F>::return_data, MSAsync<void>>)
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
	auto call(MSStringView service, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args)
	{
		MSString request;
		if constexpr (std::is_void_v<T>)
		{
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(args, request) == false) return false;
			}
			MSString response;
			return this->call(service, method, timeout, request, response);
		}
		else
		{
			if constexpr (sizeof...(Args) != 0)
			{
				if (MSTypeC(args, request) == false) return MSBinary{T(), false};
			}
			MSString response;
			if (this->call(service, method, timeout, request, response) == false)
			{
				return MSBinary{T(), false};
			}
			T result;
			if (MSTypeC(response, result) == false) return MSBinary{T(), false};
			return MSBinary{result, true};
		}
	}

	template<class F, class...Args>
	bool async(MSStringView service, MSStringView method, uint32_t timeout, MSTuple<Args...>&& args, F callback)
	{
		MSString request;
		if constexpr (sizeof...(Args) != 0)
		{
			if (MSTypeC(args, request) == false) return false;
		}
		return this->async(service, method, timeout, request, [callback](MSStringView response)
		{
			typename MSTraits<F>::argument_datas result;
			if constexpr (MSTraits<F>::argument_count) MSTypeC(response, std::get<0>(result));
			std::apply(callback, result);
		});
	}

protected:
	IMailTask read(IMail mail) final;

protected:
	Timer m_Timer;
	MSMutex m_MutexMethod;
	MSMutex m_MutexSession;
	MSAtomic<uint32_t> m_SessionID;
	MSMap<uint32_t, method_t> m_MethodMap;
	MSMap<uint32_t, session_t> m_SessionMap;
};
