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
#include <chrono>
#include <queue>

class QuickQPS
{
	std::queue<std::chrono::steady_clock::time_point> q;
	int windowSec;

public:
	QuickQPS(int sec = 5) : windowSec(sec) {}

	void hit()
	{
		auto now = std::chrono::steady_clock::now();
		q.push(now);

		// 清理过期记录
		auto cutoff = now - std::chrono::seconds(windowSec);
		while (!q.empty() && q.front() < cutoff) q.pop();
	}

	double get()
	{
		return static_cast<double>(q.size()) / windowSec;
	}
};