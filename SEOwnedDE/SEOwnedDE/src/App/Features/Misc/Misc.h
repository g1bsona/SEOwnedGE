#pragma once

#include "../../../SDK/SDK.h"

class CMisc
{
public:
	void Bunnyhop(CUserCmd* pCmd);
	void AutoStrafer(CUserCmd* pCmd);
	void NoiseMakerSpam();
	void FastStop(CUserCmd* pCmd);

	void AutoRocketJump(CUserCmd* cmd);
	void AutoDisguise(CUserCmd* cmd);
	void AutoMedigun(CUserCmd* cmd);
	void MovementLock(CUserCmd* cmd);
	void MvmInstaRespawn();
	void RegionChanger();
	
	// Event handling for backstab detection
	void OnPlayerDeath(IGameEvent* pEvent);

private:
	// Backstab tracking for AutoDisguise
	C_TFPlayer* m_pBackstabVictim = nullptr;
	float m_flLastBackstabTime = 0.0f;
};

MAKE_SINGLETON_SCOPED(CMisc, Misc, F);
