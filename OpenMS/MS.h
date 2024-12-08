#pragma once
/*=================================================
* Copyright © 2020-2024 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
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
#	define OPENMS_CPP_VERSION 0
#endif
#if OPENMS_CPP_VERSION < 17
#	error "At least c++ standard version 17"
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
#		define OPENMS_C_API extern "C"
#	endif
#endif

#ifdef _WIN32
#	ifdef _DEBUG
#		define OPENMS_DEBUG_MODE
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

#include <assert.h>
#define MSAssert(...) assert(__VA_ARGS__)

// ============================================

#include <cstdio>
#include <ctime>
#include <thread>
#define MS_FORMAT(TARGET, FORMAT, LEVEL, ...) \
do { \
char __DATETIME__[32]; auto __NOWTIME__ = std::time(nullptr); \
std::strftime(__DATETIME__, sizeof(__DATETIME__), "%Y-%m-%d %H:%M:%S", std::localtime(&__NOWTIME__)); \
auto __THREAD__ = []()->uint32_t { std::stringstream ss; ss << std::this_thread::get_id(); return std::stoul(ss.str()); }(); \
std::fprintf(TARGET, "%s:%d\n" "%s " #LEVEL " %d --- " FORMAT "\n\n", __FILE__, __LINE__, __DATETIME__, __THREAD__, ##__VA_ARGS__); \
} while (0)

#ifndef MS_DEBUG
#ifdef OPENMS_DEBUG_MODE
#	define MS_DEBUG(FORMAT, ...) MS_FORMAT(stdout, FORMAT, DEBUG, ##__VA_ARGS__)
#else
#	define MS_DEBUG(FORMAT, ...)
#endif
#endif

#ifndef MS_WARN
#	define MS_WARN(FORMAT, ...) MS_FORMAT(stdout, FORMAT, WARN, ##__VA_ARGS__)
#endif

#ifndef MS_INFO
#	define MS_INFO(FORMAT, ...) MS_FORMAT(stdout, FORMAT, INFO, ##__VA_ARGS__)
#endif

#ifndef MS_ERROR
#	define MS_ERROR(FORMAT, ...) MS_FORMAT(stderr, FORMAT, ERROR, ##__VA_ARGS__)
#endif

#ifndef MS_FATAL
#	define MS_FATAL(FORMAT, ...) do{ MS_FORMAT(stderr, FORMAT, FATAL, ##__VA_ARGS__); std::abort(); } while(0)
#endif

#ifndef MS_PRINT
#	define MS_PRINT(FORMAT, ...) MS_INFO(FORMAT, ##__VA_ARGS__)
#endif

// ============================================

template<class T>
using MSRaw = T*;
template<class T>
using MSRef = std::shared_ptr<T>;
template<class T>
using MSHnd = std::weak_ptr<T>;
using MSString = std::string;
using MSCString = const char*;
using MSWString = std::wstring;
#if 20 <= OPENMS_CPP_VERSION
using MSString8 = std::u8string;
#endif
using MSString16 = std::u16string;
using MSString32 = std::u32string;
#if 17 <= OPENMS_CPP_VERSION
using MSStringView = std::string_view;
using MSWStringView = std::wstring_view;
using MSString16View = std::u16string_view;
using MSString32View = std::u32string_view;
#endif
#if 20 <= OPENMS_CPP_VERSION
using MSString8View = std::u8string_view;
#endif
template <class T, size_t N>
using MSArray = std::array<T, N>;
#if 20 <= OPENMS_CPP_VERSION
template <class T, size_t N = std::dynamic_extent>
using MSArrayView = std::span<T, N>;
#endif
template <class T>
using MSList = std::vector<T>;
template <class T>
using MSDeque = std::deque<T>;
template <class T>
using MSLinkedList = std::list<T>;
template <class T, class L = std::less<T>>
using MSSet = std::set<T, L>;
template <class K, class T, class L = std::less<K>>
using MSMultiSet = std::multiset<K, T, L>;
template <class K, class H = std::hash<K>, class E = std::equal_to<K>>
using MSHashSet = std::unordered_set<K, H, E>;
template <class K, class T, class L = std::less<K>>
using MSMap = std::map<K, T, L>;
template <class K, class T, class L = std::less<K>>
using MSMultiMap = std::multimap<K, T, L>;
template <class K, class T, class H = std::hash<K>, class E = std::equal_to<K>>
using MSHashMap = std::unordered_map<K, T, H, E>;
template <class T>
using MSQueue = std::queue<T>;
template <class T, class C = MSList<T>, class L = std::less<typename C::value_type>>
using MSSortedQueue = std::priority_queue<T, C, L>;
template <class T>
using MSStack = std::stack<T>;
template <size_t N>
using MSBitset = std::bitset<N>;
template <class T, class U>
using MSBinary = std::pair<T, U>;
template <class ...TS>
using MSTuple = std::tuple<TS...>;
using MSAny = std::any;
using MSError = std::exception;
template <class T>
using MSLambda = std::function<T>;
using MSThread = std::thread;
template <class T>
using MSFuture = std::future<T>;
template <class T>
using MSPromise = std::promise<T>;
template <class T>
using MSAtomic = std::atomic<T>;
using MSMutex = std::recursive_mutex;
using MSMutexLock = std::lock_guard<MSMutex>;
using MSUniqueLock = std::unique_lock<MSMutex>;
using MSMutexUnlock = std::condition_variable_any;
using MSStringList = MSList<MSString>;
using MSWStringList = MSList<MSWString>;
#if 20 <= OPENMS_CPP_VERSION
using MSString8List = MSList<MSString8>;
#endif
using MSString16List = MSList<MSString16>;
using MSString32List = MSList<MSString32>;
template<class T>
using MSStringMap = MSMap<MSString, T>;
template<class T>
using MSWStringMap = MSMap<MSWString, T>;
#if 20 <= OPENMS_CPP_VERSION
template<class T>
using MSString8Map = MSMap<MSString8, T>;
#endif
template<class T>
using MSString16Map = MSMap<MSString16, T>;
template<class T>
using MSString32Map = MSMap<MSString32, T>;
template<class T>
using MSStringHashMap = MSHashMap<MSString, T>;
template<class T>
using MSWStringHashMap = MSHashMap<MSWString, T>;
#if 20 <= OPENMS_CPP_VERSION
template<class T>
using MSString8HashMap = MSHashMap<MSString8, T>;
#endif
template<class T>
using MSString16HashMap = MSHashMap<MSString16, T>;
template<class T>
using MSString32HashMap = MSHashMap<MSString32, T>;

// ============================================

template<typename T, typename ... Args>
inline MSRef<T> MSNew(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}
template<typename U, typename T>
inline MSRef<U> MSCast(MSRef<T>&& target)
{
	if (target == nullptr) return nullptr;
	return std::dynamic_pointer_cast<U>(target);
}
template<typename U, typename T>
inline MSRef<U> MSCast(MSRef<T> const& target)
{
	if (target == nullptr) return nullptr;
	return std::dynamic_pointer_cast<U>(target);
}
template<typename U, typename T>
inline MSHnd<U> MSCast(MSHnd<T>&& target)
{
	if (target == nullptr) return MSHnd<U>();
	return std::dynamic_pointer_cast<U>(target.lock());
}
template<typename U, typename T>
inline MSHnd<U> MSCast(MSHnd<T> const& target)
{
	if (target == nullptr) return MSHnd<U>();
	return std::dynamic_pointer_cast<U>(target.lock());
}
template<typename U, typename T>
inline MSRaw<U> MSCast(MSRaw<T>&& target)
{
	if (target == nullptr) return nullptr;
	return dynamic_cast<U*>((T*)target);
}
template<typename U, typename T>
inline MSRaw<U> MSCast(MSRaw<T> const& target)
{
	if (target == nullptr) return nullptr;
	return const_cast<U*>(dynamic_cast<const U*>((const T*)target));
}

inline constexpr uint32_t MSHash32(const char* const first, const size_t count) noexcept
{
	// These FNV-1a utility functions are extremely performance sensitive,
	// check examples like that in VSO-653642 before making changes.
	constexpr uint32_t _FNV_offset_basis = 2166136261U;
	constexpr uint32_t _FNV_prime = 16777619U;
	auto result = _FNV_offset_basis;
	// accumulate range [_First, _First + _Count) into partial FNV-1a hash _Val
	for (size_t i = 0; i < count; ++i)
	{
		result ^= (uint32_t)first[i];
		result *= _FNV_prime;
	}
	return result;
}
inline constexpr uint64_t MSHash64(const char* const first, const size_t count) noexcept
{
	// These FNV-1a utility functions are extremely performance sensitive,
	// check examples like that in VSO-653642 before making changes.
	constexpr uint64_t _FNV_offset_basis = 14695981039346656037ULL;
	constexpr uint64_t _FNV_prime = 1099511628211ULL;
	auto result = _FNV_offset_basis;
	// accumulate range [_First, _First + _Count) into partial FNV-1a hash _Val
	for (size_t i = 0; i < count; ++i)
	{
		result ^= (uint64_t)first[i];
		result *= _FNV_prime;
	}
	return result;
}
inline constexpr uint32_t MSHash(const char* const value) noexcept
{
	size_t count = 0; for (size_t i = 0; value[i]; ++i) ++count;
	return MSHash32(value, count);
}
inline const uint32_t MSHash(MSString const& value) noexcept
{
	return MSHash32(value.c_str(), value.size());
}
#if 17 <= OPENMS_CPP_VERSION
inline const uint32_t MSHash(MSStringView value) noexcept
{
	return MSHash32(value.data(), value.size());
}
#endif

// ============================================

#include <nlohmann/json.hpp>
#define OPENMS_TYPE NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT
#define OPENMS_IS_SCALAR(T) std::enable_if_t<std::is_scalar_v<T>, int> = 0
#define OPENMS_NOT_SCALAR(T) std::enable_if_t<!std::is_scalar_v<T>, int> = 0
#define OPENMS_BASE_OF(T, U) std::enable_if_t<std::is_base_of_v<T, U>, int> = 0
#define OPENMS_IS_SAME(T, U) std::enable_if_t<std::is_same_v<T, U>, int> = 0
#define OPENMS_NOT_SAME(T, U) std::enable_if_t<!std::is_same_v<T, U>, int> = 0
#define OPENMS_IS_TEXT(T) std::enable_if_t<std::is_same_v<T, std::string>, int> = 0
#define OPENMS_NOT_TEXT(T) std::enable_if_t<!std::is_same_v<T, std::string>, int> = 0

namespace std
{
	template <typename T>
	struct function_traits : public function_traits<decltype(&T::operator())> {};

	template <typename F, typename T, typename... Args>
	struct function_traits<T(F::*)(Args...) const>
	{
		using result_type = T;
		using argument_tuple = std::tuple<Args...>;
	};
}

template<class T, class U, OPENMS_IS_SAME(T, U)>
inline bool TTypeC(T const& src, U& dst)
{
	dst = src;
	return true;
}

template<class T, class U, OPENMS_NOT_SAME(T, U), OPENMS_IS_TEXT(T), OPENMS_NOT_SCALAR(T)>
bool TTypeC(T const& src, U& dst)
{
	nlohmann::json json = nlohmann::json::parse(src, nullptr, false, true);
	dst = json.get<U>();
	return true;
}

template<class T, class U, OPENMS_NOT_SAME(T, U), OPENMS_NOT_TEXT(T), OPENMS_IS_SCALAR(T)>
bool TTypeC(T const& src, U& dst)
{
	MSString str;
	return TTypeC(src, str) && TTypeC(str, dst);
}

template<class T, class U, OPENMS_NOT_SAME(T, U), OPENMS_NOT_TEXT(T), OPENMS_NOT_SCALAR(T)>
bool TTypeC(T const& src, U& dst)
{
	nlohmann::json json = src;
	return TTypeC(json.dump(), dst);
}

template <class T = bool, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(bool const& src, MSString& dst)
{
	dst = src ? "true" : "false";
	return true;
}
template <class T = MSString, class U = bool, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, bool& dst)
{
	dst = (src == "true");
	return true;
}

template <class T = int16_t, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(int16_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = int16_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, int16_t& dst)
{
	dst = (int16_t)std::strtol(src.c_str(), nullptr, 10);
	return true;
}

template <class T = MSString, class U = uint16_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(uint16_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = uint16_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, uint16_t& dst)
{
	dst = (uint16_t)std::strtol(src.c_str(), nullptr, 10);
	return true;
}

template <class T = int32_t, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(int32_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = int32_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, int32_t& dst)
{
	dst = std::strtol(src.c_str(), nullptr, 10);
	return true;
}

template<class T = uint32_t, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(uint32_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = uint32_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, uint32_t& dst)
{
	dst = std::strtoul(src.c_str(), nullptr, 10);
	return true;
}

template <class T = int64_t, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(int64_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = int64_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, int64_t& dst)
{
	dst = std::strtoll(src.c_str(), nullptr, 10);
	return true;
}

template<class T = uint64_t, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(uint64_t const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = uint64_t, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, uint64_t& dst)
{
	dst = std::strtoull(src.c_str(), nullptr, 10);
	return true;
}

template <class T = float, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(float const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = float, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, float& dst)
{
	dst = std::strtof(src.c_str(), nullptr);
	return true;
}

template <class T = double, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(double const& src, MSString& dst)
{
	dst = std::to_string(src);
	return true;
}
template <class T = MSString, class U = double, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, double& dst)
{
	dst = std::strtod(src.c_str(), nullptr);
	return true;
}

template <class T = const char*, class U = MSString, OPENMS_NOT_SAME(T, U)>
bool TTypeC(const char* const& src, MSString& dst)
{
	dst = (src) ? MSString(src) : MSString();
	return true;
}
template <class T = MSString, class U = const char*, OPENMS_NOT_SAME(T, U)>
bool TTypeC(MSString const& src, const char*& dst)
{
	dst = src.c_str();
	return true;
}

template <class T>
struct TTextC
{
	static MSString to_string(T const& value, MSString const& string = MSString())
	{
		MSString result;
		if (TTypeC(value, result)) return result;
		return string;
	}
	static T from_string(MSString const& string, T const& value = T())
	{
		T result;
		if (TTypeC(string, result)) return result;
		return value;
	}
};