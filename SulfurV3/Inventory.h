#include "framework.h"

namespace Inventory
{
	static void Update(AFortPlayerControllerAthena* PlayerController)
	{
		PlayerController->WorldInventory->HandleInventoryLocalUpdate();
		PlayerController->HandleWorldInventoryLocalUpdate();
		PlayerController->ClientForceUpdateQuickbar(EFortQuickBars::Primary);
		PlayerController->ClientForceUpdateQuickbar(EFortQuickBars::Secondary);

		PlayerController->WorldInventory->Inventory.MarkArrayDirty();
	}

	static UFortWorldItem* FindItemInstance(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDef)
	{
		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			auto ItemInstance = PlayerController->WorldInventory->Inventory.ItemInstances[i];

			if (ItemInstance && ItemInstance->ItemEntry.ItemDefinition == ItemDef)
				return ItemInstance;
		}

		return nullptr;
	}

	static FFortItemEntry* FindItemEntry(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDef)
	{
		if (!PlayerController->WorldInventory)
			return nullptr;

		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (PlayerController->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDef)
				return &PlayerController->WorldInventory->Inventory.ReplicatedEntries[i];
		}

		return nullptr;
	}

	static UFortWorldItem* FindItemInstance(AFortPlayerControllerAthena* PlayerController, FGuid Item)
	{
		if (!PlayerController->WorldInventory)
			return nullptr;

		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			auto ItemInstance = PlayerController->WorldInventory->Inventory.ItemInstances[i];

			if (ItemInstance && ItemInstance->ItemEntry.ItemGuid == Item)
				return ItemInstance;
		}

		return nullptr;
	}

	static FFortItemEntry* FindItemEntry(AFortPlayerControllerAthena* PlayerController, FGuid Item)
	{
		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (PlayerController->WorldInventory->Inventory.ReplicatedEntries[i].ItemGuid == Item)
				return PlayerController->WorldInventory->Inventory.ReplicatedEntries.GetRef(i);
		}

		return nullptr;
	}

	static std::vector<FFortItemEntry*> FindAllEntries(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDef)
	{
		std::vector<FFortItemEntry*> Entries;

		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ReplicatedEntries.Num(); i++)
		{
			if (PlayerController->WorldInventory->Inventory.ReplicatedEntries[i].ItemDefinition == ItemDef)
				Entries.push_back(&PlayerController->WorldInventory->Inventory.ReplicatedEntries[i]);
		}

		return Entries;
	}

	static std::vector<UFortWorldItem*> FindAllInstances(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDef)
	{
		std::vector<UFortWorldItem*> Instances;

		for (int i = 0; i < PlayerController->WorldInventory->Inventory.ItemInstances.Num(); i++)
		{
			auto ItemInstance = PlayerController->WorldInventory->Inventory.ItemInstances[i];

			if (ItemInstance && ItemInstance->ItemEntry.ItemDefinition == ItemDef)
				Instances.push_back(ItemInstance);
		}

		return Instances;
	}

	static bool RemoveItem(AFortPlayerControllerAthena* PlayerController, FGuid Item, int Count)
	{
		if (!PlayerController || !PlayerController->WorldInventory)
			return false;

		auto ItemInstance = FindItemInstance(PlayerController, Item);
		auto ItemEntry = FindItemEntry(PlayerController, Item);
		auto ReplicatedEntries = &PlayerController->WorldInventory->Inventory.ReplicatedEntries;
		auto ItemInstances = &PlayerController->WorldInventory->Inventory.ItemInstances;

		if (ItemInstance && ItemEntry)
		{
			auto NewCount = ItemInstance->ItemEntry.Count - Count;

			if (NewCount <= 0)
			{
				auto ReplicatedEntriesProp = UObject::FindObject<UArrayProperty>("/Script/FortniteGame.FortItemList.ReplicatedEntries");
				auto ItemInstancesProp = UObject::FindObject<UArrayProperty>("/Script/FortniteGame.FortItemList.ItemInstances");

				for (int i = 0; i < ItemInstances->Num(); i++)
				{
					auto ItemInstance = ItemInstances->operator[](i);

					if (!ItemInstance)
						continue;

					auto CurrentGuid = ItemInstance->GetItemGuid();

					if (CurrentGuid == Item)
					{
						Native::Array_Remove(ItemInstances, ItemInstancesProp, i);
						break;
					}
				}

				for (int i = 0; i < ReplicatedEntries->Num(); i++)
				{
					auto Entry = ReplicatedEntries->operator[](i);
					auto CurrentGuid = Entry.ItemGuid;

					if (CurrentGuid == Item)
					{
						Native::Array_Remove(ReplicatedEntries, ReplicatedEntriesProp, i);
						break;
					}
				}

				return true;
			}
			else
			{
				ItemInstance->ItemEntry.Count = NewCount;
				ItemEntry->Count = NewCount;

				PlayerController->WorldInventory->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
				PlayerController->WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
			}

			return false;
		}

		return false;
	}

	static FGuid AddItem(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* ItemDef, int Count, bool* Update = nullptr, int LoadedAmmo = 0)
	{
		if (!ItemDef || !PlayerController || !PlayerController->WorldInventory)
			return FGuid();

		if (Update)
			*Update = false;

		auto ItemInstance = FindItemInstance(PlayerController, ItemDef);

		if (ItemInstance && ItemDef->MaxStackSize >= (Count + ItemInstance->ItemEntry.Count))
		{
			auto NewCount = ItemInstance->ItemEntry.Count + Count;
			ItemInstance->ItemEntry.Count = NewCount;

			auto ItemEntry = FindItemEntry(PlayerController, ItemDef);
			ItemEntry->Count = NewCount;

			PlayerController->WorldInventory->Inventory.MarkItemDirty(ItemInstance->ItemEntry);
			PlayerController->WorldInventory->Inventory.MarkItemDirty(*ItemEntry);

			return ItemEntry->ItemGuid;
		}

		ItemInstance = (UFortWorldItem*)ItemDef->CreateTemporaryItemInstanceBP(Count, 1);
		if (ItemInstance)
		{
			ItemInstance->ItemEntry.Count = Count;
			ItemInstance->SetOwningControllerForTemporaryItem(PlayerController);

			PlayerController->WorldInventory->Inventory.ItemInstances.Add(ItemInstance);
			PlayerController->WorldInventory->Inventory.ReplicatedEntries.Add(ItemInstance->ItemEntry).LoadedAmmo = LoadedAmmo;

			if (Update)
				*Update = true;

			return ItemInstance->ItemEntry.ItemGuid;
		}
	}

	static bool IsPrimaryQuickbar(UFortItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition->IsA(UFortWeaponMeleeItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortBuildingItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortAmmoItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortResourceItemDefinition::StaticClass()) && !ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass()))
			return true;

		return false;
	}

	static bool IsInventoryFull(AFortPlayerControllerAthena* PlayerController, int Start = 0)
	{
		int ItemCounts = Start;

		auto ItemInstances = &PlayerController->WorldInventory->Inventory.ItemInstances;

		for (int i = 0; i < ItemInstances->Num(); i++)
		{
			auto ItemInstance = ItemInstances->operator[](i);

			if (!ItemInstance)
				continue;

			auto ItemDefinition = ItemInstance->ItemEntry.ItemDefinition;

			if (IsPrimaryQuickbar(ItemDefinition))
				ItemCounts++;
		}

		return ItemCounts >= 6;
	}

	static std::vector<UFortWorldItem*> GetDroppableItems(AFortPlayerControllerAthena* PlayerController, UFortItemDefinition* IgnoreDef = nullptr)
	{
		std::vector<UFortWorldItem*> DroppableItems;

		if (PlayerController->WorldInventory)
		{
			auto ItemInstances = PlayerController->WorldInventory->Inventory.ItemInstances;

			for (int i = 0; i < ItemInstances.Num(); i++)
			{
				auto ItemInstance = ItemInstances[i];

				if (!ItemInstance)
					continue;

				auto ItemEntry = ItemInstance->ItemEntry;
				auto ItemDefiniton = (UFortWorldItemDefinition*)ItemEntry.ItemDefinition;

				if (ItemInstance->CanBeDropped() && ItemInstance->ItemEntry.ItemDefinition != IgnoreDef)
					DroppableItems.push_back(ItemInstance);
			}
		}

		return DroppableItems;
	}

	static AFortPickupAthena* SpawnPickup(UFortItemDefinition* ItemDef, FVector Location, int Count, EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset)
	{
		if (auto Pickup = GetWorld()->SpawnActor<AFortPickupAthena>(Location, {}))
		{
			Pickup->PrimaryPickupItemEntry.Count = Count;
			Pickup->PrimaryPickupItemEntry.ItemDefinition = ItemDef;

			Pickup->OnRep_PrimaryPickupItemEntry();

			Pickup->TossPickup(Location, nullptr, 0, true, PickupSource, SpawnSource);

			if (PickupSource == EFortPickupSourceTypeFlag::Container)
			{
				Pickup->bTossedFromContainer = true;
				Pickup->OnRep_TossedFromContainer();
			}

			Pickup->SetReplicateMovement(true);
			Pickup->MovementComponent = (UProjectileMovementComponent*)UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), Pickup);

			return Pickup;
		}

		return nullptr;
	}

	static AFortPickupAthena* SpawnPickup(FFortItemEntry ItemEntry, FVector Location, EFortPickupSourceTypeFlag PickupSource = EFortPickupSourceTypeFlag::Other, EFortPickupSpawnSource SpawnSource = EFortPickupSpawnSource::Unset)
	{
		auto Pickup = SpawnPickup(ItemEntry.ItemDefinition, Location, ItemEntry.Count, PickupSource, SpawnSource);
		Pickup->PrimaryPickupItemEntry.LoadedAmmo = ItemEntry.LoadedAmmo;
		Pickup->PrimaryPickupItemEntry.ItemDefinition->Rarity = ItemEntry.ItemDefinition->Rarity;
		Pickup->OnRep_PrimaryPickupItemEntry();
		return Pickup;
	}
}