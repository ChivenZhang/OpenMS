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

#if defined( _MSVC_LANG )
#	define OPENMS_CPLUSPLUS _MSVC_LANG
#else
#	define __FUNCTION__ __func__
#	define OPENMS_CPLUSPLUS __cplusplus
#endif
#if 201703L < OPENMS_CPLUSPLUS
#	define OPENMS_CPP_VERSION 20
#elif 201402L < OPENMS_CPLUSPLUS
#	define OPENMS_CPP_VERSION 17
#elif 201103L < OPENMS_CPLUSPLUS
#	define OPENMS_CPP_VERSION 14
#elif 199711L < OPENMS_CPLUSPLUS
#	define OPENMS_CPP_VERSION 11
#else
#	error "At least c++ standard version 11"
#endif

// ============================================

#ifdef OPENMS_SHARED_LIBRARY
#	if defined(_WIN32)
#		define OPENMS_API __declspec(dllexport)
#		define OPENMS_C_API extern "C" __declspec(dllexport)
#	else
#		define OPENMS_API __attribute__((visibility("default")))
#		define OPENMS_C_API extern "C" __attribute__((visibility("default")))
#	endif
#else
#	if defined(_WIN32)
#		define OPENMS_API 
#		define OPENMS_C_API extern "C" 
#	else
#		define OPENMS_API 
#		define OPENMS_C_API 
#	endif
#endif

#ifdef _WIN32
#	ifdef _DEBUG
#		define OPENMS_DEBUG
#	endif
#endif

// ============================================

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(error:4150)
#pragma warning(disable:4250)
#pragma warning(disable:4200)
#pragma warning(disable:26812)
#pragma warning(disable:26815)
#pragma warning(disable:26816)

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <iostream>
#include <utility>
#include <algorithm>
#include <sstream>
#include <memory>
#include <string>
#include <array>
#include <span>
#include <vector>
#include <deque>
#include <list>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <queue>
#include <stack>
#include <bitset>
#include <exception>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <condition_variable>
#include <any>

// ============================================

template<class T>
using TRaw = T*;
template<class T>
using TRef = std::shared_ptr<T>;
template<class T>
using THnd = std::weak_ptr<T>;
using TString = std::string;
using TCString = const char*;
using TWString = std::wstring;
#if 20 <= OPENMS_CPP_VERSION
using TString8 = std::u8string;
#else
using TString8 = std::string;
#endif
using TString16 = std::u16string;
using TString32 = std::u32string;
#if 20 <= OPENMS_CPP_VERSION
using TStringView = std::string_view;
using TString8View = std::u8string_view;
using TWStringView = std::wstring_view;
using TString16View = std::u16string_view;
using TString32View = std::u32string_view;
#endif
template <class T, size_t N>
using TArray = std::array<T, N>;
#if 20 <= OPENMS_CPP_VERSION
template <class T, size_t N = std::dynamic_extent>
using TArrayView = std::span<T, N>;
#endif
template <class T>
using TVector = std::vector<T>;
template <class T>
using TDeque = std::deque<T>;
template <class T>
using TList = std::list<T>;
template <class T, class L = std::less<T>>
using TSet = std::set<T, L>;
template <class K, class T, class L = std::less<K>>
using TMultiSet = std::multiset<K, T, L>;
template <class K, class H = std::hash<K>, class E = std::equal_to<K>>
using THashSet = std::unordered_set<K, H, E>;
template <class K, class T, class L = std::less<K>>
using TMap = std::map<K, T, L>;
template <class K, class T, class L = std::less<K>>
using TMultiMap = std::multimap<K, T, L>;
template <class K, class T, class H = std::hash<K>, class E = std::equal_to<K>>
using THashMap = std::unordered_map<K, T, H, E>;
template <class T>
using TQueue = std::queue<T>;
template <class T, class C = TVector<T>, class L = std::less<typename C::value_type>>
using TPriorityQueue = std::priority_queue<T, C, L>;
template <class T>
using TStack = std::stack<T>;
template <size_t N>
using TBitset = std::bitset<N>;
template <class T, class U>
using TBinary = std::pair<T, U>;
template <class ...TS>
using TTuple = std::tuple<TS...>;
using TException = std::exception;
template <class T>
using TLambda = std::function<T>;
using TThread = std::thread;
template <class T>
using TFuture = std::future<T>;
template <class T>
using TPromise = std::promise<T>;
template <class T>
using TAtomic = std::atomic<T>;
using TMutex = std::recursive_mutex;
using TMutexLock = std::lock_guard<TMutex>;
using TUniqueLock = std::unique_lock<TMutex>;
using TMutexUnlock = std::condition_variable_any;
using TStringList = TVector<TString>;
using TWStringList = TVector<TWString>;
using TString8List = TVector<TString8>;
using TString16List = TVector<TString16>;
using TString32List = TVector<TString32>;
template<class T>
using TStringMap = TMap<TString, T>;
template<class T>
using TWStringMap = TMap<TWString, T>;
template<class T>
using TString8Map = TMap<TString8, T>;
template<class T>
using TString16Map = TMap<TString16, T>;
template<class T>
using TString32Map = TMap<TString32, T>;
template<class T>
using TStringHashMap = THashMap<TString, T>;
template<class T>
using TWStringHashMap = THashMap<TWString, T>;
template<class T>
using TString8HashMap = THashMap<TString8, T>;
template<class T>
using TString16HashMap = THashMap<TString16, T>;
template<class T>
using TString32HashMap = THashMap<TString32, T>;

// ============================================

template<typename T, typename ... Args>
inline TRef<T> TNew(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename U, typename T>
inline TRef<U> TCast(TRef<T>&& target)
{
	if (target == nullptr) return nullptr;
	return std::dynamic_pointer_cast<U>(target);
}
template<typename U, typename T>
inline TRef<U> TCast(TRef<T> const& target)
{
	if (target == nullptr) return nullptr;
	return std::dynamic_pointer_cast<U>(target);
}
template<typename U, typename T>
inline THnd<U> TCast(THnd<T>&& target)
{
	if (target == nullptr) return THnd<U>();
	return std::dynamic_pointer_cast<U>(target.lock());
}
template<typename U, typename T>
inline THnd<U> TCast(THnd<T> const& target)
{
	if (target == nullptr) return THnd<U>();
	return std::dynamic_pointer_cast<U>(target.lock());
}
template<typename U, typename T>
inline TRaw<U> TCast(TRaw<T>&& target)
{
	if (target == nullptr) return nullptr;
	return dynamic_cast<U*>((T*)target);
}
template<typename U, typename T>
inline TRaw<U> TCast(TRaw<T> const& target)
{
	if (target == nullptr) return nullptr;
	return const_cast<U*>(dynamic_cast<const U*>((const T*)target));
}
inline constexpr uint32_t THash(TCString value)
{
	uint32_t hash = 0; // From JDK 8
	if (value == nullptr) return hash;
	while (*value) hash = hash * 31 + (*value++);
	return hash;
}
inline constexpr uint32_t THash(TString const& value)
{
	return THash(value.c_str());
}
#if 20 <= OPENMS_CPP_VERSION
inline constexpr uint32_t THash(TStringView value)
{
	return THash(value.data());
}
#endif

// ============================================

template <class T, class U, std::enable_if_t<std::is_same_v<T, U>, int> = 0>
inline bool TTypeC(T const& value, U& result)
{
	result = value;
	return true;
}

template <class T, class U, std::enable_if_t<!std::is_same_v<T, U>, int> = 0>
inline bool TTypeC(T const& value, U& result)
{
	TString string;
	return TTypeC(value, string) && TTypeC(string, result);
}

template <class T>
struct TTextC
{
	static TString to_string(T const& value, TString const& string = TString())
	{
		TString result;
		if (TTypeC(value, result)) return result;
		return string;
	}
	static T from_string(TString const& string, T const& value = T())
	{
		T result;
		if (TTypeC(string, result)) return result;
		return value;
	}
};

template <>
inline bool TTypeC(bool const& value, TString& string)
{
	string = value ? "true" : "false";
	return true;
}
template <>
inline bool TTypeC(TString const& string, bool& value)
{
	value = (string == "true");
	return true;
}

template <>
inline bool TTypeC(int16_t const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, int16_t& value)
{
	value = (int16_t)std::strtol(string.c_str(), nullptr, 10);
	return true;
}

template <>
inline bool TTypeC(uint16_t const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, uint16_t& value)
{
	value = (uint16_t)std::strtol(string.c_str(), nullptr, 10);
	return true;
}

template <>
inline bool TTypeC(int32_t const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, int32_t& value)
{
	value = std::strtol(string.c_str(), nullptr, 10);
	return true;
}

template <>
inline bool TTypeC(uint32_t const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, uint32_t& value)
{
	value = std::strtoul(string.c_str(), nullptr, 10);
	return true;
}

template <>
inline bool TTypeC(float const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, float& value)
{
	value = std::strtof(string.c_str(), nullptr);
	return true;
}

template <>
inline bool TTypeC(double const& value, TString& string)
{
	string = std::to_string(value);
	return true;
}
template <>
inline bool TTypeC(TString const& string, double& value)
{
	value = std::strtod(string.c_str(), nullptr);
	return true;
}

#include <nlohmann/json.hpp>
#define OPENMS_TYPE NLOHMANN_DEFINE_TYPE_INTRUSIVE

template <class T, std::enable_if_t<!std::is_scalar_v<T> && !std::is_same_v<T, TString>, int> = 0>
inline bool TTypeC(T const& value, TString& result)
{
	nlohmann::json j = value;
	result = j.dump();
	return true;
}
template <class T, std::enable_if_t<!std::is_scalar_v<T> && !std::is_same_v<T, TString>, int> = 0>
inline bool TTypeC(TString const& value, T& result)
{
	nlohmann::json j = nlohmann::json::parse(value.c_str(), nullptr, false);
	result = j.get<T>();
	return true;
}

template<class T>
inline bool TTypeC(TVector<T> const& v, TString& s)
{
	nlohmann::json j;
	for (auto& e : v)
	{
		TString item; TTypeC(e, item);
		nlohmann::json jj = nlohmann::json::parse(item, nullptr, false);
		j.push_back(jj);
	}
	s = j.dump();
	return true;
}
template<class T>
inline bool TTypeC(TString const& s, TVector<T>& v)
{
	nlohmann::json j = nlohmann::json::parse(s, nullptr, false);
	if (j.is_array())
	{
		for (size_t i = 0; i < j.size(); ++i)
		{
			T item;
			auto& value = j[i];
			if (value.is_string()) TTypeC(value.get<std::string>(), item);
			else if (value.is_boolean()) TTypeC(value.get<bool>(), item);
			else if (value.is_number_float()) TTypeC(value.get<float>(), item);
			else if (value.is_number_integer()) TTypeC(value.get<int32_t>(), item);
			else if (value.is_number_unsigned()) TTypeC(value.get<uint32_t>(), item);
			else TTypeC(value.dump(), item);
			v.emplace_back(item);
		}
		return true;
	}
	return false;
}
template <class T, class U>
inline bool TTypeC(TVector<T> const& value, TVector<U>& result)
{
	result.clear();
	for (auto& e : value)
	{
		U item; TTypeC(e, item);
		result.emplace_back(item);
	}
	return true;
}

template<class T>
inline bool TTypeC(TSet<T> const& v, TString& s)
{
	nlohmann::json j;
	for (auto& e : v)
	{
		TString item; TTypeC(e, item);
		nlohmann::json jj = nlohmann::json::parse(item, nullptr, false);
		j.push_back(jj);
	}
	s = j.dump();
	return true;
}
template<class T>
inline bool TTypeC(TString const& s, TSet<T>& v)
{
	nlohmann::json j = nlohmann::json::parse(s, nullptr, false);
	if (j.is_array())
	{
		for (size_t i = 0; i < j.size(); ++i)
		{
			T item;
			auto& value = j[i];
			if (value.is_string()) TTypeC(value.get<std::string>(), item);
			else if (value.is_boolean()) TTypeC(value.get<bool>(), item);
			else if (value.is_number_float()) TTypeC(value.get<float>(), item);
			else if (value.is_number_integer()) TTypeC(value.get<int32_t>(), item);
			else if (value.is_number_unsigned()) TTypeC(value.get<uint32_t>(), item);
			else TTypeC(value.dump(), item);
			v.emplace(item);
		}
		return true;
	}
	return false;
}
template <class T, class U>
inline bool TTypeC(TSet<T> const& value, TSet<U>& result)
{
	result.clear();
	for (auto& e : value)
	{
		U item; TTypeC(e, item);
		result.emplace(item);
	}
	return true;
}

template<class K, class T>
inline bool TTypeC(TMap<K, T> const& v, TString& s)
{
	nlohmann::json j;
	for (auto& e : v)
	{
		TString key, item;
		TTypeC(e.first, key);
		TTypeC(e.second, item);
		nlohmann::json jj = nlohmann::json::parse(item, nullptr, false);
		j[key] = jj;
	}
	s = j.dump();
	return true;
}
template<class K, class T>
inline bool TTypeC(TString const& s, TMap<K, T>& v)
{
	nlohmann::json j = nlohmann::json::parse(s, nullptr, false);
	if (j.is_object())
	{
		for (auto& e : j.items())
		{
			K key; T item;
			TTypeC(e.key(), key);
			auto& value = e.value();
			if (value.is_string()) TTypeC(value.get<std::string>(), item);
			else if (value.is_boolean()) TTypeC(value.get<bool>(), item);
			else if (value.is_number_float()) TTypeC(value.get<float>(), item);
			else if (value.is_number_integer()) TTypeC(value.get<int32_t>(), item);
			else if (value.is_number_unsigned()) TTypeC(value.get<uint32_t>(), item);
			else TTypeC(value.dump(), item);
			v.emplace(key, item);
		}
		return true;
	}
	return false;
}
template <class K, class T, class K2, class T2>
inline bool TTypeC(TMap<K, T> const& value, TMap<K2, T2>& result)
{
	result.clear();
	for (auto& e : value)
	{
		K2 key; T2 item;
		TTypeC(e.first, key);
		TTypeC(e.second, item);
		result.emplace(key, item);
	}
	return true;
}

// ============================================

#define TAssert(...) assert(__VA_ARGS__)

#define TPrint(FORMAT, ...) do{ fprintf(stdout, "%s(%d)\n%.3f s\t[%s]\t" FORMAT "\n\n", __FILE__, __LINE__, ::clock()*0.001f, "INFO", __VA_ARGS__); }while(0)
#define TError(FORMAT, ...) do{ fprintf(stderr, "%s(%d)\n%.3f s\t[%s]\t" FORMAT "\n\n", __FILE__, __LINE__, ::clock()*0.001f, "ERROR", __VA_ARGS__); }while(0)
#define TFatal(FORMAT, ...) do{ fprintf(stderr, "%s(%d)\n%.3f s\t[%s]\t" FORMAT "\n\n", __FILE__, __LINE__, ::clock()*0.001f, "FATAL", __VA_ARGS__); exit(1); }while(0)
#ifdef OPENMS_DEBUG
#define TDebug(FORMAT, ...) do{ fprintf(stdout, "%s(%d)\n%.3f s\t[%s]\t" FORMAT "\n\n", __FILE__, __LINE__, ::clock()*0.001f, "DEBUG", __VA_ARGS__); }while(0)
#else													
#define TDebug(FORMAT, ...)
#endif