#include "framework.h"

static void Main()
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "BaseAddr: " << std::hex << (uintptr_t)GetModuleHandle(0) << '\n';

    auto PlayerController = GEngine->GameInstance->LocalPlayers[0]->PlayerController;

    PlayerController->SwitchLevel(L"Athena_Terrain?game=athena");

    GEngine->GameInstance->LocalPlayers.Remove(0);

    srand(time(0));

    Hooks::Init();
    Game::Init();
}

int __stdcall DllMain(void*, int dwReason)
{
    if (dwReason == 1)
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&Main, 0, 0, 0);

    return 1;
}

