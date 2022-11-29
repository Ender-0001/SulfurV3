#pragma once
#include "framework.h"

namespace Hooks
{
	static bool ReadyToStartMatchHook(__int64 a1)
	{
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		static bool bPreLoaded = false; 

		if (!bPreLoaded)
		{
			bPreLoaded = true;

			Game::PreLoad();
		}

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFortPlayerStartWarmup::StaticClass(), &Actors);

		if (Actors.IsEmpty())
			return false;

		static bool bLoadedPlaylist = false;

		if (!bLoadedPlaylist)
		{
			bLoadedPlaylist = true;
			GameState->OnRep_CurrentPlaylistInfo();
		}

		if (!GameState->MapInfo)
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

		return Native::HandleNewStartingPlayer(a1, PlayerController);
	}

	static char CanActivateAbilityHook()
	{
		return 1;
	}

	static __int64 GetNetModeHook(__int64)
	{
		return ((uintptr_t)_ReturnAddress() - (uintptr_t)GetModuleHandle(0)) == 0x15d0385 ? 2 : 1;
	}

	static char IsDedicatedServerHook()
	{
		return true;
	}

	static void Init()
	{
		CREATE_HOOK(ReadyToStartMatchHook, Native::ReadyToStartMatch);
		CREATE_HOOK(TickFlushHook, Native::TickFlush);
		CREATE_HOOK(KickPlayerHook, Native::KickPlayer);
		CREATE_HOOK(HandleNewStartingPlayerHook, Native::HandleNewStartingPlayer);
		CREATE_HOOK(CanActivateAbilityHook, Native::CanActivateAbility);
		CREATE_HOOK(GetNetModeHook, Native::GetNetMode_World);
		CREATE_HOOK(GetNetModeHook, Native::GetNetMode_Actor);
		CREATE_HOOK(IsDedicatedServerHook, Native::IsDedicatedServer);
	}
}