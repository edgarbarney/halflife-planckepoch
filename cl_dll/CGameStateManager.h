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

#pragma once

#include <vector>

enum class GameState
{
    InMenu,
    Paused,
    Unpaused,
    ChangingLevel
};

class CGameStateManager
{
public:
    CGameStateManager();

    void InitHud();
    void CalcRefDef(bool paused);
    void HudFrame();
    void UpdateServerState(const int state);

    bool IsInMenu() const { return currentState == GameState::InMenu; }
    bool IsPaused() const { return currentState == GameState::Paused; }
    bool IsUnpaused() const { return currentState == GameState::Unpaused; }

    void RegisterCallback(void (*callback)(GameState)) const;
    void UnregisterCallback(void (*callback)(GameState)) const;

private:
    void SetCurrentState(GameState state);

    float lastClientTime;
    double lastAbsoluteTime;
    double resetToMenuAbsTime;
    GameState currentState;
    std::vector<void (*)(GameState)> *callbacks;
};

extern CGameStateManager g_gameStateManager;
