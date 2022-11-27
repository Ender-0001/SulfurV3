#pragma once
#include "framework.h"

namespace Hooks
{
	static bool ReadyToStartMatchHook(__int64 a1)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPlayerStartWarmup::StaticClass(), &Actors);

		if (Actors.IsEmpty())
			return false;

		Game::Start();
		Game::Listen();

		return Native::ReadyToStartMatch(a1);
	}

	static void TickFlushHook(UNetDriver* NetDriver)
	{
		if (NetDriver->ReplicationDriver)
			Native::ServerReplicateActors(NetDriver->ReplicationDriver);

		Native::TickFlush(NetDriver);
	}

	static void KickPlayerHook(void*, void*)
	{

	}

	static void HandleNewStartingPlayerHook(__int64 a1, APlayerController* PlayerController)
	{
		if (auto FortPlayerController = Cast<AFortPlayerControllerAthena>(PlayerController))
			Game::HandlePlayer(FortPlayerController);

		Native::HandleNewStartingPlayer(a1, PlayerController);
	}

	static void Init()
	{
		CREATE_HOOK(ReadyToStartMatchHook, Native::ReadyToStartMatch);
		CREATE_HOOK(TickFlushHook, Native::TickFlush);
		CREATE_HOOK(KickPlayerHook, Native::KickPlayer);
		CREATE_HOOK(HandleNewStartingPlayerHook, Native::HandleNewStartingPlayer);
	}
}