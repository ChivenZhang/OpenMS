#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang.
*
* =================================================*/
#include "MS.h"

class Bean
{
public:
	std::any Data;
	TLambda<void(std::any& src, std::any& dst)> Cast;
};

class BeanManager
{
public:
	static TRaw<BeanManager> Instance();

	template<class T, class I, uint32_t N = 0, std::enable_if_t<std::is_same_v<T, I>, int> = 0>
	THnd<TRef<Bean>> create()
	{
		auto result = m_Beans[N];
		if (result == nullptr)
		{
			result = m_Beans[N] = TNew<TRef<Bean>>();
		}
		if (result->get() == nullptr)
		{
			auto bean = TNew<Bean>();
			bean->Data = TNew<T>();
			bean->Cast = [](std::any& src, std::any& dst) { dst = src; };
			(*result.get()) = bean;
		}
		return result;
	}

	template<class T, class I, uint32_t N = 0, std::enable_if_t<!std::is_same_v<T, I>, int> = 0>
	THnd<TRef<Bean>> create()
	{
		auto result = m_Beans[N];
		if (result == nullptr)
		{
			result = m_Beans[N] = TNew<TRef<Bean>>();
		}
		if (result->get() == nullptr)
		{
			auto bean = TNew<Bean>();
			bean->Data = TNew<T>();
			bean->Cast = [](std::any& src, std::any& dst) { dst = TCast<I>(std::any_cast<TRef<T>>(src)); };
			(*result.get()) = bean;
		}
		return result;
	}

	template<class T, uint32_t N = 0>
	THnd<TRef<Bean>> depend()
	{
		auto result = m_Beans[N];
		if (result == nullptr)
		{
			result = m_Beans[N] = TNew<TRef<Bean>>();
		}
		return result;
	}

public:
	TMap<uint32_t, TRef<TRef<Bean>>> m_Beans;
};

template<class T, class I, uint32_t N = 0>
class Resource
{
public:
	Resource()
	{
		m_Data = BeanManager::Instance()->create<T, I, N>();
	}

protected:
	TRaw<T> get() const
	{
		if (m_Data.lock() == nullptr) return TRaw<T>();
		if (m_Data.lock()->get() == nullptr) return TRaw<T>();
		auto result = std::any_cast<TRef<T>> (m_Data.lock()->get()->Data);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Data;
};

template<class T, uint32_t N = 0>
class Autowire
{
public:
	Autowire()
	{
		m_Data = BeanManager::Instance()->depend<T, N>();
	}

protected:
	TRaw<T> get() const
	{
		std::any dst;
		if (m_Data.lock() == nullptr) return TRaw<T>();
		if (m_Data.lock()->get() == nullptr) return TRaw<T>();
		m_Data.lock()->get()->Cast(m_Data.lock()->get()->Data, dst);
		auto result = std::any_cast<TRef<T>> (dst);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Data;
};
#define AUTOWIRE(I, ID) Autowire<I, THash(ID)>
#define RESOURCE(T, ID) Resource<T, T, THash(ID)>
#define RESOURCE2(T, I, ID) Resource<T, I, THash(ID)>