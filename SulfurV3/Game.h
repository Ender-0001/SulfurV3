#pragma once
#include "framework.h"

namespace Game
{
	static void Start()
	{
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
		std::cout << "Playlist: " << Playlist << '\n';

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;

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

		PlayerController->OverriddenBackpackSize = 5;

		PlayerController->bHasClientFinishedLoading = true;
		PlayerController->bHasServerFinishedLoading = true;
		PlayerController->OnRep_bHasServerFinishedLoading();

		PlayerState->bHasFinishedLoading = true;
		PlayerState->bHasStartedPlaying = true;
		PlayerState->OnRep_bHasStartedPlaying();

		static auto BuildingItemData_Wall = UObject::FindObject<UFortBuildingItemDefinition>(("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"));
		static auto BuildingItemData_Floor = UObject::FindObject<UFortBuildingItemDefinition>(("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"));
		static auto BuildingItemData_Stair_W = UObject::FindObject<UFortBuildingItemDefinition>(("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"));
		static auto BuildingItemData_RoofS = UObject::FindObject<UFortBuildingItemDefinition>(("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"));
		static auto EditTool = UObject::FindObject<UFortEditToolItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");
		static auto Pickaxe = UObject::FindObject<UFortWeaponMeleeItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");
		
		Util::GiveItem(PlayerController, BuildingItemData_Wall, 1);
		Util::GiveItem(PlayerController, BuildingItemData_Floor, 1);
		Util::GiveItem(PlayerController, BuildingItemData_Stair_W, 1);
		Util::GiveItem(PlayerController, BuildingItemData_RoofS, 1);
		Util::GiveItem(PlayerController, EditTool, 1);
		Util::GiveItem(PlayerController, Pickaxe, 1);

		static auto WoodItemData = UObject::FindObject<UFortResourceItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");

		Util::GiveItem(PlayerController, WoodItemData, 999);
	}

	static void ServerLoadingScreenDroppedHook(AFortPlayerControllerAthena* PlayerController) // TODO: Move this
	{
		if (auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn))
		{
			static auto HeadPart = UObject::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
			static auto BodyPart = UObject::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");

			Pawn->ServerChoosePart(EFortCustomPartType::Head, HeadPart);
			Pawn->ServerChoosePart(EFortCustomPartType::Body, BodyPart);

			static auto GameplayAbilitySet = UObject::FindObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");

			for (int i = 0; i < GameplayAbilitySet->GameplayAbilities.Num(); i++)
			{
				UClass* AbilityClass = GameplayAbilitySet->GameplayAbilities[i];
				UGameplayAbility* AbilityDefaultObject = (UGameplayAbility*)AbilityClass->CreateDefaultObject();

				/*bool bIsDuplicateAbility = false;

				for (int j = 0; i < Pawn->AbilitySystemComponent->ActivatableAbilities.Items.Num(); j++) // TODO: Check if this is needed
				{
					auto& ActivatableAbility = Pawn->AbilitySystemComponent->ActivatableAbilities.Items[j];

					if (ActivatableAbility.Ability == AbilityDefaultObject)
					{
						bIsDuplicateAbility = true;
						break;
					}
				}

				if (bIsDuplicateAbility)
					continue; */

				FGameplayAbilitySpecHandle Handle{};
				Handle.GenerateNewHandle();

				FGameplayAbilitySpec Spec{ -1, -1, -1 };
				Spec.Ability = AbilityDefaultObject;
				Spec.Level = -1;
				Spec.InputID = -1;
				Spec.Handle = Handle;

				Native::GiveAbility(Pawn->AbilitySystemComponent, &Handle, Spec);
			}
		}

		return Native::ServerLoadingScreenDropped(PlayerController);
	}

	static void ServerAcknowledgePossessionHook(APlayerController* PlayerController, APawn* P)
	{
		PlayerController->AcknowledgedPawn = P;
	}

	static void ServerExecuteInventoryItemHook(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
	{
		if (PlayerController->IsInAircraft())
			return;

		AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

		if (!Pawn)
			return;

		FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;

		TArray<UFortWorldItem*>& ItemInstances = Inventory.ItemInstances;

		for (int i = 0; i < ItemInstances.Num(); i++)
		{
			auto ItemInstance = ItemInstances[i];

			if (ItemInstance->ItemEntry.ItemGuid == ItemGuid)
			{
				// UFortWeaponItemDefinition* WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemInstance->ItemEntry.ItemDefinition);
				UFortWeaponItemDefinition* WeaponDefinition = (UFortWeaponItemDefinition*)ItemInstance->ItemEntry.ItemDefinition;
;
				if (WeaponDefinition)
					Pawn->EquipWeaponDefinition(WeaponDefinition, ItemGuid);
			}
		}
	}

	static void ClientOnPawnDiedHook(AFortPlayerControllerAthena* PlayerController, FFortPlayerDeathReport DeathReport)
	{
		auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);
		auto KillerPawn = Cast<AFortPlayerPawnAthena>(DeathReport.KillerPawn);
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);
		auto KillerPlayerState = Cast<AFortPlayerStateAthena>(DeathReport.KillerPlayerState);
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		if (GameState->GamePhase <= EAthenaGamePhase::Aircraft)
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

	}

	static void ServerTryActivateAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey)
	{
		Util::ActivateAbility(AbilitySystemComponent, Handle, PredictionKey);
	}

	static void ServerTryActivateAbilityWithEventDataHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, FPredictionKey PredictionKey, FGameplayEventData TriggerEventData)
	{
		Util::ActivateAbility(AbilitySystemComponent, Handle, PredictionKey, &TriggerEventData);
	}

	static void RestartPlayerAtPlayerStartHook(AGameMode* GameMode, APlayerController* PlayerController, AActor* StartSpot)
	{
		FTransform Transform{};
		Transform.Translation = StartSpot->K2_GetActorLocation();
		Transform.Scale3D = { 1,1,1 };

		GameMode->RestartPlayerAtTransform(PlayerController, Transform);
	}

	static void ServerCreateBuildingActorHook(AFortPlayerControllerAthena* PlayerController, FCreateBuildingActorData CreateBuildingData)
	{
		UClass* BuildingClass = PlayerController->BroadcastRemoteClientInfo->RemoteBuildableClass;

		/* static */ auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);
		
		bool bAllowedBuildingClass = false;

		for (int i = 0; i < GameState->BuildingActorClasses.Num(); i++)
		{
			auto CurrentClass = GameState->BuildingActorClasses[i];

			if (CurrentClass == BuildingClass)
			{
				bAllowedBuildingClass = true;
				break;
			}
		}

		if (!bAllowedBuildingClass)
			return;

		FVector BuildingLocation = CreateBuildingData.BuildLoc;
		FRotator BuildingRotation = CreateBuildingData.BuildRot;
		bool bMirrored = CreateBuildingData.bMirrored;

		__int64 v32[2]{};
		char a;

		if (!Native::CantBuild(GetWorld(), BuildingClass, BuildingLocation, BuildingRotation, bMirrored, v32, &a))
		{
			ABuildingSMActor* NewBuildingActor = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingLocation, BuildingRotation, BuildingClass);
			NewBuildingActor->InitializeKismetSpawnedBuildingActor(NewBuildingActor, PlayerController, true);
		}
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
		Util::BindHook(DefaultFortPCAthena, 548, ServerCreateBuildingActorHook, nullptr);
		// Util::BindHook(DefaultFortPCAthena, 604, ServerReadyToStartMatchHook, nullptr);
		Util::BindHook(DefaultFortPCAthena, 606, ServerLoadingScreenDroppedHook, (PVOID*)&Native::ServerLoadingScreenDropped);
		Util::BindHook(DefaultFortPawnAthena, 455, ServerHandlePickupHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 141, ServerTryActivateAbilityHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 139, ServerTryActivateAbilityWithEventDataHook, nullptr);

		CREATE_HOOK(ClientOnPawnDiedHook, Native::ClientOnPawnDied);
		CREATE_HOOK(RestartPlayerAtPlayerStartHook, Native::RestartPlayerAtPlayerStart);
	}
}