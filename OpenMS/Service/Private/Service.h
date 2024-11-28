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
	void sendEvent(TLambda<void()> && event) override final;
	uint32_t startTimer(uint64_t timeout, uint64_t repeat, TLambda<void(uint32_t handle)> && task) override final;
	bool stopTimer(uint32_t handle) override final;
	using IService::property;
	TString property(TString const& name) const override final;

protected:
	virtual void onInit();
	virtual void onExit();
	virtual void onUpdate(float time);
	virtual void onFrame(uint32_t frame);
	virtual void onError(TException && error);

protected:
	Timer m_Timer;
	TMutex m_Mutex;
	TMutexUnlock m_Unlock;
	TAtomic<bool> m_Running;
	TAtomic<bool> m_Working;
	TQueue<TLambda<void()>> m_Events;
};