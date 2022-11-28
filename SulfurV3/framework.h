#pragma once
#include <Windows.h>
#include <string>
#include <locale>
#include <iostream>
#include <detours.h>
#include <vector>
#include <format>
#include <random>
#include <fstream>
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

#include "SDK.hpp"
using namespace SDK;

UFortEngine* GEngine = decltype(GEngine)(*(uintptr_t*)((uintptr_t)GetModuleHandle(0) + 0x65A40A0));

#define ANY_PACKAGE (UObject*)-1
#define CREATE_HOOK(Detour, Original) DetourTransactionBegin(); DetourUpdateThread(GetCurrentThread()); DetourAttach(&(void*&)Original, Detour); DetourTransactionCommit();

#ifdef DEVELOPER
#define UE_LOG(log, msg, ...) std::cout << #log << ": " << std::format(msg,  ##__VA_ARGS__) << '\n';
#else
#define UE_LOG(log, msg, ...)
#endif

static __forceinline UWorld* GetWorld()
{
	return GEngine->GameViewport->World;
}

template <typename T>
static __forceinline T* Cast(UObject* Object)
{
	if (Object && Object->IsA(T::StaticClass()))
	{
		return (T*)Object;
	}

	return nullptr;
}

#include "Native.h"
#include "Util.h"
#include "Inventory.h"
//#include "Looting.h"
#include "Game.h"
#include "Hooks.h"