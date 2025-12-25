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
#include <tuple>
#include <functional>

template<class T, class... Args>
struct TTraitsBase
{
	using return_type = T;
	using return_data = std::remove_cvref_t<T>;
	using argument_types = std::tuple<Args...>;
	using argument_datas = std::tuple<std::remove_cvref_t<Args>...>;
	static constexpr std::size_t argument_count = sizeof...(Args);
	template<std::size_t N>
	using argument_type = std::tuple_element<N, std::tuple<Args...>>::type;
	template<std::size_t N>
	using argument_data = std::remove_cvref_t<typename std::tuple_element<N, std::tuple<Args...>>::type>;
};

template<class T>
struct TTraits;

template<class F>
struct TTraits : TTraits<decltype(&F::operator())> {};

template<class T, class... Args>
struct TTraits<T(Args...)> : TTraitsBase<T, Args...> {};

template<class T, class... Args>
struct TTraits<T(*)(Args...)> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...)> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const&> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const&&> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const noexcept> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const& noexcept> : TTraitsBase<T, Args...> {};

template<class C, class T, class... Args>
struct TTraits<T(C::*)(Args...) const&& noexcept> : TTraitsBase<T, Args...> {};

template<class T, class... Args>
struct TTraits<std::function<T(Args...)>> : TTraitsBase<T, Args...> {};

template<class T>
struct TFirstType;

template<>
struct TFirstType<std::tuple<>>
{
	using first_type = void;
};

template<class... Args>
struct TFirstType<std::tuple<Args...>>
{
	using first_type = std::tuple_element<0, std::tuple<Args...>>::type;
};

template<class T>
struct TSecondTypes;

template<class T, class... Args>
struct TSecondTypes<std::tuple<T, Args...>>
{
	using second_types = std::tuple<Args...>;
	using second_datas = std::tuple<std::remove_cvref_t<Args>...>;
	static constexpr std::size_t second_count = sizeof...(Args);
	template<std::size_t N>
	using second_type = std::tuple_element<N, std::tuple<Args...>>::type;
};