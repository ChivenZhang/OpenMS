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
#include <graphics.h>

#define MOVE_SIZE 10
#define MOVE_STEP 20

struct operate_t
{
	uint32_t Frame;
	uint32_t Type;
	uint32_t Actor;
};

struct status_t
{
	uint32_t Actor = 0;
	int32_t X = 100;
	int32_t Y = 100;
};

int openms_main(int argc, char** argv)
{
	initgraph(640, 480, INIT_DEFAULT);

	TVector<status_t> actorStatus;
	TQueue<operate_t> operateQueue;
	TMutex mutex; TMutexUnlock unlock;

	// Sync Time

	TPromise<void> promise;
	auto future = promise.get_future();
	time_t t1, t2, t3, t4;
	TCPClientReactor timeSync(IPv4Address::New("127.0.0.1", 9090), 0, TCPClientReactor::callback_tcp_t{
		[&](TRef<IChannel> channel) {
			channel->getPipeline()->addFirst("timesync", {
				.OnRead = [&](TRaw<IChannelContext> context, TRaw<IChannelEvent> event)->bool {
					time_t buffer[2];
					memcpy(buffer, event->Message.data(), sizeof(buffer));
					t2 = buffer[0];
					t3 = buffer[1];
					t4 = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
					promise.set_value();
					return false;
				}
				});
		}
		});
	timeSync.startup();
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	t1 = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
	timeSync.write(IChannelEvent::New("123"), nullptr);
	future.wait();
	timeSync.shutdown();

	auto serverTime = t1 + (t2 - t1 + t3 - t4) / 2;

	// Setup Client

	TCPClientReactor client(IPv4Address::New("127.0.0.1", 8080), 0, TCPClientReactor::callback_tcp_t{
		[&](TRef<IChannel> channel) {
			TString buffer;
			channel->getPipeline()->addFirst("broadcast", {
				.OnRead = [&, buffer](TRaw<IChannelContext> context, TRaw<IChannelEvent> event) mutable ->bool {
					buffer += event->Message;
					while (sizeof(operate_t) <= buffer.size())
					{
						operate_t* op = (operate_t*)buffer.data();
						{
							TMutexLock lock(mutex);
							operateQueue.push(*op);
						}
						buffer = buffer.substr(sizeof(operate_t));
					}
					return false;
				}
				});
		}
		});
	client.startup();

	srand(time(nullptr));
	auto actorID = uint32_t(1 + rand() % 10000);
	actorStatus.emplace_back(status_t{ actorID });

	bool pressedA = false, pressedD = false, pressedS = false, pressedW = false;

	auto update_func = [&](uint32_t frame) {
		{
			TMutexLock lock(mutex);
			while (operateQueue.size())
			{
				auto op = operateQueue.front();
				operateQueue.pop();

				auto actorID = op.Actor;
				auto result = std::find_if(actorStatus.begin(), actorStatus.end(), [actorID](const status_t& s) { return s.Actor == actorID; });
				if (result == actorStatus.end()) result = actorStatus.emplace(result, status_t{ actorID });
				auto& status = *result;

				switch (op.Type)
				{
				case 'A': status.X -= MOVE_STEP; break;
				case 'D': status.X += MOVE_STEP; break;
				case 'S': status.Y += MOVE_STEP; break;
				case 'W': status.Y -= MOVE_STEP; break;
				}
			}
		}

		auto& status = actorStatus[0];
		if (keystate(key_A))
		{
			operate_t op = { frame, 'A', status.Actor };
			auto event = IChannelEvent::New(TString((char*)&op, sizeof(op)));
			client.write(event, nullptr);
			status.X -= MOVE_STEP;

			pressedA = true;
		}
		else pressedA = false;

		if (keystate(key_D))
		{
			operate_t op = { frame, 'D', status.Actor };
			auto event = IChannelEvent::New(TString((char*)&op, sizeof(op)));
			client.write(event, nullptr);
			status.X += MOVE_STEP;

			pressedD = true;
		}
		else pressedD = false;

		if (keystate(key_S))
		{
			operate_t op = { frame, 'S', status.Actor };
			auto event = IChannelEvent::New(TString((char*)&op, sizeof(op)));
			client.write(event, nullptr);
			status.Y += MOVE_STEP;

			pressedS = true;
		}
		else pressedS = false;

		if (keystate(key_W))
		{
			operate_t op = { frame, 'W', status.Actor };
			auto event = IChannelEvent::New(TString((char*)&op, sizeof(op)));
			client.write(event, nullptr);
			status.Y -= MOVE_STEP;

			pressedW = true;
		}
		else pressedW = false;
		};

	auto render_func = [&](float time) {

		cleardevice();
		for (auto& status : actorStatus)
		{
			fillcircle(status.X, status.Y, MOVE_SIZE);
		}
		};

	uint32_t frame = 0;
	auto frameLength = 1000 / 20;
	auto timePerLoop = 1000 / 60;
	auto frameTime = ::clock();
	auto frameNext = frameTime + frameLength;
	while (is_run())
	{
		frameTime = ::clock();
		while (frameNext < frameTime)
		{
			update_func(frame++);
			frameNext += frameLength;
		}
		render_func(frameTime);
		delay(timePerLoop);
	}

	client.shutdown();
	closegraph();
	return 0;
}
