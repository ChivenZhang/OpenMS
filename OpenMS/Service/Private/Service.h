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
#include "../IService.h"
#include "../IProperty.h"
#include "OpenMS/Toolkit/Timer.h"

#ifndef OPENMS_HEARTBEAT
#define OPENMS_HEARTBEAT 10 /*second*/
#endif

class Service
	:
	public IService,
	public AUTOWIRE(IProperty)
{
public:
	int startup() final;
	void shutdown() final;
	MSString identity() const override;
	void sendEvent(MSLambda<void()> && event) final;
	uint32_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)> && task) final;
	bool stopTimer(uint32_t handle) final;
	using IService::property;
	MSString property(MSString const& name) const final;

protected:
	virtual void onInit();
	virtual void onExit();
	virtual void onUpdate(float time);
	virtual void onFrame(uint32_t frame);
	virtual void onError(MSError && error);

protected:
	Timer m_Timer;
	MSMutex m_Lock;
	MSAtomic<bool> m_Running;
	MSAtomic<bool> m_Working;
	MSQueue<MSLambda<void()>> m_Events;
};