#pragma once
#include "framework.h"
#include <numbers>
#include <unordered_map>

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

    static FFortLootTierData* GetLootTierData(std::vector<FFortLootTierData*>& LootTierData)
    {
        float TotalWeight = 0;

        for (auto Item : LootTierData)
            TotalWeight += Item->Weight;

        float RandomNumber = UKismetMathLibrary::RandomFloatInRange(0, TotalWeight); // is -1 needed?

        FFortLootTierData* SelectedItem = nullptr;

        for (auto Item : LootTierData)
        {
            if (RandomNumber <= Item->Weight)
            {
                SelectedItem = Item;
                break;
            }

            RandomNumber = RandomNumber - Item->Weight;
        }

        if (!SelectedItem)
            return GetLootTierData(LootTierData);

        return SelectedItem;
    }

    static FFortLootPackageData* GetLootPackage(std::vector<FFortLootPackageData*>& LootPackages)
    {
        float TotalWeight = 0;

        for (auto Item : LootPackages)
            TotalWeight += Item->Weight;

        float RandomNumber = UKismetMathLibrary::RandomFloatInRange(0, TotalWeight); // is -1 needed?

        FFortLootPackageData* SelectedItem = nullptr;

        for (auto Item : LootPackages)
        {
            if (RandomNumber <= Item->Weight)
            {
                SelectedItem = Item;
                break;
            }

            RandomNumber = RandomNumber - Item->Weight;
        }

        if (!SelectedItem)
            return GetLootPackage(LootPackages);

        return SelectedItem;
    }

    static int GetClipSize(UFortWeaponItemDefinition* WeaponDef)
    {
        if (!WeaponDef)
            return 0;

        auto Table = WeaponDef->WeaponStatHandle.DataTable;

        if (!Table)
            return 0;

        auto Row = (FFortRangedWeaponStats*)Table->RowMap[WeaponDef->WeaponStatHandle.RowName];

        if (!Row)
            return 0;

        return Row->ClipSize;
    }

    static bool PickLootDrops(FName& TierGroupName, int WorldLevel, int ForcedLootTier, std::vector<FFortItemEntry>& LootDrops)
    {
        static auto LTD = UObject::FindObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"); // Cast<AFortGameStateAthena>(GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist->LootTierData.Get();
        static auto LP = UObject::FindObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"); // Cast<AFortGameStateAthena>(GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist->LootPackages.Get();

        auto& LTDRowMap = LTD->RowMap;

        std::vector<FFortLootTierData*> TierGroupLTDs;

        auto LTDRowMapNum = LTDRowMap.Pairs.Elements.Num();
        auto TierGroupStr = TierGroupName.ToString();

        for (int i = 0; i < LTDRowMapNum; i++)
        {
            auto& CurrentLTD = LTDRowMap.Pairs.Elements[i].ElementData.Value;
            auto TierData = (FFortLootTierData*)CurrentLTD.Value();

            auto TierDataGroupStr = TierData->TierGroup.ToString();

            // std::cout << "TierData->TierGroup.ToString(): " << TierDataGroupStr << '\n';

            if (TierDataGroupStr == TierGroupStr)
            {
                TierGroupLTDs.push_back(TierData);
            }
        }

        if (TierGroupLTDs.size() == 0)
            return false;

        FFortLootTierData* ChosenRowLootTierData = GetLootTierData(TierGroupLTDs);

        if (!ChosenRowLootTierData)
            return false;

        float NumLootPackageDrops = ChosenRowLootTierData->NumLootPackageDrops;

        auto& LPRowMap = LP->RowMap;

        std::vector<FFortLootPackageData*> TierGroupLPs;

        for (int i = 0; i < LPRowMap.Pairs.Elements.Num(); i++)
        {
            auto& CurrentLP = LPRowMap.Pairs.Elements[i].ElementData.Value;
            auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

            if (!LootPackage)
                continue;

            if (LootPackage->LootPackageID == ChosenRowLootTierData->LootPackage)
            {
                TierGroupLPs.push_back(LootPackage);
            }
        }

        bool bIsWorldList = ChosenRowLootTierData->LootPackage.ToString().contains("WorldList");

        LootDrops.reserve(NumLootPackageDrops);

        for (int i = 0; i < NumLootPackageDrops; i++)
        {
            if (i >= TierGroupLPs.size())
                break;

            auto TierGroupLP = TierGroupLPs.at(i);
            auto TierGroupLPStr = TierGroupLP->LootPackageCall.ToString();

            std::vector<FFortLootPackageData*> lootPackageCalls;

            for (int j = 0; j < LPRowMap.Pairs.Elements.Num(); j++)
            {
                auto& CurrentLP = LPRowMap.Pairs.Elements[j].ElementData.Value;

                if (bIsWorldList)
                {
                    auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

                    lootPackageCalls.push_back(LootPackage);
                }
                else
                {
                    auto LootPackage = (FFortLootPackageData*)CurrentLP.Value();

                    if (LootPackage->LootPackageID.ToString() == TierGroupLPStr)
                    {
                        lootPackageCalls.push_back(LootPackage);
                    }
                }
            }

            if (lootPackageCalls.size() == 0)
                continue;

            FFortLootPackageData* LootPackageCall = GetLootPackage(lootPackageCalls);

            if (!LootPackageCall)
                continue;

            auto ItemDef = LootPackageCall->ItemDefinition.Get();

            if (!ItemDef)
                continue;

            FFortItemEntry LootDropEntry{};

            LootDropEntry.ItemDefinition = ItemDef;
            LootDropEntry.LoadedAmmo = GetClipSize(Cast<UFortWeaponItemDefinition>(ItemDef));
            LootDropEntry.Count = LootPackageCall->Count;

            LootDrops.push_back(LootDropEntry);
        }

        return LootDrops.size() > 0;
    }
}