#pragma once
#include "framework.h"

namespace Game
{
	static void Start()
	{
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		GameState->CurrentPlaylistInfo.BasePlaylist = UObject::FindObject<UFortPlaylistAthena>("Playlist_DefaultSolo", ANY_PACKAGE);

		GameState->OnRep_CurrentPlaylistInfo();
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameMode->GameSession->MaxPlayers = 100;
		GameMode->bWorldIsReady = true;

		GameMode->StartMatch();
		GameMode->StartPlay();
	}

	static void Listen()
	{
		GetWorld()->NetDriver = Native::CreateNetDriver(GEngine, GetWorld(), UKismetStringLibrary::Conv_StringToName(L"GameNetDriver"));

		FString Error;
		auto URL = FURL();
		URL.Port = 7777;

		GetWorld()->NetDriver->World = GetWorld();
		GetWorld()->NetDriver->NetDriverName = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

		Native::InitListen(GetWorld()->NetDriver, GetWorld(), URL, true, Error);
		Native::SetWorld(GetWorld()->NetDriver, GetWorld());

		// Native::ServerReplicateActors = decltype(Native::ServerReplicateActors)(GetWorld()->NetDriver->ReplicationDriver->VFT[0x56]);

		GetWorld()->NetDriver = GetWorld()->NetDriver;
		GetWorld()->LevelCollections[0].NetDriver = GetWorld()->NetDriver;
		GetWorld()->LevelCollections[1].NetDriver = GetWorld()->NetDriver;
	}

	static void HandlePlayer(AFortPlayerControllerAthena* PlayerController)
	{
		auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		PlayerController->OverriddenBackpackSize = 5;

		PlayerController->bHasClientFinishedLoading = true;
		PlayerController->bHasServerFinishedLoading = true;
		PlayerController->OnRep_bHasServerFinishedLoading();

		PlayerState->bHasFinishedLoading = true;
		PlayerState->bHasStartedPlaying = true;
		PlayerState->OnRep_bHasStartedPlaying();
	}

	static void ServerAcknowledgePossessionHook(APlayerController* PlayerController, APawn* P)
	{
		PlayerController->AcknowledgedPawn = P;
	}

	static void ServerExecuteInventoryItemHook(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
	{
		if (PlayerController->IsInAircraft())
			return;


	}

	static void ClientOnPawnDiedHook(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport DeathReport)
	{
		auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
		auto KillerPawn = Cast<AFortPlayerPawnAthena>(DeathReport.KillerPawn);
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);
		auto KillerPlayerState = Cast<AFortPlayerStateAthena>(DeathReport.KillerPlayerState);
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		if (GameState->GamePhase == EAthenaGamePhase::Warmup && GameState->GamePhase == EAthenaGamePhase::Aircraft)
			return Native::ClientOnPawnDied(PlayerController, DeathReport);

		if (PlayerController && PlayerState)
		{
			FDeathInfo DeathInfo;
			DeathInfo.bDBNO = false;
			DeathInfo.DeathLocation = PlayerController->Pawn ? PlayerController->Pawn->K2_GetActorLocation() : FVector();

			DeathInfo.DeathCause = AFortPlayerStateAthena::ToDeathCause(DeathReport.Tags, false);
			DeathInfo.FinisherOrDowner = KillerPlayerState ? KillerPlayerState : PlayerState;

			PlayerState->DeathInfo = DeathInfo;
			PlayerState->OnRep_DeathInfo();

			if (KillerPlayerState && KillerPlayerState != PlayerState)
			{
				KillerPlayerState->KillScore++;
				KillerPlayerState->TeamKillScore++;

				KillerPlayerState->ClientReportKill(PlayerState);
				KillerPlayerState->OnRep_Kills();
			}

			/*auto DroppableItems = Inventory::GetDroppableItems(PlayerController);

			for (int i = 0; i < DroppableItems.size(); i++)
			{
				auto Item = DroppableItems[i];

				if (!Item)
					continue;

				if (Pawn)
				{
					auto ItemEntry = Item->ItemEntry;
					for (int i = 0; i < Pawn->CurrentWeaponList.Num(); i++)
					{
						auto Weapon = Pawn->CurrentWeaponList[i];

						if (Weapon && Weapon->ItemEntryGuid == ItemEntry.ItemGuid)
						{
							ItemEntry.LoadedAmmo = Weapon->AmmoCount;
							break;
						}
					}
				}

				//Looting::SpawnPickup(Item->ItemEntry, DeathInfo.DeathLocation, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
			}*/

			PlayerController->bMarkedAlive = false;

			auto Winners = Util::GetAlivePlayers();

			for (int i = 0; i < GameMode->AlivePlayers.Num(); i++)
			{
				auto AlivePlayer = GameMode->AlivePlayers[i];

				if (AlivePlayer == PlayerController)
				{
					GameMode->AlivePlayers.Remove(i);
					break;
				}
			}

			GameState->PlayersLeft = Winners.size();
			GameState->TotalPlayers = Winners.size();
			GameState->OnRep_PlayersLeft();

			if (Util::OnSameTeam(Winners) && Winners.size() != 0)
			{
				for (auto Winner : Winners)
				{
					if (Winner)
						GameState->WinningPlayerList.Add(FFortWinnerPlayerData{ Winner->PlayerID });
				}

				GameState->OnRep_WinningPlayerList();

				GameState->WinningPlayerState = Winners[0];
				GameState->OnRep_WinningPlayerState();

				for (auto Winner : Winners)
				{
					if (auto PlayerController = Cast<AFortPlayerControllerAthena>(Winner->Owner))
					{ 
						AActor* DamageCauser = DeathReport.DamageCauser;
						UFortWeaponItemDefinition* KillerWeaponDef = nullptr;

						if (auto ProjectileBase = Cast<AFortProjectileBase>(DamageCauser))
							KillerWeaponDef = ((AFortWeapon*)ProjectileBase->GetOwner())->WeaponData;
						if (auto Weapon = Cast<AFortWeapon>(DamageCauser))
							KillerWeaponDef = Weapon->WeaponData;

						PlayerController->PlayWinEffects(DeathReport.KillerPawn, KillerWeaponDef, DeathInfo.DeathCause, false);
						PlayerController->ClientNotifyWon(DeathReport.KillerPawn, KillerWeaponDef, DeathInfo.DeathCause);
						PlayerController->ClientGameEnded(Winner, true);
						GameMode->ReadyToEndMatch();
						GameMode->EndMatch();
						GameMode->EndGamePhaseStarted();
					}
				}
			}
		}

		Native::ClientOnPawnDied(PlayerController, DeathReport);
	}

	static void ServerHandlePickupHook(AFortPlayerPawnAthena* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
	{
		if (Pickup->bPickedUp)
			return;

		AFortPlayerControllerAthena* PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

		if (PlayerController->IsInAircraft())
			return;

		UFortItemDefinition* ItemDefinition = Pickup->PrimaryPickupItemEntry.ItemDefinition;
		int Count = Pickup->PrimaryPickupItemEntry.Count;

		FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;

		TArray<UFortWorldItem*>& ItemInstances = Inventory.ItemInstances;
		TArray<FFortItemEntry>& ReplicatedEntries = Inventory.ReplicatedEntries;

		UFortWorldItem* FortItem = (UFortWorldItem*)ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1);

		ItemInstances.Add(FortItem);
		ReplicatedEntries.Add(FortItem->ItemEntry);

		Inventory.MarkArrayDirty();

		Pickup->PickupLocationData.PickupTarget = Pawn;
		Pickup->PickupLocationData.FlyTime = 0.40f;
		Pickup->PickupLocationData.ItemOwner = Pawn;
		Pickup->OnRep_PickupLocationData();

		Pickup->bPickedUp = true;
		Pickup->OnRep_bPickedUp();
	}

	static void ServerAttemptInventoryDropHook(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, int Count)
	{
		if (!PlayerController->MyFortPawn || PlayerController->IsInAircraft())
			return;

		FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;

		TArray<UFortWorldItem*>& ItemInstances = Inventory.ItemInstances;
		TArray<FFortItemEntry>& ReplicatedEntries = Inventory.ReplicatedEntries;

		bool bWasFoundInItemInstances = false;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances[i];

			if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
			{
				if (ItemInstance->ItemEntry.Count - Count < 0)
					return;

				//SummonPickup(ItemInstance->ItemEntry.ItemDefinition, Count, PlayerController->MyFortPawn->K2_GetActorLocation());

				ItemInstance->ItemEntry.Count -= Count;

				if (ItemInstance->ItemEntry.Count == 0)
				{
					ItemInstances.Remove(i);
				}

				bWasFoundInItemInstances = true;

				break;
			}
		}

		if (!bWasFoundInItemInstances)
			return;

		for (int i = 0; i < ReplicatedEntries.Num(); i++)
		{
			FFortItemEntry ReplicatedEntry = ReplicatedEntries[i];

			if (ReplicatedEntry.ItemGuid == ItemGuid)
			{				
				ReplicatedEntry.Count -= Count;

				if (ReplicatedEntry.Count == 0)
				{
					ReplicatedEntries.Remove(i);
					Inventory.MarkArrayDirty();
				}
				else
				{
					Inventory.MarkItemDirty(ReplicatedEntry);
				}

				break;
			}
		}
	}

	static void ServerTryActivateAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey)
	{
		Util::ActivateAbility(AbilitySystemComponent, Handle, PredictionKey);
	}

	static void ServerTryActivateAbilityWithEventDataHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData)
	{
		Util::ActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &TriggerEventData);
	}

	static void RestartPlayerAtPlayerStartHook(AGameMode* GameMode, APlayerController* PlayerController, __int64 a3)
	{
		FTransform Transform;
		Transform.Translation = ((AActor*)a3)->K2_GetActorLocation();
		Transform.Scale3D = { 1,1,1 };

		GameMode->RestartPlayerAtTransform(PlayerController, Transform);

		if (auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn))
			Native::GrantAbilities(Pawn);
	}

	static void Init()
	{
		auto DefaultFortPCAthena = UObject::FindObject<AFortPlayerControllerAthena>("/Game/Athena/Athena_PlayerController.Default__Athena_PlayerController_C");
		auto DefaultFortAbilitySystemComp = UFortAbilitySystemComponentAthena::StaticClass()->CreateDefaultObject();
		auto DefaultFortPlayerStateAthena = AFortPlayerStateAthena::StaticClass()->CreateDefaultObject();
		auto DefaultFortPawnAthena = UObject::FindObject<AFortPlayerPawnAthena>("/Game/Athena/PlayerPawn_Athena.Default__PlayerPawn_Athena_C");
		auto DefaultTrapTool = UObject::FindObject<AFortTrapTool>("/Game/Weapons/FORT_BuildingTools/TrapTool.Default__TrapTool_C");

		Util::BindHook(DefaultFortPCAthena, 264, ServerAcknowledgePossessionHook, nullptr);
		Util::BindHook(DefaultFortPCAthena, 510, ServerExecuteInventoryItemHook, nullptr);
		Util::BindHook(DefaultFortPCAthena, 529, ServerAttemptInventoryDropHook, nullptr);
		Util::BindHook(DefaultFortPawnAthena, 455, ServerHandlePickupHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 141, ServerTryActivateAbilityHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 139, ServerTryActivateAbilityWithEventDataHook, nullptr);

		CREATE_HOOK(ClientOnPawnDiedHook, Native::ClientOnPawnDied);
		CREATE_HOOK(RestartPlayerAtPlayerStartHook, Native::RestartPlayerAtPlayerStart);
	}
}