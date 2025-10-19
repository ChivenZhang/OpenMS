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
#include "Server.h"
#include <cpptrace/cpptrace.hpp>

int IStartup::Argc = 0;
char** IStartup::Argv = nullptr;

int Server::startup()
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
		m_Running = false;
	}
	catch (...)
	{
		onError(cpptrace::logic_error("unknown exception"));
		m_Running = false;
	}

	while (m_Running)
	{
		if (m_Working)
		{
			MSMutexLock lock(m_Lock);
			m_Working = false;
			while (m_Running && m_Events.empty() == false)
			{
				auto event = m_Events.front();
				m_Events.pop();
				try
				{
					event();
				}
				catch (MSError& ex)
				{
					onError(std::forward<MSError>(ex));
				}
				catch (...)
				{
					onError(cpptrace::logic_error("unknown exception"));
				}
			}
		}

		frameTime = ::clock();
		onUpdate((float)frameTime * 0.001f);
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
	catch (MSError& ex)
	{
		onError(std::forward<MSError>(ex));
		m_Running = false;
	}
	catch (...)
	{
		onError(cpptrace::logic_error("unknown exception"));
		m_Running = false;
	}

	return 0;
}

void Server::shutdown()
{
	m_Running = false;
}

MSString Server::identity() const
{
	return "service";
}

void Server::sendEvent(MSLambda<void()>&& event)
{
	MSMutexLock lock(m_Lock);
	m_Working = true;
	m_Events.emplace(std::move(event));
}

MSString Server::property(MSString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return {};
	return source->property(name);
}

uint32_t Server::startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void(uint32_t handle)>&& task)
{
	return m_Timer.start(timeout, repeat, [=, this](uint32_t handle)
	{
		sendEvent([&]() { task(handle); });
	});
}

bool Server::stopTimer(uint32_t handle)
{
	return m_Timer.stop(handle);
}

void Server::onInit()
{
}

void Server::onExit()
{
}

void Server::onUpdate(float time)
{
}

void Server::onFrame(uint32_t frame)
{
}

void Server::onError(MSError&& error)
{
}
