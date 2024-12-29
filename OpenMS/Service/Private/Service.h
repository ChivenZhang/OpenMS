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
#include "../IService.h"
#include "../IProperty.h"
#include "OpenMS/Toolkit/Timer.h"

class Service :
	public IService,
	public AUTOWIRE(IProperty)
{
public:
	int startup() override final;
	void shutdown() override final;
	MSString identity() const override;
	void sendEvent(MSLambda<void()> && event) override final;
	uint32_t startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)> && task) override final;
	bool stopTimer(uint32_t handle) override final;
	using IService::property;
	MSString property(MSString const& name) const override final;

protected:
	virtual void onInit();
	virtual void onExit();
	virtual void onUpdate(float time);
	virtual void onFrame(uint32_t frame);
	virtual void onError(MSError && error);

protected:
	Timer m_Timer;
	MSMutex m_Mutex;
	MSMutexUnlock m_Unlock;
	MSAtomic<bool> m_Running;
	MSAtomic<bool> m_Working;
	MSQueue<MSLambda<void()>> m_Events;
};