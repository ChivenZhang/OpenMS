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
#include "Service.h"

int IApplication::Argc = 0;
char** IApplication::Argv = nullptr;

int Service::startup()
{
	m_Running = true;
	auto frame = 0UL;
	auto frameTime = ::clock();
	auto frameNext = frameTime;
	constexpr auto timePerFrame = 1000 / 15;

	try
	{
		onInit();
	}
	catch (MSError& ex)
	{
		onError(std::move(ex));
	}
	catch (...)
	{
		onError(std::exception("unknown exception"));
	}

	while (m_Running)
	{
		if (m_Working)
		{
			MSMutexLock lock(m_Mutex);
			m_Working = false;
			while (m_Running && m_Events.size())
			{
				auto event = m_Events.front();
				m_Events.pop();
				try
				{
					event();
				}
				catch (MSError ex)
				{
					onError(std::forward<MSError>(ex));
				}
				catch (...)
				{
					onError(std::exception("unknown exception"));
				}
			}
		}

		frameTime = ::clock();
		onUpdate(frameTime * 0.001f);
		while (frameNext < frameTime)
		{
			onFrame(frame++);
			frameNext += timePerFrame;
		}
	}

	try
	{
		onExit();
	}
	catch (MSError ex)
	{
		onError(std::forward<MSError>(ex));
	}
	catch (...)
	{
		onError(std::exception("unknown exception"));
	}

	return 0;
}

void Service::shutdown()
{
	m_Running = false;
	m_Unlock.notify_one();
}

uint32_t Service::startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)>&& task)
{
	return m_Timer.start(timeout, repeat, [=](uint32_t handle) {
		sendEvent([&]() { task(handle); });
		});
}

bool Service::stopTimer(uint32_t handle)
{
	return m_Timer.stop(handle);
}

void Service::sendEvent(MSLambda<void()>&& event)
{
	{
		MSMutexLock lock(m_Mutex);
		m_Working = true;
		m_Events.emplace(std::move(event));
	}
	m_Unlock.notify_one();
}

MSString Service::property(MSString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return MSString();
	return source->property(name);
}

void Service::onInit()
{
}

void Service::onExit()
{
}

void Service::onUpdate(float time)
{
}

void Service::onFrame(uint32_t frame)
{
}

void Service::onError(MSError&& error)
{
}
