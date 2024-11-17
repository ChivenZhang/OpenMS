#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
*=====================History========================
* Created by ChivenZhang@gmail.com.
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

	template<class T, class I, std::enable_if_t<std::is_same_v<T, I>, int> = 0>
	THnd<TRef<Bean>> create(uint32_t N = 0)
	{
		auto name = ((uint64_t)THash(typeid(T).name()) << 32) | N;
		auto result = m_Beans[name];
		if (result == nullptr)
		{
			result = m_Beans[name] = TNew<TRef<Bean>>();
		}
		if (result->get() == nullptr)
		{
			auto bean = TNew<Bean>();
			bean->Data = TNew<T>();
			bean->Cast = [](std::any& src, std::any& dst) { dst = src; };
			(*result) = bean;
		}
		return result;
	}

	template<class T, class I, std::enable_if_t<std::is_same_v<T, I> == false && std::is_base_of_v<I, T>, int> = 0>
	THnd<TRef<Bean>> create(uint32_t N = 0)
	{
		auto name1 = ((uint64_t)THash(typeid(I).name()) << 32) | N;
		auto name2 = ((uint64_t)THash(typeid(T).name()) << 32) | N;
		auto name3 = ((uint64_t)THash(typeid(I).name()) << 32) | 0;
		auto result = m_Beans[name1];
		if (result == nullptr)
		{
			result = m_Beans[name1] = m_Beans[name2] = TNew<TRef<Bean>>();
			m_Beans.emplace(name3, result);
		}
		if (result->get() == nullptr)
		{
			auto bean = TNew<Bean>();
			bean->Data = TNew<T>();
			bean->Cast = [](std::any& src, std::any& dst) { dst = TCast<I>(std::any_cast<TRef<T>>(src)); };
			(*result) = bean;
			if (N) (*m_Beans[name3]) = (*result);
		}
		return result;
	}

	template<class T>
	THnd<TRef<Bean>> depend(uint32_t N = 0)
	{
		auto name = ((uint64_t)THash(typeid(T).name()) << 32) | N;
		auto result = m_Beans[name];
		if (result == nullptr)
		{
			result = m_Beans[name] = TNew<TRef<Bean>>();
		}
		return result;
	}

public:
	TMap<uint64_t, TRef<TRef<Bean>>> m_Beans;
};

template<class T, class I, uint32_t N = 0>
class Resource
{
public:
	Resource()
	{
		m_Bean = BeanManager::Instance()->create<T, I>(N);
	}

protected:
	TRaw<T> bean() const
	{
		if (m_Bean.lock() == nullptr) return TRaw<T>();
		if (m_Bean.lock()->get() == nullptr) return TRaw<T>();
		auto result = std::any_cast<TRef<T>> (m_Bean.lock()->get()->Data);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Bean;
};

template<class T, uint32_t N = 0>
class Autowire
{
public:
	Autowire()
	{
		m_Bean = BeanManager::Instance()->depend<T>(N);
	}

protected:
	TRaw<T> bean() const
	{
		std::any dst;
		if (m_Bean.lock() == nullptr) return TRaw<T>();
		if (m_Bean.lock()->get() == nullptr) return TRaw<T>();
		m_Bean.lock()->get()->Cast(m_Bean.lock()->get()->Data, dst);
		auto result = std::any_cast<TRef<T>> (dst);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Bean;
};

template<class T, class I>
class ResourceThis
{
public:
	ResourceThis(uint32_t N = 0)
	{
		m_Bean = BeanManager::Instance()->create<T, I>(N);
	}

	TRaw<T> bean() const
	{
		if (m_Bean.lock() == nullptr) return TRaw<T>();
		if (m_Bean.lock()->get() == nullptr) return TRaw<T>();
		auto result = std::any_cast<TRef<T>> (m_Bean.lock()->get()->Data);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Bean;
};

template<class T>
class AutowireThis
{
public:
	AutowireThis(uint32_t N = 0)
	{
		m_Bean = BeanManager::Instance()->depend<T>(N);
	}

	TRaw<T> bean() const
	{
		std::any dst;
		if (m_Bean.lock() == nullptr) return TRaw<T>();
		if (m_Bean.lock()->get() == nullptr) return TRaw<T>();
		m_Bean.lock()->get()->Cast(m_Bean.lock()->get()->Data, dst);
		auto result = std::any_cast<TRef<T>> (dst);
		return result.get();
	}

private:
	THnd<TRef<Bean>> m_Bean;
};

// Autowire(Type or Interface of Type)
#define AUTOWIRE(T) Autowire<T, 0>
// Autowire(Type or Interface of Type, Name)
#define AUTOWIRE2(T, N) Autowire<T, THash(N)>
// Autowire(Type or Interface of Type)
#define AUTOWIRE_THIS(T) AutowireThis<T>(0).bean()
// Autowire(Type or Interface of Type, Name)
#define AUTOWIRE2_THIS(T, N) AutowireThis<T>(THash(N)).bean()

// Resource(Type, Name)
#define RESOURCE(T, N) Resource<T, T, THash(N)>
// Resource(Type, Interface of Type, Name)
#define RESOURCE2(T, I, N) Resource<T, I, THash(N)>
// Resource(Type, Name)
#define RESOURCE_THIS(T, N) ResourceThis<T, T>(THash(N)).bean()
// Resource(Type, Interface of Type, Name)
#define RESOURCE2_THIS(T, I, N) ResourceThis<T, I>(THash(N)).bean()