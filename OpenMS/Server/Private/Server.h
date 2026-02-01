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
#include "Server/IServer.h"
#include "Server/IProperty.h"
#ifndef OPENMS_HEARTBEAT
#define OPENMS_HEARTBEAT 10 /*second*/
#endif

class Server : public IServer, public AUTOWIRE(IProperty)
{
public:
	int startup() final;
	void shutdown() final;
	MSString identity() const override;
	void postEvent(MSLambda<void()>&& event) final;
	using IServer::property;
	MSString property(MSString const& name) const final;
	timer_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void()>&& task) final;
	void stopTimer(timer_t handle) final;

protected:
	virtual void onInit();
	virtual void onExit();
	virtual void onUpdate(float time);
	virtual void onFrame(uint32_t frame);
	virtual void onError(MSError&& error);

protected:
	TimerUtility m_Timer;
	MSMutex m_Lock;
	MSAtomic<bool> m_Running;
	MSAtomic<bool> m_Working;
	MSQueue<MSLambda<void()>> m_Events;
};