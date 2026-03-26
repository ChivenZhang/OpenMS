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
static constexpr auto OPENMS_FRAME_TIME = 1000 / 20;

int Server::startup()
{
	if (m_Looping == true) return 0;
	m_FrameCount = 0UL;
	m_FrameTime = ::clock();
	m_FrameNext = m_FrameTime;
	m_Looping = m_Running = true;

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
	MS_INFO("started {}", this->identity().c_str());

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

		m_FrameTime = ::clock();
		onUpdate((float)m_FrameTime * 0.001f);
		while (m_FrameNext < m_FrameTime)
		{
			onFrame(m_FrameCount++);
			m_FrameNext += OPENMS_FRAME_TIME;
		}
	}

	try
	{
		onExit();
	}
	catch (MSError& ex)
	{
		onError(std::forward<MSError>(ex));
	}
	catch (...)
	{
		onError(cpptrace::logic_error("unknown exception"));
	}
	m_Looping = m_Running = false;
	MS_INFO("terminated {}", this->identity().c_str());
	return 0;
}

int Server::looping()
{
	if (m_Looping == false)
	{
		m_FrameCount = 0UL;
		m_FrameTime = ::clock();
		m_FrameNext = m_FrameTime;
		m_Looping = m_Running = true;

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
		MS_INFO("started {}", this->identity().c_str());
	}
	else
	{
		if (m_Running)
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

			m_FrameTime = ::clock();
			onUpdate((float)m_FrameTime * 0.001f);
			while (m_FrameNext < m_FrameTime)
			{
				onFrame(m_FrameCount++);
				m_FrameNext += OPENMS_FRAME_TIME;
			}
		}
		else
		{
			try
			{
				onExit();
			}
			catch (MSError& ex)
			{
				onError(std::forward<MSError>(ex));
			}
			catch (...)
			{
				onError(cpptrace::logic_error("unknown exception"));
			}
			m_Looping = m_Running = false;
			MS_INFO("terminated {}", this->identity().c_str());
			return 0;
		}
	}
	return 1;
}

void Server::shutdown()
{
	if (m_Looping == true) m_Running = false;
}

MSString Server::identity() const
{
	return "service";
}

void Server::postEvent(MSLambda<void()>&& event)
{
	m_Working = true;
	MSMutexLock lock(m_Lock);
	m_Events.emplace(std::move(event));
}

MSString Server::property(MSString const& name) const
{
	auto source = AUTOWIRE_DATA(IProperty);
	if (source == nullptr) return {};
	return source->property(name);
}

IServer::timer_t Server::startTimer(uint64_t timeout, uint64_t repeat, MSLambda<void()>&& task)
{
	return m_Timer.start(timeout, repeat, [=, this]()
	{
		postEvent([=]() { task(); });
	});
}

void Server::stopTimer(timer_t handle)
{
	m_Timer.stop(handle);
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

#ifdef OPENMS_PLATFORM_WINDOWS
#include <client/windows/handler/exception_handler.h>
static bool MinidumpCallback(
	const wchar_t* dump_path,
	const wchar_t* id,
	void* context, EXCEPTION_POINTERS* exinfo,
	MDRawAssertionInfo* assertion,
	bool succeeded)
{
	return succeeded;
}
static google_breakpad::ExceptionHandler exceptionHandler(
		L".", nullptr, MinidumpCallback, nullptr,
		google_breakpad::ExceptionHandler::HANDLER_ALL);
#endif