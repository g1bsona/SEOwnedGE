#include "Misc.h"

#include "../CFG.h"

void CMisc::Bunnyhop(CUserCmd* pCmd)
{
	if (!CFG::Misc_Bunnyhop)
		return;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (pLocal->deadflag() || pLocal->m_nWaterLevel() > static_cast<byte>(WL_Feet))
			return;

		static bool bJumpState = false;

		if (pCmd->buttons & IN_JUMP)
		{
			if (!bJumpState && !(pLocal->m_fFlags() & FL_ONGROUND))
				pCmd->buttons &= ~IN_JUMP;

			else if (bJumpState)
				bJumpState = false;
		}

		else if (!bJumpState)
		{
			bJumpState = true;
		}
	}
}

void CMisc::AutoStrafer(CUserCmd* pCmd)
{
	//credits: KGB

	if (!CFG::Misc_Auto_Strafe)
		return;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (pLocal->deadflag() || (pLocal->m_fFlags() & FL_ONGROUND))
			return;

		if (pLocal->m_nWaterLevel() > static_cast<byte>(WL_Feet) || pLocal->GetMoveType() != MOVETYPE_WALK)
			return;

		if (!(pLocal->m_afButtonLast() & IN_JUMP) && (pCmd->buttons & IN_JUMP))
			return;

		if (pCmd->buttons & IN_MOVELEFT || pCmd->buttons & IN_MOVERIGHT || pCmd->buttons & IN_FORWARD || pCmd->buttons & IN_BACK)
		{
			const float flForwardMove = pCmd->forwardmove;
			const float flSideMove = pCmd->sidemove;

			Vec3 vForward = {}, vRight = {};
			Math::AngleVectors(pCmd->viewangles, &vForward, &vRight, nullptr);

			vForward.z = vRight.z = 0.0f;

			vForward.Normalize();
			vRight.Normalize();

			Vec3 vWishDir = {};
			Math::VectorAngles({(vForward.x * flForwardMove) + (vRight.x * flSideMove), (vForward.y * flForwardMove) + (vRight.y * flSideMove), 0.0f}, vWishDir);

			Vec3 vCurDir = {};
			Math::VectorAngles(pLocal->m_vecVelocity(), vCurDir);

			const float flDirDelta = Math::NormalizeAngle(vWishDir.y - vCurDir.y);
			const float flTurnScale = Math::RemapValClamped(CFG::Misc_Auto_Strafe_Turn_Scale, 0.0f, 1.0f, 0.9f, 1.0f);
			const float flRotation = DEG2RAD((flDirDelta > 0.0f ? -90.0f : 90.f) + (flDirDelta * flTurnScale));

			const float flCosRot = cosf(flRotation);
			const float flSinRot = sinf(flRotation);

			pCmd->forwardmove = (flCosRot * flForwardMove) - (flSinRot * flSideMove);
			pCmd->sidemove = (flSinRot * flForwardMove) + (flCosRot * flSideMove);
		}
	}
}

void CMisc::NoiseMakerSpam()
{
	if (!CFG::Misc_NoiseMaker_Spam)
		return;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (pLocal->deadflag() || pLocal->m_flNextNoiseMakerTime() >= I::GlobalVars->curtime)
			return;

		const auto kv = new KeyValues("use_action_slot_item_server");
		I::EngineClient->ServerCmdKeyValues(kv);
	}
}

void CMisc::FastStop(CUserCmd* pCmd)
{
	if (!CFG::Misc_Fast_Stop)
		return;

	if (const auto pLocal = H::Entities->GetLocal())
	{
		if (!pLocal->deadflag()
			&& pLocal->GetMoveType() == MOVETYPE_WALK
			&& (pLocal->m_fFlags() & FL_ONGROUND)
			&& pLocal->m_vecVelocity().Length2D() >= 10.0f
			&& !(pCmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVERIGHT | IN_MOVELEFT)))
		{
			const Vector velocity = pLocal->m_vecVelocity();
			QAngle direction;

			Math::VectorAngles(velocity, direction);

			const float speed = velocity.Length();

			direction.y = I::EngineClient->GetViewAngles().y - direction.y;

			Vec3 forward{};

			Math::AngleVectors(direction, &forward);

			const Vector negatedDirection = forward * -speed;

			pCmd->forwardmove = negatedDirection.x;
			pCmd->sidemove = negatedDirection.y;
		}
	}
}

void CMisc::AutoRocketJump(CUserCmd* cmd)
{
	if (!H::Input->IsDown(CFG::Misc_Auto_Rocket_Jump_Key)
		|| I::EngineVGui->IsGameUIVisible()
		|| I::MatSystemSurface->IsCursorVisible()
		|| SDKUtils::BInEndOfMatch())
	{
		return;
	}

	const auto local{H::Entities->GetLocal()};

	if (!local
		|| local->deadflag()
		|| local->GetMoveType() != MOVETYPE_WALK
		|| local->m_nWaterLevel() > WL_Feet
		|| local->m_fFlags() & FL_DUCKING
		|| !(local->m_fFlags() & FL_ONGROUND)
		|| local->InCond(TF_COND_TAUNTING)
		|| local->InCond(TF_COND_HALLOWEEN_GHOST_MODE)
		|| local->InCond(TF_COND_HALLOWEEN_BOMB_HEAD)
		|| local->InCond(TF_COND_HALLOWEEN_KART))
	{
		return;
	}

	const auto weapon{H::Entities->GetWeapon()};

	if (!weapon
		|| (weapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER && weapon->GetWeaponID() != TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT)
		|| weapon->m_iItemDefinitionIndex() == Soldier_m_TheBeggarsBazooka
		|| !weapon->HasPrimaryAmmoForShot())
	{
		return;
	}

	if (weapon->m_iReloadMode() != 0)
	{
		cmd->buttons |= IN_ATTACK;
	}

	if (local->m_vecViewOffset().z > 45.65f)
	{
		cmd->buttons |= IN_DUCK;
	}

	else
	{
		auto pitch{Math::RemapValClamped(I::EngineClient->GetViewAngles().x, -89.0f, 0.0f, 89.0f, 50.0f)};
		auto yaw{Math::NormalizeAngle(Math::VelocityToAngles(local->m_vecVelocity()).y + 180.0f)};

		if (!(cmd->buttons & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT)))
		{
			yaw = Math::NormalizeAngle(I::EngineClient->GetViewAngles().y + 180.0f);
		}

		cmd->viewangles.Set(pitch, yaw, weapon->m_iItemDefinitionIndex() != Soldier_m_TheOriginal ? 90.0f : 0.0f);

		cmd->buttons |= IN_ATTACK | IN_DUCK | IN_JUMP;
		cmd->buttons &= ~IN_ATTACK2;

		G::bSilentAngles = true;
	}
}

void CMisc::AutoDisguise(CUserCmd* cmd)
{
	if (!CFG::Misc_Auto_Disguise)
	{
		return;
	}

	const auto local{H::Entities->GetLocal()};

	if (!local || local->deadflag() || local->m_iClass() != TF_CLASS_SPY)
	{
		return;
	}

	// Don't do anything if we're already disguised or disguising
	if (local->InCond(TF_COND_DISGUISED) || local->InCond(TF_COND_DISGUISING))
	{
		return;
	}

	// Check if we're using Your Eternal Reward - skip disguise logic if so
	const auto weapon = H::Entities->GetWeapon();
	if (weapon && weapon->m_iItemDefinitionIndex() == Spy_t_YourEternalReward)
	{
		return;
	}

	// Using our backstab victim information populated from game events
	if (m_flLastBackstabTime > 0.0f && (I::GlobalVars->curtime - m_flLastBackstabTime) < 0.5f && m_pBackstabVictim)
	{
		// Clear the backstab data since we're handling it now
		m_flLastBackstabTime = 0.0f;
		
		// Get victim's class
		int victimClass = m_pBackstabVictim->m_iClass();
		
		// Skip Heavy, Soldier, and Demoman
		if (victimClass == TF_CLASS_HEAVYWEAPONS || victimClass == TF_CLASS_SOLDIER || victimClass == TF_CLASS_DEMOMAN)
		{
			// Find nearest player that isn't Heavy/Soldier/Demoman
			float closestDistance = FLT_MAX;
			C_TFPlayer* closestPlayer = nullptr;
			
			for (const auto ent : H::Entities->GetGroup(EEntGroup::PLAYERS_ENEMIES))
			{
				if (!ent || ent == m_pBackstabVictim)
					continue;
				
				auto player = ent->As<C_TFPlayer>();
				if (!player || player->deadflag())
					continue;
					
				int playerClass = player->m_iClass();
				if (playerClass == TF_CLASS_HEAVYWEAPONS || playerClass == TF_CLASS_SOLDIER || playerClass == TF_CLASS_DEMOMAN)
					continue;
					
				float distance = player->GetAbsOrigin().DistTo(local->GetAbsOrigin());
				if (distance < closestDistance)
				{
					closestDistance = distance;
					closestPlayer = player;
				}
			}
			
			if (closestPlayer)
			{
				// Disguise as the closest valid player
				int disguiseClass = closestPlayer->m_iClass();
				char command[32];
				sprintf_s(command, "disguise %d -1", disguiseClass);
				I::EngineClient->ClientCmd_Unrestricted(command);
			}
			else
			{
				// No valid player found, do random disguise
				// Choose a random class that isn't Heavy/Soldier/Demoman
				int validClasses[] = {TF_CLASS_SCOUT, TF_CLASS_PYRO, TF_CLASS_MEDIC, TF_CLASS_ENGINEER, TF_CLASS_SNIPER, TF_CLASS_SPY};
				int randomIndex = rand() % 6;
				int randomClass = validClasses[randomIndex];
				
				char command[32];
				sprintf_s(command, "disguise %d -1", randomClass);
				I::EngineClient->ClientCmd_Unrestricted(command);
			}
		}
		else
		{
			// Victim is not Heavy/Soldier/Demoman, disguise as them
			char command[32];
			sprintf_s(command, "disguise %d -1", victimClass);
			I::EngineClient->ClientCmd_Unrestricted(command);
		}
		
		// Clear the victim pointer after using it
		m_pBackstabVictim = nullptr;
	}
}

// Process player_death event to track backstabs
void CMisc::OnPlayerDeath(IGameEvent* pEvent)
{
	if (!CFG::Misc_Auto_Disguise)
		return;
		
	const auto local = H::Entities->GetLocal();
	if (!local || local->m_iClass() != TF_CLASS_SPY)
		return;
		
	// Check if we're the attacker
	const auto attacker = pEvent->GetInt("attacker");
	const auto attackerEntIndex = I::EngineClient->GetPlayerForUserID(attacker);
	
	if (attackerEntIndex != local->entindex())
		return;
		
	// Check if it was a backstab
	const auto customkill = pEvent->GetInt("customkill");
	if (customkill != TF_DMG_CUSTOM_BACKSTAB)
		return;
		
	// Get the victim
	const auto victim = pEvent->GetInt("userid");
	const auto victimEntIndex = I::EngineClient->GetPlayerForUserID(victim);
	auto victimEnt = I::ClientEntityList->GetClientEntity(victimEntIndex);
	
	if (!victimEnt)
		return;
		
	// Store the victim and time for processing in our AutoDisguise function
	m_pBackstabVictim = victimEnt->As<C_TFPlayer>();
	m_flLastBackstabTime = I::GlobalVars->curtime;
}

void CMisc::AutoMedigun(CUserCmd* cmd)
{
	static std::vector<C_TFPlayer*> potential{};

	if (!H::Input->IsDown(CFG::Misc_Auto_Medigun_Key))
	{
		if (!potential.empty())
		{
			potential.clear();
		}

		return;
	}

	if (I::EngineVGui->IsGameUIVisible() || I::MatSystemSurface->IsCursorVisible() || SDKUtils::BInEndOfMatch())
	{
		potential.clear();

		return;
	}

	const auto local{H::Entities->GetLocal()};

	if (!local
		|| local->deadflag()
		|| local->InCond(TF_COND_TAUNTING)
		|| local->InCond(TF_COND_HALLOWEEN_GHOST_MODE)
		|| local->InCond(TF_COND_HALLOWEEN_BOMB_HEAD)
		|| local->InCond(TF_COND_HALLOWEEN_KART))
	{
		potential.clear();

		return;
	}

	const auto weapon{H::Entities->GetWeapon()};

	if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN)
	{
		potential.clear();

		return;
	}

	const auto medigun{weapon->As<C_WeaponMedigun>()};

	if (!medigun)
	{
		potential.clear();

		return;
	}

	auto isPlayerGood = [&](C_TFPlayer* pl)
	{
		if (!pl || pl->deadflag() || pl->GetCenter().DistTo(local->GetShootPos()) > 449.0f || pl->InCond(TF_COND_STEALTHED))
		{
			return false;
		}

		if (medigun->m_hHealingTarget().Get())
		{
			static C_BaseEntity* last{};

			if (medigun->m_bChargeRelease())
			{
				if (medigun->m_hHealingTarget().Get() == pl)
				{
					return false;
				}
			}

			else
			{
				if (pl->IsInvulnerable())
				{
					return false;
				}

				if (medigun->m_hHealingTarget().Get() == pl && last == pl)
				{
					return false;
				}

				auto mult{1.44f};

				if (medigun->GetMedigunType() == MEDIGUN_QUICKFIX)
				{
					mult = 1.24f;
				}

				if (static_cast<float>(pl->m_iHealth()) >= static_cast<float>(pl->GetMaxHealth()) * mult)
				{
					return false;
				}
			}

			last = medigun->m_hHealingTarget().Get();
		}

		CTraceFilterHitscan filter{};
		trace_t tr{};

		H::AimUtils->Trace(local->GetShootPos(), pl->GetCenter(), (MASK_SHOT & ~CONTENTS_HITBOX), &filter, &tr);

		return tr.fraction > 0.99f || tr.m_pEnt == pl;
	};

	if (potential.empty())
	{
		for (const auto ent : H::Entities->GetGroup(EEntGroup::PLAYERS_TEAMMATES))
		{
			if (!ent || ent == local)
			{
				continue;
			}

			auto pl{ent->As<C_TFPlayer>()};

			if (!isPlayerGood(pl))
			{
				continue;
			}

			potential.push_back(pl);
		}
	}

	if ((cmd->tick_count % 2) && G::bCanPrimaryAttack && !potential.empty())
	{
		for (int n{0}; n < static_cast<int>(potential.size()); n++)
		{
			const auto pl{potential[n]};

			if (!isPlayerGood(pl))
			{
				potential.erase(potential.begin() + n);

				continue;
			}

			const auto angle{Math::CalcAngle(local->GetShootPos(), pl->GetCenter())};

			H::AimUtils->FixMovement(cmd, angle);

			cmd->viewangles = angle;
			cmd->buttons |= IN_ATTACK;

			G::bPSilentAngles = true;

			if (CFG::Misc_Accuracy_Improvements)
			{
				cmd->tick_count = TIME_TO_TICKS(pl->m_flSimulationTime() + SDKUtils::GetLerp());
			}

			break;
		}
	}

	else
	{
		cmd->buttons &= ~IN_ATTACK;
	}
}

void CMisc::MovementLock(CUserCmd* cmd)
{
	static auto active{false};

	if (!CFG::Misc_Movement_Lock_Key)
	{
		active = false;

		return;
	}

	const auto local{H::Entities->GetLocal()};

	if (!local || (local->m_fFlags() & FL_ONGROUND))
	{
		active = false;

		return;
	}

	const auto& vel{local->m_vecVelocity()};
	static Vec3 lastTickVel{};

	static auto angles{cmd->viewangles};

	if (!active
		&& static_cast<int>(vel.x) == 0 && static_cast<int>(vel.y) == 0 && static_cast<int>(vel.z) == -6
		&& static_cast<int>(lastTickVel.x) == 0 && static_cast<int>(lastTickVel.y) == 0 && static_cast<int>(lastTickVel.z) == -6)
	{
		active = true;

		angles = cmd->viewangles;

		if (cmd->buttons & IN_MOVELEFT)
		{
			angles.y += 45.0f;
		}

		if (cmd->buttons & IN_MOVERIGHT)
		{
			angles.y -= 45.0f;
		}
	}

	lastTickVel = vel;

	if (active && static_cast<int>(vel.x) != 0 && static_cast<int>(vel.y) != 0 && static_cast<int>(vel.z) != -6)
	{
		active = false;
	}

	if (H::Input->IsPressed(CFG::Misc_Movement_Lock_Key) && !I::MatSystemSurface->IsCursorVisible() && !I::EngineVGui->IsGameUIVisible())
	{
		active = false;
	}

	if (!active)
	{
		return;
	}

	const auto angle = DEG2RAD((angles.y - cmd->viewangles.y) + 90.0f);

	cmd->forwardmove = sinf(angle) * 450.0f;
	cmd->sidemove = cosf(angle) * 450.0f;
}

void CMisc::MvmInstaRespawn()
{
	if (!H::Input->IsDown(CFG::Misc_MVM_Instant_Respawn_Key)
		|| I::EngineVGui->IsGameUIVisible()
		|| I::MatSystemSurface->IsCursorVisible()
		|| SDKUtils::BInEndOfMatch())
	{
		return;
	}

	auto* kv{new KeyValues("MVM_Revive_Response")};

	kv->SetInt("accepted", 1);

	I::EngineClient->ServerCmdKeyValues(kv);
}

void CMisc::RegionChanger()
{
	if (!CFG::Misc_Region_Changer)
		return;

	// Get the network channel info to check if we're connected
	auto pNetChannel = I::EngineClient->GetNetChannelInfo();
	if (!pNetChannel)
		return;

	// Check if matchmaking ConVar exists - if not, create it
	static ConVar* mm_dedicated_search_maxping = I::CVar->FindVar("mm_dedicated_search_maxping");
	static ConVar* tf_mm_strict = I::CVar->FindVar("tf_mm_strict");
	static ConVar* cl_cmdrate = I::CVar->FindVar("cl_cmdrate");
	static ConVar* cl_updaterate = I::CVar->FindVar("cl_updaterate");
	static ConVar* rate = I::CVar->FindVar("rate");
	
	if (!mm_dedicated_search_maxping || !tf_mm_strict || !cl_cmdrate || !cl_updaterate || !rate)
		return;

	// Store default network settings to restore when region forcing is disabled
	static int defaultMaxPing = mm_dedicated_search_maxping->GetInt();
	static bool defaultStrict = tf_mm_strict->GetBool();
	static int defaultCmdRate = cl_cmdrate->GetInt();
	static int defaultUpdateRate = cl_updaterate->GetInt();
	static int defaultRate = rate->GetInt();

	// Store last selected region to detect changes
	static int lastSelectedRegion = -1;
	
	// If region changer was disabled, restore default settings
	if (lastSelectedRegion != -1 && CFG::Misc_Region_Selected == 0)
	{
		mm_dedicated_search_maxping->SetValue(defaultMaxPing);
		tf_mm_strict->SetValue(defaultStrict);
		cl_cmdrate->SetValue(defaultCmdRate);
		cl_updaterate->SetValue(defaultUpdateRate);
		rate->SetValue(defaultRate);
		lastSelectedRegion = 0;
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Restored to default region\n");
		return;
	}

	// If current region is the same as last region, don't change anything
	if (lastSelectedRegion == CFG::Misc_Region_Selected)
		return;

	// Configure network settings based on selected region
	switch (CFG::Misc_Region_Selected)
	{
	case 1: // EU (Europe)
		mm_dedicated_search_maxping->SetValue(130);
		tf_mm_strict->SetValue(true);
		cl_cmdrate->SetValue(66);
		cl_updaterate->SetValue(66);
		rate->SetValue(60000);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Region set to Europe\n");
		break;

	case 2: // NA (North America)
		mm_dedicated_search_maxping->SetValue(100);
		tf_mm_strict->SetValue(true);
		cl_cmdrate->SetValue(66);
		cl_updaterate->SetValue(66);
		rate->SetValue(60000);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Region set to North America\n");
		break;

	case 3: // Asia
		mm_dedicated_search_maxping->SetValue(180);
		tf_mm_strict->SetValue(true);
		cl_cmdrate->SetValue(33);
		cl_updaterate->SetValue(33);
		rate->SetValue(35000);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Region set to Asia\n");
		break;

	case 4: // Australia
		mm_dedicated_search_maxping->SetValue(150);
		tf_mm_strict->SetValue(true);
		cl_cmdrate->SetValue(33);
		cl_updaterate->SetValue(33);
		rate->SetValue(35000);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Region set to Australia\n");
		break;

	case 5: // South America
		mm_dedicated_search_maxping->SetValue(170);
		tf_mm_strict->SetValue(true);
		cl_cmdrate->SetValue(33);
		cl_updaterate->SetValue(33);
		rate->SetValue(30000);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Region set to South America\n");
		break;

	default:
		// Default region - use system defaults
		mm_dedicated_search_maxping->SetValue(defaultMaxPing);
		tf_mm_strict->SetValue(defaultStrict);
		cl_cmdrate->SetValue(defaultCmdRate);
		cl_updaterate->SetValue(defaultUpdateRate);
		rate->SetValue(defaultRate);
		I::CVar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "[Region Changer] Using default region\n");
		break;
	}

	// Update last selected region
	lastSelectedRegion = CFG::Misc_Region_Selected;
}
