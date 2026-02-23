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
#include "../IEndpoint.h"

class WSServer : public IEndpoint
{
public:
	struct config_t
	{
		MSString IP;
		uint16_t PortNum;
	};

public:
	explicit WSServer(config_t const& config);
	void startup() override;
	void shutdown() override;
	bool running() const override;
	bool connect() const override;
	MSHnd<IChannelAddress> address() const override;

	void bind_connect(MSLambda<void(MSStringView address)> callback);
	void bind_disconnect(MSLambda<void(MSStringView address)> callback);
	void bind_binary(MSLambda<void(MSStringView binary)> callback);
	void bind_message(MSLambda<void(MSStringView message)> callback);
	void bind_binary(MSLambda<void(MSStringView frame, bool last)> callback);
	void bind_message(MSLambda<void(MSStringView frame, bool last)> callback);

protected:
	const config_t m_Config;
	MSThread m_EventThread;
	MSAtomic<bool> m_Running;
	MSAtomic<bool> m_Connect;
	MSLambda<void(MSStringView address)> m_ConnectCallback;
	MSLambda<void(MSStringView address)> m_DisconnectCallback;
	MSLambda<void(MSStringView binary)> m_BinaryCallback;
	MSLambda<void(MSStringView message)> m_MessageCallback;
	MSLambda<void(MSStringView frame, bool last)> m_BinaryFrameCallback;
	MSLambda<void(MSStringView frame, bool last)> m_MessageFrameCallback;
};
