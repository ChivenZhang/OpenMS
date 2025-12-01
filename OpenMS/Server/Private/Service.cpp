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
#include "Service.h"

struct request_t
{
	uint32_t Method;
	char Buffer[0];
};

bool Service::bind(uint32_t method, method_t&& callback)
{
	MSMutexLock lock(m_MutexMethod);
	auto result = m_MethodMap.emplace(method, callback);
	return result.second;
}

bool Service::call(uint32_t service, uint32_t method, uint32_t timeout, MSStringView request, MSString& response)
{
	MSString input(sizeof(request_t) + request.size(), 0);
	auto& requestView = *(request_t*)input.data();
	requestView.Method = method;
	if (request.empty() == false) ::memcpy(requestView.Buffer, request.data(), request.size());

	IMail mail = {};
	mail.To = service;
	mail.Body = input;
	mail.Type = 1;
	mail.Date = ++m_SessionID;

	MSPromise<MSString> promise;
	auto future = promise.get_future();
	{
		MSMutexLock lock(m_MutexSession);
		m_SessionMap.emplace(mail.Date, [&](MSStringView output)
		{
			promise.set_value(MSString(output));
		});
	}

	send(mail);

	auto status = future.wait_for(std::chrono::milliseconds(timeout));
	{
		MSMutexLock lock(m_MutexSession);
		m_SessionMap.erase(mail.Date);
	}
	if (status == std::future_status::ready)
	{
		response = std::move(future.get());
		return true;
	}
	return false;
}

bool Service::async(uint32_t service, uint32_t method, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)>&& callback)
{
	MSString input(sizeof(request_t) + request.size(), 0);
	auto& requestView = *(request_t*)input.data();
	requestView.Method = method;
	if (request.empty() == false) ::memcpy(requestView.Buffer, request.data(), request.size());

	IMail mail = {};
	mail.To = service;
	mail.Body = input;
	mail.Type = 1;
	mail.Date = ++m_SessionID;

	{
		MSMutexLock lock(m_MutexSession);
		m_SessionMap.emplace(mail.Date, [callback](MSStringView output)
		{
			if (callback) callback(output);
		});
	}

	send(mail);

	m_Timer.start(timeout, 0, [session = mail.Date, this](uint32_t)
	{
		session_t response;
		{
			MSMutexLock lock(m_MutexSession);
			auto result = m_SessionMap.find(session);
			if (result != m_SessionMap.end())
			{
				response = result->second;
				m_SessionMap.erase(result);
			}
		}
		if (response) response({});
	});
	return true;
}

bool Service::bind(MSStringView method, method_t && callback)
{
	return bind(MSHash(method), std::move(callback));
}

bool Service::call(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSString& response)
{
	return call(MSHash(service), MSHash(method), timeout, request, response);
}

bool Service::async(MSStringView service, MSStringView method, uint32_t timeout, MSStringView request, MSLambda<void(MSStringView)>&& callback)
{
	return async(MSHash(service), MSHash(method), timeout, request, std::move(callback));
}

IMailTask Service::read(IMail mail)
{
	switch (mail.Type)
	{
	case 0:
		if (sizeof(request_t) <= mail.Body.size())
		{
			auto& request = *(request_t*)mail.Body.data();
			method_t method;
			{
				MSMutexLock lock(m_MutexMethod);
				auto result = m_MethodMap.find(request.Method);
				if (result != m_MethodMap.end()) method = result->second;
			}
			if (method)
			{
				auto input = MSStringView(request.Buffer, mail.Body.size() - sizeof(request_t));
				co_await method(input);
			}
			else
			{
				co_await onRead(mail);
			}
		} break;
	case 1:
		if (sizeof(request_t) <= mail.Body.size())
		{
			auto& request = *(request_t*)mail.Body.data();
			method_t method;
			{
				MSMutexLock lock(m_MutexMethod);
				auto result = m_MethodMap.find(request.Method);
				if (result != m_MethodMap.end()) method = result->second;
			}
			MSString response;
			if (method)
			{
				auto input = MSStringView(request.Buffer, mail.Body.size() - sizeof(request_t));
				response = co_await method(input);
			}
			mail.Type = 2;
			mail.Body = response;
			std::swap(mail.From, mail.To);
			send(mail);
		} break;
	case 2:
		{
			session_t response;
			{
				MSMutexLock lock(m_MutexSession);
				auto result = m_SessionMap.find(mail.Date);
				if (result != m_SessionMap.end())
				{
					response = result->second;
					m_SessionMap.erase(result);
				}
			}
			if (response) response(mail.Body);
		} break;
	default: break;
	}
	co_return;
}

IMailTask Service::onRead(IMail mail)
{
	co_return;
}
