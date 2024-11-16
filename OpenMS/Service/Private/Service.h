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
#include "../IService.h"
#include "OpenMS/Channel/TCP/TCPServerReactor.h"

class Service : public IService
{
public:
	void startup(int argc, char** argv) override;
	void shutdown() override;
	bool contains(TStringView name) const;
	TString property(TStringView name) const;

	template <class T>
	T property(TStringView name, T const& value = T()) const
	{
		return TTextC<T>::from_string(property(name), value);
	}

protected:
	TRef<TCPServerReactor> m_Reactor;
	TMap<uint32_t, TString> m_PropertyMap;
};