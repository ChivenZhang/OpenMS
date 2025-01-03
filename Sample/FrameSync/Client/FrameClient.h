#pragma once
/*=================================================
* Copyright © 2020-2025 ChivenZhang.
* All Rights Reserved.
* =====================Note=========================
*
*
* ====================History=======================
* Created by ChivenZhang@gmail.com.
*
* =================================================*/
#include <OpenMS/Endpoint/TCP/TCPClient.h>
#include <OpenMS/Service/IService.h>

class FrameClient :
	public TCPClient,
	public AUTOWIRE(IService)
{
public:
	void configureEndpoint(config_t& config) const override;
};