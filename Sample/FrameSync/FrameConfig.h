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
#include "Server/FrameServer.h"
#include "Client/FrameClient.h"

#define OPENMS_AES256_IV "y8WOkCzXZHmRLMq6"
#define OPENMS_AES256_KEY "6BGtsnEW9s2QJalfTHFcXUi46JgJmmDe"

struct FramePackage
{
	uint32_t Size;
	TString Data;
};

class FrameConfig1
	:
	public RESOURCE(FrameServer)
{
};

class FrameConfig2
	:
	public RESOURCE(FrameClient)
{
};