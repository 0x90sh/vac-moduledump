#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>

#include <MinHook/MinHook.h>
#include "util.hpp"
// SPDX-License-Identifier: MIT
//
// Copyright (c) 2025 0x90
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction... yaddah yaddah yaddah
// 
//s/o to og creator i forked code from: https://github.com/nevioo1337/VAC-ModuleDumper
// pretty much just optimized a bit and fixed signature to load vac modules routine in steamservice.dll

std::string dumpPath = "C:\\VACModules";

int moduleCounter = 0;
typedef void(__stdcall* LoadVACModule)(DWORD* VACModule, char flags);
LoadVACModule pLoadVACModule = nullptr;
void __stdcall LoadVACModuleDetour(DWORD* VACModule, char flags) {
    if ((void*)VACModule[6] != nullptr) {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0A);

        std::cout << "[+] LoadVACModule called (" << std::dec << moduleCounter << ")" << std::endl;
        CreateDirectoryA(dumpPath.c_str(), NULL);
        std::ofstream file(dumpPath + "\\VACModule_" + std::to_string(moduleCounter) + ".dll", std::ios::out | std::ios::binary);
        file.write((char*)VACModule[6], VACModule[5]);
        file.close();

        moduleCounter++;
    }
    return pLoadVACModule(VACModule, flags);
}

void moduleDumper() {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x0E);
    SetConsoleTitleA("VAC Module Dumper [dbg]");
	std::cout << "VAC Module Dumper" << std::endl;
    HWND hConsole = GetConsoleWindow();
    if (hConsole) {
        ShowWindow(hConsole, SW_RESTORE);
        SetWindowPos(hConsole, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        SetForegroundWindow(hConsole);
    }

    HMODULE steamService = nullptr;
    while (steamService == nullptr) {
        steamService = GetModuleHandleA("steamservice.dll");
    }
    std::cout << "[1/3] Found steamservice.dll" << std::endl;

    uintptr_t funcPtr = util::findSig(steamService, "55 8B EC 83 EC 24 53 56 8B 75 08 8B D9 83 7E 0C 00 0F 85 ? ? ? ?");
    if (funcPtr == 0) {
        std::cerr << "ERROR: Signature not found!" << std::endl;
        return;
    }
    std::cout << "[2/3] Found signature at: 0x" << std::hex << funcPtr << std::endl;

    MH_Initialize();
    MH_CreateHook(reinterpret_cast<LPVOID>(funcPtr), &LoadVACModuleDetour, reinterpret_cast<LPVOID*>(&pLoadVACModule));
    MH_EnableHook(reinterpret_cast<LPVOID>(funcPtr));
    std::cout << "[3/3] Hooked LoadVacModule function in steamservice.dll" << std::endl;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        moduleDumper();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

