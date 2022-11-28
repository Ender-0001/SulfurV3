#pragma once
#include "framework.h"
#include <numbers>

#define HALF_PI 1.5707964
#define PI 3.14159265359

namespace Util
{
    static void BindHook(void* Object, int Index, void* Detour, void** Original)
    {
        if (Original)
            *Original = (*(void***)(Object))[Index];
        (*(void***)(Object))[Index] = Detour;
    }

    static FGameplayAbilitySpec* FindAbilitySpecFromHandle(UAbilitySystemComponent* AbilityComp, FGameplayAbilitySpecHandle Handle)
    {
        for (int i = 0; i < AbilityComp->ActivatableAbilities.Items.Num(); i++)
        {
            if (AbilityComp->ActivatableAbilities.Items[i].Handle.Handle == Handle.Handle)
            {
                return &AbilityComp->ActivatableAbilities.Items[i];
            }
        }

        return nullptr;
    }

    static FGameplayAbilitySpec* FindAbilitySpecFromClass(UAbilitySystemComponent* AbilityComp, UClass* Class)
    {
        for (int i = 0; i < AbilityComp->ActivatableAbilities.Items.Num(); i++)
        {
            if (AbilityComp->ActivatableAbilities.Items[i].Ability && AbilityComp->ActivatableAbilities.Items[i].Ability->Class == Class)
            {
                return &AbilityComp->ActivatableAbilities.Items[i];
            }
        }

        return nullptr;
    }

    static void ActivateAbility(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Ability, FPredictionKey PredictionKey, FGameplayEventData* EventData = nullptr)
    {
        auto Spec = Util::FindAbilitySpecFromHandle(AbilitySystemComponent, Ability);

        if (!Spec)
        {
            AbilitySystemComponent->ClientActivateAbilityFailed(Ability, PredictionKey.Current);
            return;
        }

        UGameplayAbility* InstancedAbility;
        Spec->InputPressed = true;
        if (!Native::InternalTryActivateAbility(AbilitySystemComponent, Ability, PredictionKey, &InstancedAbility, nullptr, EventData))
        {
            AbilitySystemComponent->ClientActivateAbilityFailed(Ability, PredictionKey.Current);
            Spec->InputPressed = false;
        }

        AbilitySystemComponent->ActivatableAbilities.MarkArrayDirty();
    }

    static std::vector<AFortPlayerStateAthena*> GetAlivePlayers()
    {
        std::vector<AFortPlayerStateAthena*> AlivePlayers;

        if (!GetWorld()->NetDriver)
            return AlivePlayers;

        for (int i = 0; i < GetWorld()->NetDriver->ClientConnections.Num(); i++)
        {
            auto Connection = GetWorld()->NetDriver->ClientConnections[i];
            auto PlayerController = Cast<AFortPlayerControllerAthena>(Connection->PlayerController);

            if (!PlayerController)
                continue;

            auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->PlayerState);

            if (!PlayerState)
                continue;

            if (PlayerController->bMarkedAlive)
                AlivePlayers.push_back(PlayerState);
        }

        return AlivePlayers;
    }

    static bool OnSameTeam(std::vector<AFortPlayerStateAthena*> Players)
    {
        for (auto Player : Players)
        {
            if (Player)
            {
                for (auto CurrentPlayer : Players)
                {
                    if (CurrentPlayer)
                    {
                        if (CurrentPlayer->TeamIndex != Player->TeamIndex)
                            return false;
                    }
                }
            }
        }

        return true;
    }

    static void Update(AFortPlayerControllerAthena* PlayerController, bool bMarkArrayDirty = true) // TODO: Add the quickbar to update?
    {
        FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;

        if (bMarkArrayDirty)
            Inventory.MarkArrayDirty();

        PlayerController->WorldInventory->HandleInventoryLocalUpdate();

        PlayerController->ClientForceUpdateQuickbar(EFortQuickBars::Primary);
        PlayerController->ClientForceUpdateQuickbar(EFortQuickBars::Secondary);
    }

    static void SummonPickup(AFortPawn* Pawn, UFortWorldItemDefinition* ItemDefinition, int Count, const FVector& Location, EFortPickupSourceTypeFlag SourceFlag, EFortPickupSpawnSource SpawnSource)
    {
        auto Pickup = GetWorld()->SpawnActor<AFortPickupAthena>(Location, FRotator());

        Pickup->PrimaryPickupItemEntry.ItemDefinition = ItemDefinition;
        Pickup->PrimaryPickupItemEntry.Count = Count;

        if (Pawn)
            Pickup->TossPickup(Location, Pawn, 0, true, SourceFlag, SpawnSource);

        Pickup->SetReplicateMovement(true);
        Pickup->MovementComponent = (UProjectileMovementComponent*)UGameplayStatics::SpawnObject(UProjectileMovementComponent::StaticClass(), Pickup);
    }

    static void RemoveItem(AFortPlayerControllerAthena* PlayerController, const FGuid& ItemGuid, int Count)
    {
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

                // SummonPickup(ItemInstance->ItemEntry.ItemDefinition, Count, PlayerController->MyFortPawn->K2_GetActorLocation());

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

    static UFortWorldItem* GiveItem(AFortPlayerControllerAthena* PlayerController, UFortWorldItemDefinition* ItemDefinition, int Count)
    {
        FFortItemList& Inventory = PlayerController->WorldInventory->Inventory;

        TArray<UFortWorldItem*>& ItemInstances = Inventory.ItemInstances;
        TArray<FFortItemEntry>& ReplicatedEntries = Inventory.ReplicatedEntries;

        UFortWorldItem* FortItem = (UFortWorldItem*)ItemDefinition->CreateTemporaryItemInstanceBP(Count, 1);

        FortItem->SetOwningControllerForTemporaryItem(PlayerController);

        ItemInstances.Add(FortItem);
        ReplicatedEntries.Add(FortItem->ItemEntry);

        Update(PlayerController);

        return FortItem;
    }

    static void SinCos(float* ScalarSin, float* ScalarCos, float  Value)
    {
        // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
        float quotient = (0.31830988618379067153776752674502872406891929148091 * 0.5f) * Value;
        if (Value >= 0.0f)
        {
            quotient = (float)((int)(quotient + 0.5f));
        }
        else
        {
            quotient = (float)((int)(quotient - 0.5f));
        }
        float y = Value - (2.0f * PI) * quotient;

        float sign;
        if (y > HALF_PI)
        {
            y = PI - y;
            sign = -1.0f;
        }
        else if (y < -HALF_PI)
        {
            y = -PI - y;
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        float y2 = y * y;

        *ScalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

        float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *ScalarCos = sign * p;
    }

    template <class  T>
    static auto DegreesToRadians(T const& DegVal) -> decltype(DegVal* (PI / 180.f))
    {
        return DegVal * (PI / 180.f);
    }

    static std::string GetName(UObject* Object)
    {
        auto Namef = UKismetSystemLibrary::GetObjectName(Object);
        auto Name = Namef.ToString();
        VirtualFree((void*)Namef.c_str(), sizeof(wchar_t) * Namef.Count, MEM_RELEASE);
        Namef.Count = 0;
        Namef.Max = 0;
        Namef.Data = nullptr;
        return Name;
    }

    static std::string GetFullName(UObject* Object)
    {
        auto Namef = UKismetSystemLibrary::GetPathName(Object);
        auto Name = Namef.ToString();
        VirtualFree((void*)Namef.c_str(), sizeof(wchar_t) * Namef.Count, MEM_RELEASE);
        Namef.Count = 0;
        Namef.Max = 0;
        Namef.Data = nullptr;
        return Name;
    }

    static std::string FName_ToString(FName InName)
    {
        auto Namef = UKismetStringLibrary::Conv_NameToString(InName);
        auto Name = Namef.ToString();
        VirtualFree((void*)Namef.c_str(), sizeof(wchar_t) * Namef.Count, MEM_RELEASE);
        Namef.Count = 0;
        Namef.Max = 0;
        Namef.Data = nullptr;

        return Name;
    }
}