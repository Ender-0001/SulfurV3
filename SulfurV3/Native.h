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

	static void (*RestartPlayerAtPlayerStart)(AGameMode*, APlayerController*, AActor*) =
		decltype(RestartPlayerAtPlayerStart)((uintptr_t)GetModuleHandle(0) + 0x30BAAE0);

	static FGameplayAbilitySpecHandle* (*GiveAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, FGameplayAbilitySpec) =
		decltype(GiveAbility)((uintptr_t)GetModuleHandle(0) + 0x935010);

	static __int64 (*CantBuild)(UWorld*, UClass*, FVector, FRotator, char, void*, char*) =
		decltype(CantBuild)((uintptr_t)GetModuleHandle(0) + 0x1601820);

	static void (*ServerLoadingScreenDropped)(AFortPlayerControllerAthena*);

	static void (*Array_Remove)(void* Array, UProperty*, int) =
		decltype(Array_Remove)((uintptr_t)GetModuleHandle(0) + 0x312C110);

	static __int64 (*OnSafeZoneStateChange)(AFortSafeZoneIndicator* SafeZoneIndicator, char a2, char a3) =
		decltype(OnSafeZoneStateChange)((uintptr_t)GetModuleHandle(0) + 0x1F83F10);

	static char (*CanActivateAbility)(__int64 a1, unsigned int a2, int* a3, __int64 a4, __int64 a5, void* a6) =
		decltype(CanActivateAbility)((uintptr_t)GetModuleHandle(0) + 0x9214C0);

	static ABuildingSMActor* (*BuildingSMActorReplaceBuildingActor)(ABuildingSMActor*, __int64, UClass*, int, int, uint8_t, AFortPlayerController*) =
		decltype(BuildingSMActorReplaceBuildingActor)((uintptr_t)GetModuleHandle(0) + 0x13D0DE0);

	static void (*GetNetMode_World)(UWorld*) =
		decltype(GetNetMode_World)((uintptr_t)GetModuleHandle(0) + 0x34D2140);

	static void (*GetNetMode_Actor)(AActor*) =
		decltype(GetNetMode_Actor)((uintptr_t)GetModuleHandle(0) + 0x2E3E970);

	static char (*IsDedicatedServer)() = 
		decltype(IsDedicatedServer)((uintptr_t)GetModuleHandle(0) + 0x3132CE0);

	static void (*ServerAttemptInteract)(UFortControllerComponent_Interaction* InteractionComp, AActor* ReceivingActor, __int64* a3, __int64 a4, unsigned int a5, __int64 a6) =
		decltype(ServerAttemptInteract)((uintptr_t)GetModuleHandle(0) + 0x1495B20);

	static void (*OnDamageServer)(ABuildingActor* BuildingActor, float Damage, FGameplayTagContainer DamageTags, FVector Momentum, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser, FGameplayEffectContextHandle EffectContext) =
		decltype(OnDamageServer)((uintptr_t)GetModuleHandle(0) + 0x1CC36A0);
 }