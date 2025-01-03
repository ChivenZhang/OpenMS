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
#include <OpenMS/MS.h>
#include "Message.pb.h"

#define OPENMS_AES256_IV "y8WOkCzXZHmRLMq6"
#define OPENMS_AES256_KEY "6BGtsnEW9s2QJalfTHFcXUi46JgJmmDe"

struct FramePackage
{
	uint32_t Size;
	MSString Data;
};