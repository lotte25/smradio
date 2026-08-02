#include "hooks.hpp"

System_Update_t o_Update = nullptr;
EventInstance_Start_t o_Start = nullptr;
EventInstance_Stop_t o_Stop = nullptr;
EventInstance_Set3DAttributes_t o_Set3DAttributes = nullptr;
EventInstance_SetVolume_t o_SetVolume = nullptr;

int g_LogThrottle = 0;
FMOD_SYSTEM* g_coreSystem = nullptr;
std::map<FMOD_STUDIO_EVENTINSTANCE*, ActiveRadio> g_activeRadios;

FMOD_RESULT F_API h_Update(FMOD_SYSTEM* system) {
    if (g_coreSystem == nullptr) {
        printf("radio: update - got system! %p\n", system);
        g_coreSystem = system;
    }

    for (auto& pair : g_activeRadios) {
        FMOD_STUDIO_EVENTINSTANCE* event = pair.first;
        ActiveRadio& radio = pair.second;

        if (radio.sound != nullptr && radio.channel == nullptr) {
            FMOD_OPENSTATE status;
            FMOD_Sound_GetOpenState(radio.sound, &status, nullptr, nullptr, nullptr);
    
            if (status == FMOD_OPENSTATE_READY) {
                printf("radio: update - stream is ready\n");
                FMOD_System_PlaySound(g_coreSystem, radio.sound, nullptr, true, &radio.channel);

                FMOD_3D_ATTRIBUTES attributes;
                if (FMOD_Studio_EventInstance_Get3DAttributes(event, &attributes) == FMOD_OK) {
                    FMOD_Channel_Set3DAttributes(radio.channel, &attributes.position, &attributes.velocity);
                }

                FMOD_Channel_SetPaused(radio.channel, false);
            } else if (status == FMOD_OPENSTATE_ERROR) {
                printf("radio: update - error\n");
                FMOD_Sound_Release(radio.sound);
                radio.sound = nullptr;
            } else if (status == FMOD_OPENSTATE_CONNECTING) {
                // printf("radio: update - loading\n");
            }
        }
    }

    return o_Update(system);
}

FMOD_RESULT F_API h_Start(FMOD_STUDIO_EVENTINSTANCE* thiz) { 
    FMOD_STUDIO_EVENTINSTANCE* eventinstance = static_cast<FMOD_STUDIO_EVENTINSTANCE*>(thiz);
    FMOD_STUDIO_EVENTDESCRIPTION* eventDesc = nullptr;
    FMOD_Studio_EventInstance_GetDescription(eventinstance, &eventDesc);

    char eventPath[256] = { 0 };
    int retrieved = 0;
    FMOD_Studio_EventDescription_GetPath(eventDesc, eventPath, 256, &retrieved);
    
    if (strstr(eventPath, "radioReciever")) {
        // printf("radio: reciever start: %s\n", eventPath);
        FMOD_Studio_EventInstance_SetVolume(eventinstance, 0.0f);

        ActiveRadio& radio = g_activeRadios[eventinstance];
        if (radio.sound != nullptr) {
            if (radio.channel) FMOD_Channel_Stop(radio.channel);
            FMOD_Sound_Release(radio.sound);
        }

        if (g_coreSystem) {
            FMOD_CREATESOUNDEXINFO exinfo = { 0 };
            exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            exinfo.suggestedsoundtype = FMOD_SOUND_TYPE_OGGVORBIS;

            FMOD_RESULT res = FMOD_System_CreateSound(
                g_coreSystem, 
                "http://radio.tonightsong.com:8000/stream.ogg",
                FMOD_CREATESTREAM | FMOD_3D | FMOD_NONBLOCKING, 
                &exinfo, 
                &radio.sound
            );

            if (res != FMOD_OK) {
                printf("radio: start - GOT AN ERROR FROM CREATE: %i", res);
            }
        }
    }

    return o_Start(thiz);
}

FMOD_RESULT F_API h_Stop(FMOD_STUDIO_EVENTINSTANCE* eventinstance, FMOD_STUDIO_STOP_MODE mode) {
    auto it = g_activeRadios.find(eventinstance);
    if (it != g_activeRadios.end()) {
        printf("radio: stop - radio reciever %i\n", static_cast<int>(std::distance(g_activeRadios.begin(), it)));
        ActiveRadio& radio = it->second;
        
        if (radio.channel) FMOD_Channel_Stop(radio.channel);
        if (radio.sound) FMOD_Sound_Release(radio.sound);

        g_activeRadios.erase(it);
    }

    return o_Stop(eventinstance, mode);
}

FMOD_RESULT F_API h_Set3DAttributes(FMOD_STUDIO_EVENTINSTANCE* eventinstance, const FMOD_3D_ATTRIBUTES* attributes) {
/*     std::string text = fmt::format("radio: 3d update -> Pos({:.2f}, {:.2f}, {:.2f}) Vel({:.2f}, {:.2f}, {:.2f})\n", 
        attributes->position.x, attributes->position.y, attributes->position.z,
        attributes->velocity.x, attributes->velocity.y, attributes->velocity.z
    ); */

    // printf(text.c_str());

    auto it = g_activeRadios.find(eventinstance);

    if (it != g_activeRadios.end()) {
        if (it->second.channel != nullptr) {
            FMOD_Channel_Set3DAttributes(it->second.channel, &attributes->position, &attributes->velocity);
        }
    }

    return o_Set3DAttributes(eventinstance, attributes);
}

FMOD_RESULT F_API h_SetVolume(FMOD_STUDIO_EVENTINSTANCE* eventinstance, float volume) {
    auto it = g_activeRadios.find(eventinstance);
    if (it != g_activeRadios.end()) {
        if (it->second.channel != nullptr) {
            FMOD_Channel_SetVolume(it->second.channel, volume);
        }
        return o_SetVolume(eventinstance, 0.f);
    }
    return o_SetVolume(eventinstance, volume);
}

DWORD WINAPI InitHooks(LPVOID lpParam) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    setvbuf(stdout, nullptr, _IONBF, 0);

    printf("Scrap Mechanic Radio Mod!! (github:lotte25/smradio)\n");
    printf("Default radio (tonightsong.com) by JunePost\n");
    printf("- - - - - - - -");

    HMODULE hFmod = nullptr;
    while (!(hFmod = GetModuleHandleA("fmodstudio.dll"))) {
        Sleep(100);
    }

    HMODULE hFmodCore = nullptr;
    while (!(hFmodCore = GetModuleHandleA("fmod.dll"))) {
        Sleep(100);
    }

    // printf("radio: got handles (%p, %p)\n", hFmod, hFmodCore);

    if (MH_Initialize() != MH_OK) {
        printf("radio: error initializing minhook");
        return 1;
    }
    
    void* UpdateAddr = (void*)GetProcAddress(hFmodCore, "?update@System@FMOD@@QEAA?AW4FMOD_RESULT@@XZ");
    void* StartAddr = (void*)GetProcAddress(hFmod, "?start@EventInstance@Studio@FMOD@@QEAA?AW4FMOD_RESULT@@XZ");
    void* Set3DAttributesAddr = (void*)GetProcAddress(hFmod, "?set3DAttributes@EventInstance@Studio@FMOD@@QEAA?AW4FMOD_RESULT@@PEBUFMOD_3D_ATTRIBUTES@@@Z");
    void* SetVolumeAddr = (void*)GetProcAddress(hFmod, "?setVolume@EventInstance@Studio@FMOD@@QEAA?AW4FMOD_RESULT@@M@Z");
    void* StopAddr = (void*)GetProcAddress(hFmod, "?stop@EventInstance@Studio@FMOD@@QEAA?AW4FMOD_RESULT@@W4FMOD_STUDIO_STOP_MODE@@@Z");

    MH_STATUS status = MH_OK;

    status = MH_CreateHook(
        UpdateAddr, 
        reinterpret_cast<LPVOID>(&h_Update), 
        reinterpret_cast<LPVOID*>(&o_Update)
    );
    if (status != MH_OK) {
        printf("radio: EventInstance_Start hook creation failed - %i\n", status);
    };

    status = MH_CreateHook(
        StartAddr, 
        reinterpret_cast<LPVOID>(&h_Start), 
        reinterpret_cast<LPVOID*>(&o_Start)
    );
    if (status != MH_OK) {
        printf("radio: EventInstance_Start hook creation failed - %i\n", status);
    };

    status = MH_CreateHook(
        StopAddr, 
        reinterpret_cast<LPVOID>(&h_Stop), 
        reinterpret_cast<LPVOID*>(&o_Stop)
    );
    if (status != MH_OK) {
        printf("radio: EventInstance_Start hook creation failed - %i\n", status);
    };

    status = MH_CreateHook(
        Set3DAttributesAddr, 
        reinterpret_cast<LPVOID>(&h_Set3DAttributes), 
        reinterpret_cast<LPVOID*>(&o_Set3DAttributes)
    );
    if (status != MH_OK) {
        printf("radio: EventInstance_Set3DAttributes hook creation failed - %i\n", status);
    }
    
    status = MH_CreateHook(
        SetVolumeAddr, 
        reinterpret_cast<LPVOID>(&h_SetVolume), 
        reinterpret_cast<LPVOID*>(&o_SetVolume)
    );
    if (status != MH_OK) {
        printf("radio: EventInstance_SetPaused hook creation failed - %i\n", status);
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        printf("radio: failed to enable all hooks");
        return 1;
    } else {
        printf("radio: applied hooks\n");
    }

    return 0;
}