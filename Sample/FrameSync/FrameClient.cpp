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
#include <OpenMS/Service/IStartup.h>
#include <OpenMS/Reactor/TCP/TCPClientReactor.h>
#include <Windows.h>


struct operate_t
{
	uint32_t Frame;
	uint8_t Type;
};

auto IsPressed = [](int key) {
	auto state = GetAsyncKeyState(key);
	return !(state & 0x10) && state & 0x01;
	};

int openms_main(int argc, char** argv)
{
	TQueue<operate_t> operateQueue;
	TMutex mutex; TMutexUnlock unlock;

	TCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 0, TCPClientReactor::callback_tcp_t{
		[&](TRef<IChannel> channel) {
			TString buffer;
			channel->getPipeline()->addFirst("broadcast", {
				.OnRead = [&, buffer](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable ->bool {
					/*buffer += event->Message;
					if (sizeof(operate_t) <= buffer.size())
					{
						operate_t* op = (operate_t*)buffer.data();
						{
							TMutexLock lock(mutex);
							operateQueue.push(*op);
						}
						buffer = buffer.substr(sizeof(operate_t));
					}*/
					return false;
				}
				});
		},
		});
	client.startup();

	bool pressedA = false, pressedD = false, pressedS = false, pressedW = false;
	while (true)
	{
		{
			TMutexLock lock(mutex);
			while (operateQueue.size())
			{
				TPrint("frame %d, type: %d", operateQueue.front().Frame, operateQueue.front().Type);
				operateQueue.pop();
			}
		}

		if (IsPressed('A')) pressedA = true;
		else
		{
			if (pressedA)
			{
				auto event = IChannelEvent::New("move left");
				client.write(event, nullptr);
			}
			pressedA = false;
		}
		if (IsPressed('D')) pressedD = true;
		{
			if (pressedD)
			{
				auto event = IChannelEvent::New("move right");
				client.write(event, nullptr);
			}
			pressedD = false;
		}
		if (IsPressed('S')) pressedS = true;
		else
		{
			if (pressedS)
			{
				auto event = IChannelEvent::New("move down");
				client.write(event, nullptr);
			}
			pressedS = false;
		}
		if (IsPressed('W')) pressedW = true;
		else
		{
			if (pressedW)
			{
				auto event = IChannelEvent::New("move up");
				client.write(event, nullptr);
			}
			pressedW = false;
		}
	}

	client.shutdown();
	return 0;
}
