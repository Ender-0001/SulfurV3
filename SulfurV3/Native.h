#pragma once
#include "framework.h"

namespace Native
{
	static bool (*ReadyToStartMatch)(__int64) =
		decltype(ReadyToStartMatch)((uintptr_t)GetModuleHandle(0) + 0x353F830);

	static UNetDriver* (*CreateNetDriver)(UEngine*, UWorld*, FName) = 
		decltype(CreateNetDriver)((uintptr_t)GetModuleHandle(0) + 0x347FAF0);

	static char (*InitListen)(UNetDriver*, void*, FURL&, bool, FString&) = 
		decltype(InitListen)((uintptr_t)GetModuleHandle(0) + 0x6F5F90);

	static void (*SetWorld)(UNetDriver*, UWorld*) = 
		decltype(SetWorld)((uintptr_t)GetModuleHandle(0) + 0x31EDF40);

	static void (*TickFlush)(UNetDriver*) = 
		decltype(TickFlush)((uintptr_t)GetModuleHandle(0) + 0x31EECB0);

	static void (*ServerReplicateActors)(UReplicationDriver*) = 
		decltype(ServerReplicateActors)((uintptr_t)GetModuleHandle(0) + 0xA33E90);

	static bool (*InternalTryActivateAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle Handle, FPredictionKey InPredictionKey, UGameplayAbility** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData) = 
		decltype(InternalTryActivateAbility)((uintptr_t)GetModuleHandle(0) + 0x9367F0);
	
	static void (*ClientOnPawnDied)(AFortPlayerControllerAthena*, FFortPlayerDeathReport) =
		decltype(ClientOnPawnDied)((uintptr_t)GetModuleHandle(0) + 0x1F34E50);

	static void (*KickPlayer)(void*, void*) =
		decltype(KickPlayer)((uintptr_t)GetModuleHandle(0) + 0x17F07B0);

	static void (*HandleNewStartingPlayer)(__int64, APlayerController*) =
		decltype(HandleNewStartingPlayer)((uintptr_t)GetModuleHandle(0) + 0x3543090);

	static void (*RestartPlayerAtPlayerStart)(AGameMode*, APlayerController*, __int64) =
		decltype(RestartPlayerAtPlayerStart)((uintptr_t)GetModuleHandle(0) + 0x30BAAE0);

	static void (*GrantAbilities)(AFortPlayerPawnAthena*) =
		decltype(GrantAbilities)((uintptr_t)GetModuleHandle(0) + 0x192D570);

}