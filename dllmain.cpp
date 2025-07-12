
#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <math.h> 
#include <vector>

using namespace std;
class CPlayer {
public:
    void* vtable;//0x00
    char pad_4[0x30]; //0x04 + 0x2c = 0x30
    float xpos;//0x34
    float ypos;//0x38
    float zpos;//0x3C
    float yaw;//0x40
    float pitch;//0x44
    char pad_48[0xB0];// 0x44 + 0xB4 = 0xf8
    int health; //0xf8
    int armor; //0xfc = 0xf8 + 0x04
    char pad_100[0x3C];
    int ammo1;//0x13C
    int ammo2;//0x140
    int ammo3;//0x144
    uintptr_t ammo4;//0x0148
    int ammo5;//0x14C
    int ammo6;//0x150
    char pad_154[0x04];
    int ammo7;//0x158
    char pad_15C[0x04];
    int on;//0x160
    char pad_164[0x10];
    int sped;//0x174
    char pad_178[0x0C];
    int sped2;//0x180
    char pad_184[0x1B4];
    int flyhack; //0x338
    char pad_33C[0x4634AC];
    int delay;//0x004637E4


};
class CEntities {
public: 
    void* vtable;            // 0x00
    char pad_4[0x30];        // skip to 0x34
    float x;                 // 0x34
    float y;                 // 0x38
    float z;                 // 0x3C
    char pad_40[0xB8];       // skip to 0xF8
    int health;              // 0xF8

};
bool aimboton = false;
void aimbot(CPlayer* localPlayer, CEntities** entityList, int playercount) {
    CEntities* closestentity = nullptr;
    //finding closest enemy
    float closestDistSq = FLT_MAX;
    float lx = localPlayer->xpos;
    float ly = localPlayer->ypos;
    float lz = localPlayer->zpos;
    cout << "player x: " << lx << endl << "player y: " << ly << endl << "player z: " << lz << endl;
    if (entityList != nullptr) {
            for (int i = 0; i < playercount; i++)
            {
                CEntities* entity = entityList[i];
                if (!entity || entity == reinterpret_cast<CEntities*>(localPlayer)) {
                    continue;
                }
                else {
                    float dx = entity->x - lx;
                    float dy = entity->y - ly;
                    float dz = entity->z - lz;
                    float distSq = dx * dx + dy * dy;
                    if (distSq < closestDistSq && entity->health > 0)
                    {
                        closestDistSq = distSq;
                        closestentity = entity;
                        //cout << "closest enemy is player #" << closestentity << " enemy is at X: " << closestentity->x << " Y: " << closestentity->y << " Z: " << closestentity->z << endl;

                    }
                }

            }
        if (closestentity != nullptr) {
            float deltax;
            float deltay;
            float deltaz;

                deltax = closestentity->x - lx;
                deltay = closestentity->y - ly;
                deltaz = closestentity->z - lz;
                
                float newyaw = atan2f(deltax, -deltay) * (180.0f / 3.1415926535f);
                float dist2d = sqrtf(deltax * deltax + deltay * deltay);
                float newpitch = atan2f(deltaz, dist2d) * (180.0f / 3.1415926535f);
                if (newyaw < 0)
                {
                    newyaw += 360.0f;
                }
                else if (newyaw > 360.0f)
                {
                    newyaw -= 360.0f;
                }
                //cout << "yaw: " << newyaw << " pitch: " << newpitch << endl;
                localPlayer->yaw = newyaw;
                localPlayer->pitch = newpitch;
        }
    }
}
typedef void(__thiscall* tUpdate)(void*);
tUpdate oUpdate = nullptr;
CEntities** g_entityList = nullptr;
CPlayer* g_localPlayer = nullptr;
uintptr_t base = (uintptr_t)GetModuleHandleA("ac_client.exe");
bool hkupdatecall = false;
bool flyhackon = false;
void PatchNOP() {
    if (!base) {
        cout << "Error: Base address is null" << endl;
        return;
    }

    uintptr_t addr = base + 0x637E4;
    BYTE patch[] = { 0x90, 0x90 }; // NOP NOP
    DWORD oldProtect;

    // Make memory writable
    if (VirtualProtect((void*)addr, sizeof(patch), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        // Apply patch
        memcpy((void*)addr, patch, sizeof(patch));
        // Restore protection
        VirtualProtect((void*)addr, sizeof(patch), oldProtect, &oldProtect);
        cout << "NOP patch applied at +0x637E4" << endl;
    }
    else {
        cout << "VirtualProtect failed!" << endl;
    }
}
bool rapidfire = false;
bool norecoil = false;
void __fastcall hkUpdate(CPlayer* ecx, void* edx) {
    if (hkupdatecall == false)
    {
        cout << "hook thread called" << endl;
        hkupdatecall = true;
    }
    if (ecx->health <= 100) {
        cout << "updating health" << endl;
        ecx->health = 1337;
    }
    if (ecx->armor < 100) {
        cout << "updating armor" << endl;
        ecx->armor = 1337;
    }
    ecx->ammo1 = 9999;
    ecx->ammo2 = 9999;
    ecx->ammo3 = 9999;
    ecx->ammo4 = (uintptr_t)99;
    ecx->ammo5 = 9999;
    ecx->ammo6 = 9999;
    ecx->ammo7 = 9999;
    if (GetAsyncKeyState(VK_F3)) {
        flyhackon = !flyhackon;
        cout << "flyhack " << (flyhackon ? "on" : "off") << endl;
        Sleep(500);
    }
    if (GetAsyncKeyState(VK_F4)) {

        if (rapidfire == false) {
            Sleep(1000);
            PatchNOP();
            //ecx->delay = 1000000;
            ecx->sped = 200;
            ecx->on = 1;
            ecx->sped2 = 200;
            rapidfire = true;
        }
    }
    if (GetAsyncKeyState(VK_F5)) {
        norecoil = !norecoil;
        if (norecoil) {
            vector<BYTE> nop(10, 0x90);
            DWORD old;
            void* dst = (void*)(base + 0x63786);
            VirtualProtect(dst, 10, PAGE_EXECUTE_READWRITE, &old);
            memcpy(dst, nop.data() , 10);
            VirtualProtect(dst, 10, old, &old);
        }
        else if (!norecoil)
        {
            void* dst = (void*)(base + 0x63786);
            BYTE* src = (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2";
            DWORD old;
            VirtualProtect(dst, 10, PAGE_EXECUTE_READWRITE, &old);
            memcpy(dst, src, 10);
            VirtualProtect(dst, 10, old, &old);

        }
        Sleep(250);
    }

    if (flyhackon)
    {
        ecx->flyhack = 5;
    }
    else
    {
        ecx->flyhack = 0;
    }

    oUpdate(ecx);
}
DWORD WINAPI AimbotThread(LPVOID) {
    static uintptr_t base = (uintptr_t)GetModuleHandleA("ac_client.exe");

    while (true) {
        // Toggle aimbot on F2 press
        if (GetAsyncKeyState(VK_RBUTTON) & 1) { // only triggers on key **press down**
            aimboton = !aimboton;
            std::cout << "[*] Aimbot toggled: " << (aimboton ? "ON" : "OFF") << std::endl;
            Sleep(150); // simple debounce
        }

        if (aimboton) {
            if (!g_entityList) {
                g_entityList = *(CEntities***)(base + 0x10F4F8);
                if (!g_entityList) {
                    std::cout << "[!] entity list is null\n";
                    Sleep(100);
                    continue;
                }
            }

            int playerCount = *(int*)(base + 0x10F500);

            if (g_localPlayer && g_entityList) {
                aimbot(g_localPlayer, g_entityList, playerCount);
            }
        }

        Sleep(5); // smooth + responsive loop
    }

    return 0;
}
class VMT {
    
    //hook
public:
    void** original = nullptr;
    void** custom = nullptr;
    void*** instance = nullptr;
    size_t count = 0;

    VMT(void* obj) {
        instance = (void***)obj;
        original = *instance;

        while ((BYTE*)original[count] >= (BYTE*)GetModuleHandle(nullptr))
            count++;
        custom = new void* [count];
        memcpy(custom, original, sizeof(uintptr_t) * count);
        cout << "ts hooked" << endl;

    }
    void Hook(int index, void* function) {
        custom[index] = function;
    }
    void apply() {
        *instance = custom;
    }
    void Unhook() {
        *instance = original;
    }
    
};
void AttachConsoleWindow() {
    AllocConsole();
    SetConsoleTitleA("Assault CUBE INTERNAL DEBUG");
    FILE* fOut;
    freopen_s(&fOut, "CONOUT$", "w", stdout);
    FILE* fIn;
    freopen_s(&fIn, "CONIN$", "r", stdin);

}

DWORD WINAPI MainThread(HMODULE hModule) {
    AttachConsoleWindow();

    CPlayer* localPlayer = *(CPlayer**)(base + 0x10F4F4);
    CEntities** entityList = *(CEntities***)(base + 0x10F4F8);
    if (!localPlayer)
    {
        cout << "Local Player NOTT FOUND!!!";
        return 0;
    }
    else {
        g_localPlayer = localPlayer;
    }
    if (!entityList)
    {
        cout << "entity list not found!!!";
    }
    else {
        g_entityList = entityList;
    }
    if (localPlayer) {

    }
    CreateThread(0, 0, AimbotThread, 0, 0, 0);
    VMT hook((void*)localPlayer);
    oUpdate = (tUpdate)hook.original[1];
    hook.Hook(1, (void*)&hkUpdate);
    hook.apply();

    MessageBoxA(0, "DLL Injected!", "Success", MB_OK);
    while (!GetAsyncKeyState(VK_F1)) Sleep(100);
        hook.Unhook();

    cout << "F1 pressed, unhooking" << endl;
    FreeLibraryAndExitThread(hModule, 0);
    return 0;

}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}

