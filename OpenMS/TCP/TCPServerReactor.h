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
#include "../Private/ChannelReactor.h"
#include "../Private/ChannelAddress.h"
#include <uv.h>

class TCPServerReactor : public ChannelReactor
{
public:
	TCPServerReactor(TStringView ip, uint16_t port, uint32_t backlog, size_t workerNum, callback_t callback);
	void startup() override;
	void shutdown() override;

protected:
	void onConnect(TRef<Channel> channel) override;
	void onDisconnect(TRef<Channel> channel) override;

protected:
	static void on_connect(uv_stream_t* server, int status);
	static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
	static void on_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
	static void on_stop(uv_async_t* handle);

protected:
	TString m_Address;
	uint16_t m_PortNum;
	uint32_t m_Backlog;
	uv_async_t m_AsyncStop;
	TMap<uint32_t, TRef<Channel>> m_Connections;
};