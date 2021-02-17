/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "CGameStateManager.h"

#include <algorithm>

#include "hud/hud.h"

CGameStateManager g_gameStateManager;

void GameStateDebug_PrintState(const GameState state)
{
    gEngfuncs.Con_Printf(" >>> --- Client state: %s --- <<< \n", 
          state == GameState::InMenu        ? "Menu"
        : state == GameState::Paused        ? "Paused"
        : state == GameState::ChangingLevel ? "Changing level"
        : state == GameState::Unpaused      ? "In game"
        :                                     "Unknown"
    );
}

/**
 * This class makes use of a number of quirks of the client, server, and engine
 * processes in order to detect if the game is running, paused, or in the menu.
 * Unfortunately the engine doesn't provide us with APIs for this, but we can use
 * timings and a few client hooks (like init hud, calc refdef) in order to guess
 * what the server state is. Add a server-side message in as well with some hacky
 * engine inspections and we have a functional, if somewhat crazy, implementation.
 */

CGameStateManager::CGameStateManager()
{
    currentState = GameState::InMenu;
    callbacks = new std::vector<void (*)(GameState)>();
    lastClientTime = 0;
    lastAbsoluteTime = 0;
    resetToMenuAbsTime = -1;

    // Uncomment this to see state changes in the console
    // this->RegisterCallback(GameStateDebug_PrintState);
}

void CGameStateManager::InitHud()
{
    // InitHud is called when the game first loads
    // Which means the player is in-game and unpaused
    SetCurrentState(GameState::Unpaused);
    lastClientTime = gEngfuncs.GetClientTime();
    lastAbsoluteTime = gEngfuncs.GetAbsoluteTime();
    resetToMenuAbsTime = -1;
}

void CGameStateManager::CalcRefDef(bool paused)
{
    // V_CalcRefdef is called every frame when the player is in-game
    // even if they are paused. It is not called in the main menu, so we
    // know that they can only be either paused or unpaused
    if (paused && currentState != GameState::Paused) SetCurrentState(GameState::Paused);
    if (!paused && currentState != GameState::Unpaused) SetCurrentState(GameState::Unpaused);
    resetToMenuAbsTime = -1;
}

void CGameStateManager::HudFrame()
{
    const auto clTime = gEngfuncs.GetClientTime();
    const auto absTime = gEngfuncs.GetAbsoluteTime();

    // Hud_Frame is called every frame after the client dll is first
    // loaded, even when in the menu. The client time resets to 0 when
    // a new map is loaded, or when the client disconnects.
    // But afterwards it might jump up, depending on the level change, so
    // the menu detection must be in absolute time.
    if (clTime < lastClientTime)
    {
        // The time has reset, a new map has loaded, or we're back at the menu
        // If the server has hinted that we're changing levels, allow quite a delay
        // before we disconnect. Otherwise, don't allow much time at all.
        if (currentState == GameState::ChangingLevel) resetToMenuAbsTime = absTime + 5;
        else resetToMenuAbsTime = absTime + 0.1f;
    }

    lastClientTime = clTime;
    lastAbsoluteTime = absTime;

    //if (resetToMenuAbsTime >= 0) gEngfuncs.Con_Printf("Last: %f, Menu: %f, Time: %f; Abs: %f \n", lastClientTime, resetToMenuAbsTime, time, gEngfuncs.GetAbsoluteTime());
    if (resetToMenuAbsTime >= 0 && resetToMenuAbsTime < absTime)
    {
        // The menu time has expired - drop to the menu
        SetCurrentState(GameState::InMenu);
        resetToMenuAbsTime = -1;
    }
}

void CGameStateManager::UpdateServerState(const int state)
{
    // Don't treat these as gospel (for the loading/inactive),
    // but these will hint to us about what the server is up to
    switch (state)
    {
        case 0: // inactive
            // Server shouldn't send us this state, it's just a placeholder
            break;
        case 1: // active
            // Only do something if we still think we're in the menu.
            // We can't be in the menu, because the server has contacted us.
            // We don't change from any of the other states here, as we have more
            // reliable ways of detecting those.
            if (currentState == GameState::InMenu) SetCurrentState(GameState::Unpaused);
            resetToMenuAbsTime = -1;
            break;
        case 2: // paused
            // Don't listen to the server about pausing.
            // We get reliable pause status from the refdef.
            break;
        case 3: // loading
            // The server has suggested to us that it's changing levels, so
            // we delay any menu disconnection for a while.
            SetCurrentState(GameState::ChangingLevel);
            resetToMenuAbsTime = gEngfuncs.GetAbsoluteTime() + 5;
            break;
    }
}


void CGameStateManager::SetCurrentState(GameState state)
{
    if (currentState == state) return;

    currentState = state;

    // callbacks
    std::for_each(callbacks->begin(), callbacks->end(), [state](void (*callback)(GameState))
    {
        callback(state);
    });
}

void CGameStateManager::RegisterCallback(void (*callback)(GameState)) const
{
    callbacks->push_back(callback);
}

void CGameStateManager::UnregisterCallback(void (*callback)(GameState)) const
{
    callbacks->erase(std::remove(callbacks->begin(), callbacks->end(), callback), callbacks->end());
}
