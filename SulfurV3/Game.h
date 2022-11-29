#pragma once
#include "framework.h"
#include "Util.h"

namespace Game
{
	static void PreLoad()
	{
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");

		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;

		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameMode->GameSession = GetWorld()->SpawnActor<AFortGameSessionDedicatedAthena>({}, {});
	}

	static void Start()
	{
		auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
		auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GameState);

		GameMode->GameSession->MaxPlayers = 100;
		GameMode->bWorldIsReady = true;

		GameMode->StartMatch();
		GameMode->StartPlay();

		auto SpawnIsland_FloorLoot = UObject::FindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
		auto BRIsland_FloorLoot = UObject::FindObject<UBlueprintGeneratedClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");

		TArray<AActor*> SpawnIsland_FloorLoot_Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), SpawnIsland_FloorLoot, &SpawnIsland_FloorLoot_Actors);

		TArray<AActor*> BRIsland_FloorLoot_Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), BRIsland_FloorLoot, &BRIsland_FloorLoot_Actors);

		auto SpawnIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot_Warmup");
		auto BRIslandTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaFloorLoot");

		for (int i = 0; i < SpawnIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)SpawnIsland_FloorLoot_Actors[i];

			auto Location = CurrentActor->K2_GetActorLocation();
			Location.Z += 50;

			std::vector<FFortItemEntry> LootDrops;

			if (Util::PickLootDrops(SpawnIslandTierGroup, 1, -1, LootDrops))
			{
				for (auto& LootDrop : LootDrops)
					Inventory::SpawnPickup(LootDrop, Location, EFortPickupSourceTypeFlag::FloorLoot);
			}
		}

		for (int i = 0; i < BRIsland_FloorLoot_Actors.Num(); i++)
		{
			ABuildingContainer* CurrentActor = (ABuildingContainer*)BRIsland_FloorLoot_Actors[i];

			auto Location = CurrentActor->K2_GetActorLocation();
			Location.Z += 50;

			std::vector<FFortItemEntry> LootDrops;

			if (Util::PickLootDrops(BRIslandTierGroup, 1, -1, LootDrops))
			{
				for (auto& LootDrop : LootDrops)
					Inventory::SpawnPickup(LootDrop, Location, EFortPickupSourceTypeFlag::FloorLoot);
			}
		}
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

		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData"), 500);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData"), 300);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData"), 100);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03"), 1);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/ShieldSmall/Athena_ShieldSmall.Athena_ShieldSmall"), 6);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells"), 999);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium"), 999);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight"), 999);
		Inventory::AddItem(PlayerController, UObject::FindObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy"), 999);
		Inventory::Update(PlayerController);

		static auto HeadPart = UObject::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1");
		static auto BodyPart = UObject::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01");
		static auto BackpackPart = UObject::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Backpacks/NoBackpack.NoBackpack");

		PlayerState->CharacterData.Parts[0] = HeadPart;
		PlayerState->CharacterData.Parts[1] = BodyPart;
		PlayerState->CharacterData.Parts[3] = BackpackPart;
		PlayerState->OnRep_CharacterData();

		static auto GameplayAbilitySet = UObject::FindObject<UFortAbilitySet>("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");

		for (int i = 0; i < GameplayAbilitySet->GameplayAbilities.Num(); i++)
		{
			UClass* AbilityClass = GameplayAbilitySet->GameplayAbilities[i];
			UGameplayAbility* AbilityDefaultObject = (UGameplayAbility*)AbilityClass->CreateDefaultObject();

			FGameplayAbilitySpecHandle Handle{};
			Handle.GenerateNewHandle();

			FGameplayAbilitySpec Spec{ -1, -1, -1 };
			Spec.Ability = AbilityDefaultObject;
			Spec.Level = -1;
			Spec.InputID = -1;
			Spec.Handle = Handle;

			Native::GiveAbility(PlayerState->AbilitySystemComponent, &Handle, Spec);
		}
	}

	static void ServerAcknowledgePossessionHook(APlayerController* PlayerController, APawn* P)
	{
		PlayerController->AcknowledgedPawn = P;
	}

	static void ServerExecuteInventoryItemHook(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid)
	{
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

		if (!Pawn)
			return;

		auto ItemEntry = Inventory::FindItemEntry(PlayerController, ItemGuid);

		if (!ItemEntry)
			return;

		auto ItemDef = ItemEntry->ItemDefinition;

		if (auto GadgetDef = Cast<UAthenaGadgetItemDefinition>(ItemDef))
			ItemDef = GadgetDef->GetWeaponItemDefinition();

		if (!ItemDef)
			return;

		if (auto Weapon = Pawn->EquipWeaponDefinition(Cast<UFortWeaponItemDefinition>(ItemDef), ItemGuid))
		{
			ItemEntry->LoadedAmmo = Weapon->AmmoCount;
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

			auto DroppableItems = Inventory::GetDroppableItems(PlayerController);

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

				Inventory::SpawnPickup(Item->ItemEntry, DeathInfo.DeathLocation, EFortPickupSourceTypeFlag::Player, EFortPickupSpawnSource::PlayerElimination);
				Inventory::RemoveItem(PlayerController, Item->ItemEntry.ItemGuid, Item->ItemEntry.Count);
			}

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

	static void ServerHandlePickupHook(AFortPlayerPawnAthena* Pawn, AFortPickup* Pickup, float InFlyTime, FVector InStartDirection, char bPlayPickupSound)
	{
		auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

		if (Pickup && !Pickup->bPickedUp)
		{
			auto ItemDef = Cast<UFortWorldItemDefinition>(Pickup->PrimaryPickupItemEntry.ItemDefinition);
			auto ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;
			auto ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;

			if (!ItemDef)
				return;

			bool Update = false;

			if (Inventory::IsPrimaryQuickbar(ItemDef) && Inventory::IsInventoryFull(PlayerController, 1) && Pawn->CurrentWeapon)
			{
				auto CurrentItemGuid = Pawn->CurrentWeapon->ItemEntryGuid;

				if (auto ItemInstance = Inventory::FindItemInstance(PlayerController, CurrentItemGuid))
				{
					auto ItemEntry = &ItemInstance->ItemEntry;

					if (ItemEntry->ItemDefinition && ItemInstance->CanBeDropped())
					{
						if (ItemEntry)
						{
							Inventory::SpawnPickup(*ItemEntry, Pawn->K2_GetActorLocation());
							Inventory::RemoveItem(PlayerController, CurrentItemGuid, ItemEntry->Count);
							Update = true;
						}
					}
				}
			}

			auto Item = Inventory::AddItem(PlayerController, ItemDef, Pickup->PrimaryPickupItemEntry.Count, &Update, Pickup->PrimaryPickupItemEntry.LoadedAmmo);

			if (Update)
				Inventory::Update(PlayerController);

			Pickup->PickupLocationData.PickupTarget = Pawn;
			Pickup->PickupLocationData.FlyTime = 0.50;
			Pickup->PickupLocationData.ItemOwner = Pawn;
			Pickup->OnRep_PickupLocationData();

			Pickup->bPickedUp = true;
			Pickup->OnRep_bPickedUp();
		}
	}


	static void ServerHandlePickupWithRequestedSwapHook(AFortPlayerPawnAthena* Pawn, AFortPickup* Pickup, FGuid Swap, float InFlyTime, FVector InStartDirection, bool bPlayPickupSound)
	{
		auto PlayerController = (AFortPlayerControllerAthena*)Pawn->Controller;

		if (Pickup && !Pickup->bPickedUp && PlayerController->GetUseHoldToSwapPickupSetting())
		{
			auto ItemDef = Cast<UFortWorldItemDefinition>(Pickup->PrimaryPickupItemEntry.ItemDefinition);
			auto ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;
			auto ReplicatedEntries = PlayerController->WorldInventory->Inventory.ReplicatedEntries;

			if (!ItemDef)
				return;

			bool Update = false;

			if (Inventory::IsPrimaryQuickbar(ItemDef) && Pawn->CurrentWeapon)
			{
				auto CurrentItemGuid = Swap;

				if (auto ItemInstance = Inventory::FindItemInstance(PlayerController, CurrentItemGuid))
				{
					auto ItemEntry = &ItemInstance->ItemEntry;
					if (ItemEntry->ItemDefinition && ItemInstance->CanBeDropped())
					{
						if (ItemEntry)
						{
							Inventory::SpawnPickup(*ItemEntry, Pawn->K2_GetActorLocation());
							Inventory::RemoveItem(PlayerController, CurrentItemGuid, ItemEntry->Count);
							Update = true;
						}
					}
				}
			}

			auto Item = Inventory::AddItem(PlayerController, ItemDef, Pickup->PrimaryPickupItemEntry.Count, &Update, Pickup->PrimaryPickupItemEntry.LoadedAmmo);

			if (Update)
				Inventory::Update(PlayerController);

			Pickup->PickupLocationData.PickupTarget = Pawn;
			Pickup->PickupLocationData.FlyTime = 0.50;
			Pickup->PickupLocationData.ItemOwner = Pawn;
			Pickup->OnRep_PickupLocationData();

			Pickup->bPickedUp = true;
			Pickup->OnRep_bPickedUp();
		}
	}

	static void ServerAttemptInventoryDropHook(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, int Count)
	{
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

		if (!Pawn)
			return;

		if (auto ItemEntry = Inventory::FindItemEntry(PlayerController, ItemGuid))
		{
			for (int i = 0; i < Pawn->CurrentWeaponList.Num(); i++)
			{
				auto Weapon = Pawn->CurrentWeaponList[i];

				if (Weapon && Weapon->ItemEntryGuid == ItemEntry->ItemGuid)
				{
					ItemEntry->LoadedAmmo = Weapon->AmmoCount;
					break;
				}
			}

			if (auto Pickup = Inventory::SpawnPickup(*ItemEntry, Pawn->K2_GetActorLocation()))
			{
				Pickup->PrimaryPickupItemEntry.Count = Count;
				Pickup->OnRep_PrimaryPickupItemEntry();
			}

			Inventory::RemoveItem(PlayerController, ItemGuid, Count);
			Inventory::Update(PlayerController);
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

	static void RestartPlayerAtPlayerStartHook(AGameMode* GameMode, APlayerController* PlayerController, AActor* StartSpot)
	{
		FTransform Transform{};
		Transform.Translation = StartSpot->K2_GetActorLocation();
		Transform.Scale3D = { 1,1,1 };

		GameMode->RestartPlayerAtTransform(PlayerController, Transform);
	}

	static void ServerCreateBuildingActorHook(AFortPlayerControllerAthena* PlayerController, FCreateBuildingActorData& CreateBuildingActorData)
	{
		if (!PlayerController->BroadcastRemoteClientInfo)
			return;

		if (!PlayerController->BroadcastRemoteClientInfo->RemoteBuildableClass)
			return;

		char v33[8];
		__int64 v41[3]{};

		v41[0] = 0;
		v41[1] = 0;

		if (!(*(unsigned int(*)(AFortPlayerControllerAthena*, UClass*, void*, void*, DWORD, __int64*, char*))(PlayerController->VFT[0x333]))(
			PlayerController,
			PlayerController->BroadcastRemoteClientInfo->RemoteBuildableClass,
			&CreateBuildingActorData.BuildLoc,
			&CreateBuildingActorData.BuildRot,
			CreateBuildingActorData.bMirrored,
			v41,
			v33))
		{
			auto Building = GetWorld()->SpawnActor<ABuildingSMActor>(CreateBuildingActorData.BuildLoc, CreateBuildingActorData.BuildRot, PlayerController->BroadcastRemoteClientInfo->RemoteBuildableClass);
			if (Building)
			{
				Building->SetMirrored(CreateBuildingActorData.bMirrored);
				Building->InitializeKismetSpawnedBuildingActor(Building, PlayerController, true);

				if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState))
					Building->TeamIndex = PlayerState->TeamIndex;

				Building->OnRep_Team();

				Building->bPlayerPlaced = true;

				bool bUpdate = false;

				if (PlayerController->bBuildFree)
					return;

				if (auto ItemEntry = Inventory::FindItemEntry(PlayerController, UFortKismetLibrary::K2_GetResourceItemDefinition(Building->ResourceType)))
				{
					if (Inventory::RemoveItem(PlayerController, ItemEntry->ItemGuid, 10))
						Inventory::Update(PlayerController);

					return;
				}
			}
		}
	}

	static void ServerBeginEditingBuildingActorHook(AFortPlayerControllerAthena* PlayerController, ABuildingSMActor* BuildingActorToEdit)
	{
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController->Pawn);

		if (!Pawn)
			return;

		if (Pawn->CurrentWeapon)
		{
			if (Pawn->CurrentWeapon->IsA(AFortWeap_BuildingToolBase::StaticClass()))
				return;
		}

		if (PlayerController && BuildingActorToEdit)
		{
			static auto EditToolDef = UObject::FindObject<UFortWeaponItemDefinition>("/Game/Items/Weapons/BuildingTools/EditTool.EditTool");

			if (auto EditTool = Cast<AFortWeap_EditingTool>(Pawn->EquipWeaponDefinition(EditToolDef, FGuid{})))
			{
				EditTool->EditActor = BuildingActorToEdit;
				EditTool->OnRep_EditActor();

				BuildingActorToEdit->EditingPlayer = Cast<AFortPlayerStateZone>(Pawn->PlayerState);
				BuildingActorToEdit->OnRep_EditingPlayer();
			}
		}
	}

	static void ServerEditBuildingActorHook(AFortPlayerControllerAthena* PlayerController, ABuildingSMActor* BuildingActorToEdit, UClass* NewBuildingClass, int RotationIterations, char bMirrored)
	{
		if (!BuildingActorToEdit || !NewBuildingClass)
			return;

		if (auto BuildingActor = Native::BuildingSMActorReplaceBuildingActor(BuildingActorToEdit, 1, NewBuildingClass, 0, RotationIterations, bMirrored, PlayerController))
		{
			BuildingActor->bPlayerPlaced = true;

			if (auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState))
				BuildingActor->SetTeam(PlayerState->TeamIndex);

			BuildingActor->OnRep_Team();
		}
	}

	static void ServerEndEditingBuildingActorHook(AFortPlayerControllerAthena* PlayerController, ABuildingSMActor* BuildingActorToStopEditing)
	{
		if (!PlayerController->IsInAircraft() && BuildingActorToStopEditing && PlayerController->Pawn)
		{
			auto EditTool = Cast<AFortWeap_EditingTool>((((AFortPlayerPawnAthena*)PlayerController->Pawn)->CurrentWeapon));

			BuildingActorToStopEditing->EditingPlayer = nullptr;
			BuildingActorToStopEditing->OnRep_EditingPlayer();

			if (EditTool)
			{
				EditTool->bEditConfirmed = true;
				EditTool->EditActor = nullptr;
				EditTool->OnRep_EditActor();
			}
		}
	}

	static void ServerAttemptInteractHook(UFortControllerComponent_Interaction* InteractionComp, AActor* ReceivingActor, __int64* a3, __int64 a4, unsigned int a5, __int64 a6)
	{
		Native::ServerAttemptInteract(InteractionComp, ReceivingActor, a3, a4, a5, a6);

		if (InteractionComp && ReceivingActor)
		{		
			if (auto BuildingContainer = Cast<ABuildingContainer>(ReceivingActor))
			{
				if (!BuildingContainer->bAlreadySearched)
				{	
					std::vector<FFortItemEntry> LootDrops;
					auto SearchLootTierGroup = BuildingContainer->SearchLootTierGroup;

					auto CorrectLocation = BuildingContainer->K2_GetActorLocation() + BuildingContainer->GetActorRightVector() * 70.0f + FVector{ 0, 0, 50 };

					EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset;

					if (SearchLootTierGroup.ToString() == "Loot_Treasure") // Very bad, we should probably do a loop of all chests and ammo boxes and fix their SearchLootTierGroup.
					{
						SearchLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaTreasure");
						SpawnSource = EFortPickupSpawnSource::Chest;
					}

					if (SearchLootTierGroup.ToString() == "Loot_Ammo")
					{
						SearchLootTierGroup = UKismetStringLibrary::Conv_StringToName(L"Loot_AthenaAmmoLarge");
						SpawnSource = EFortPickupSpawnSource::AmmoBox;
					}

					if (Util::PickLootDrops(SearchLootTierGroup, -1, 1, LootDrops))
					{
						BuildingContainer->bAlreadySearched = true;
						BuildingContainer->OnRep_bAlreadySearched();
						BuildingContainer->OnLoot();

						for (auto& LootDrop : LootDrops)
							Inventory::SpawnPickup(LootDrop, CorrectLocation, EFortPickupSourceTypeFlag::Container, SpawnSource);
					}
				}
			}
		}
	}

	static void OnDamageServerHook(ABuildingActor* BuildingActor, float Damage, struct FGameplayTagContainer DamageTags, struct FVector Momentum, struct FHitResult HitInfo, class AController* InstigatedBy, class AActor* DamageCauser, struct FGameplayEffectContextHandle EffectContext)
	{
		auto BuildingSMActor = Cast<ABuildingSMActor>(BuildingActor);
		auto PlayerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);
		auto Pawn = Cast<AFortPlayerPawnAthena>(PlayerController ? PlayerController->Pawn : nullptr);
		auto Weapon = Cast<AFortWeapon>(DamageCauser);

		if (!BuildingSMActor)
			return;

		if (!PlayerController)
			return;

		if (!Pawn)
			return;

		if (!Weapon)
			return;

		if (BuildingSMActor->bPlayerPlaced)
			return;

		if (Weapon->WeaponData && Cast<UFortWeaponMeleeItemDefinition>(Weapon->WeaponData))
		{
			if (PlayerController)
			{		
				auto ResourceCount = 0;
				UFortResourceItemDefinition* ItemDef = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingSMActor->ResourceType);

				auto BuildingResourceAmountOverride = BuildingSMActor->BuildingResourceAmountOverride;

				if (BuildingResourceAmountOverride.RowName.IsValid())
				{
					printf("Evaluating\n");

					//static auto CurveTable = UObject::FindObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaResourceRates.AthenaResourceRates");
					TEnumAsByte<EEvaluateCurveTableResult> Result;
					float Out;

					printf("X: %f\n", HitInfo.Time);
					std::cout << std::format("Momentum: {} {} {}\n", Momentum.X, Momentum.Y, Momentum.Z);
					UDataTableFunctionLibrary::EvaluateCurveTableRow(BuildingResourceAmountOverride.CurveTable, BuildingResourceAmountOverride.RowName, HitInfo.Time, L"", &Result, &Out);
					ResourceCount = Out;
					printf("Out: %f\n", Out);
				}

				if (!ItemDef || ResourceCount == 0 || !BuildingResourceAmountOverride.RowName.IsValid())
				{
					Native::OnDamageServer(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
					return;
				}

				bool bIsWeakspot = Damage == 100.0f;

				if (bIsWeakspot)
					ResourceCount *= 2; // for now..

				bool Update;
				Inventory::AddItem(PlayerController, ItemDef, ResourceCount, &Update);

				if (Update)
					Inventory::Update(PlayerController);

				PlayerController->ClientReportDamagedResourceBuilding(BuildingSMActor, BuildingSMActor->ResourceType, ResourceCount, false, bIsWeakspot);
			}
		}

		Native::OnDamageServer(BuildingActor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);
	}

	static char GetTeamIndexHook(UObject* a1, UObject* a2)
	{
		return 4;
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
		Util::BindHook(DefaultFortPCAthena, 550, ServerEditBuildingActorHook, nullptr);
		Util::BindHook(DefaultFortPCAthena, 553, ServerEndEditingBuildingActorHook, nullptr);
		Util::BindHook(DefaultFortPCAthena, 555, ServerBeginEditingBuildingActorHook, nullptr);
		Util::BindHook(DefaultFortPawnAthena, 455, ServerHandlePickupHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 141, ServerTryActivateAbilityHook, nullptr);
		Util::BindHook(DefaultFortAbilitySystemComp, 139, ServerTryActivateAbilityWithEventDataHook, nullptr);

		CREATE_HOOK(ClientOnPawnDiedHook, Native::ClientOnPawnDied);
		CREATE_HOOK(RestartPlayerAtPlayerStartHook, Native::RestartPlayerAtPlayerStart);
		CREATE_HOOK(ServerAttemptInteractHook, Native::ServerAttemptInteract);
		CREATE_HOOK(OnDamageServerHook, Native::OnDamageServer);
		CREATE_HOOK(GetTeamIndexHook, Native::GetTeamIndex);
	}
}