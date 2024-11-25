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
#ifndef OPENMS_SHARED_LIBRARY
#include <OpenMS/Service/IService.h>
#include <csignal>

extern int openms_main(int argc, char** argv);

int main(int argc, char** argv)
{
	IService::argc = argc;
	IService::argv = argv;
	return openms_main(argc, argv);
}
#endif