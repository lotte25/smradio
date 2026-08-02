#pragma once

#include <map>
#include <fmod.h>
#include <fmod_studio.h>
#include <MinHook.h>
#include <fmt/format.h>

typedef FMOD_RESULT (__fastcall* System_Update_t)(void* thiz);
typedef FMOD_RESULT (__fastcall* EventInstance_Start_t)(void* thiz);
typedef FMOD_RESULT (__fastcall* EventInstance_Stop_t)(void* thiz, FMOD_STUDIO_STOP_MODE mode);
typedef FMOD_RESULT (__fastcall* EventInstance_Set3DAttributes_t)(void* thiz, const FMOD_3D_ATTRIBUTES* attributes);
typedef FMOD_RESULT (__fastcall* EventInstance_SetPaused_t)(void* thiz, FMOD_BOOL paused);
typedef FMOD_RESULT (__fastcall* EventInstance_SetVolume_t)(void* thiz, float volume);

extern System_Update_t o_Update;
extern EventInstance_Start_t o_Start;
extern EventInstance_Stop_t o_Stop;
extern EventInstance_Set3DAttributes_t o_Set3DAttributes;
extern EventInstance_SetVolume_t o_SetVolume;

struct ActiveRadio {
    FMOD_SOUND* sound = nullptr;
    FMOD_CHANNEL* channel = nullptr;
};

extern int g_LogThrottle;
extern FMOD_SYSTEM* g_coreSystem;
extern std::map<FMOD_STUDIO_EVENTINSTANCE*, ActiveRadio> g_activeRadios;

DWORD WINAPI InitHooks(LPVOID lpParam);

FMOD_RESULT F_API h_Update(FMOD_SYSTEM* system);
FMOD_RESULT F_API h_Start(FMOD_STUDIO_EVENTINSTANCE* thiz);
FMOD_RESULT F_API h_Stop(FMOD_STUDIO_EVENTINSTANCE* eventinstance, FMOD_STUDIO_STOP_MODE mode);
FMOD_RESULT F_API h_Set3DAttributes(FMOD_STUDIO_EVENTINSTANCE* eventinstance, const FMOD_3D_ATTRIBUTES* attributes);
FMOD_RESULT F_API h_SetVolume(FMOD_STUDIO_EVENTINSTANCE* eventinstance, float volume);