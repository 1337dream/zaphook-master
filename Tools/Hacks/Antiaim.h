#pragma once
#include "stdafx.h"
#include "../Menu/Menu.h"
#include "Misc.h"
#include "Aimbot.h"
#include "../Utils/LocalInfo.h"
#include "../Menu/SettingsManager.h"
#include <chrono>
#include "../../SDK/SDK Headers/EngineClient.h"
#include "Aimbot.h"

#define TIME_TO_TICKS(dt) ((int)( 0.5f + (float)(dt) / Interfaces.pGlobalVars->interval_per_tick))
#define TICKS_TO_TIME(t) (Interfaces.pGlobalVars->interval_per_tick * (t) )

template <class T>
constexpr const T& Max(const T& x, const T& y)
{
	return (x < y) ? y : x;
}


float GetLBYRotatedYaw1(float lby, float yaw)
{
	float delta = Misc::Normalize_y(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

Vector TraceToEnd(Vector start, Vector end)
{
	trace_t trace;
	CTraceFilter filter;
	Ray_t ray;

	ray.Init(start, end);
	Interfaces.pTrace->TraceRay(ray, MASK_ALL, &filter, &trace);

	return trace.endpos;
}

class AntiAim
{
private:
	bool fakelagging = false;



	bool CanFireNext(CBaseEntity* pEntity)
	{
		CBaseCombatWeapon* pWeapon = pEntity->GetActiveBaseCombatWeapon();
		float flServerTime = pEntity->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
		float flNextShot = pWeapon->NextPrimaryAttack() - flServerTime;
		return flNextShot == 1;
	}

	void AtTargets(Vector& viewangles)
	{

		auto local_player = Hacks.LocalPlayer;

		if (!(local_player))
			return;
		if (local_player->GetHealth() <= 0)
			return;
		if (Aimbot.Target == Vector(0, 0, 0))
			return;

		if (Vars.Ragebot.Antiaim.AtTarget > 0)
		{
			if (Aimbot.Target != Vector(0, 0, 0))
			{
				Misc::CalcAngle(Hacks.LocalPlayer->GetVecOrigin(), Aimbot.Target + Hacks.LocalPlayer->GetVecOrigin(), viewangles);
			}
			else if (Vars.Ragebot.Antiaim.AtTarget == 2)
				ShouldAA = false;
		}
	}

	bool jitter = false;
	bool jitter2 = false;
	bool flips = false;
	void jittertime()
	{
		static clock_t start1_t1 = clock();
		double timeSoFar1 = (double)(clock() - start1_t1) / CLOCKS_PER_SEC;
		if (timeSoFar1 < 0.04)
			return;
		jitter = !jitter;
		jitter2 = !jitter2;
		flips = !flips;
		start1_t1 = clock();
	}

	void fliptime()
	{
		static clock_t start_t1 = clock();
		double timeSoFar = (double)(clock() - start_t1) / CLOCKS_PER_SEC;
		if (timeSoFar < 4.0)
			return;
		start_t1 = clock();
	}


	void doSlowWalk()
	{
		auto local_player = Hacks.LocalPlayer;

		if (!(local_player) || local_player->GetHealth() <= 0 || !(local_player->GetFlags() & FL_ONGROUND) || local_player->GetMoveType() == MOVETYPE_LADDER)
			return;

		if (!(GetAsyncKeyState(VK_SHIFT)))
			return;

		Hacks.CurrentCmd->forwardmove *= .06f;
		Hacks.CurrentCmd->sidemove *= .06f;
		Hacks.CurrentCmd->upmove = 0;

		/*float nextPrimaryAttack = Hacks.LocalWeapon->NextPrimaryAttack;

		if (nextPrimaryAttack > Interfaces.pGlobalVars->curtime)
			return;

		if (Aimbot.Target != Vector(0, 0, 0))
		{
			
		}*/
	}

	


	void Axis(Vector& View)
	{
		float Add = 0;
		static int SpinYaw = 0;
		float JitterAng = 0;
		static float StoredAng = 0;
		jittertime();
		fliptime();
		//static int jitter = 0;
		static int last = 0;
		int help = {};
		int Fake = rand() % 3;
		static int Spin[2] = { 0, 0 };
		for (int& s : Spin)
			if (s > 180)
				s -= 360;
			else if (s < -180)
				s += 360;
		int yaw = Vars.Ragebot.Antiaim.Yaw;

		auto local_player = Hacks.LocalPlayer;



		Vector viewangles;
		Interfaces.pEngine->GetViewAngles(viewangles);

		auto DoFreestanding = [local_player, viewangles, this](float& yaw, float& lby) -> bool {

			if (!Vars.Ragebot.Antiaim.Freestanding
				|| Vars.Ragebot.Antiaim.ManualAA)
				return false;

			static constexpr int damage_tolerance = 10;

			std::vector<CBaseEntity*> enemies;

			/// Find the lowest fov enemy
			CBaseEntity* closest_enemy = nullptr;
			float lowest_fov = 360.f;
			for (int i = 0; i < 64; i++) {
				auto entity = Interfaces.pEntList->GetClientEntity(i);
				if (!entity || entity->IsDormant() || entity->GetTeam() == local_player->GetTeam() || entity->GetHealth() <= 0)
					continue;

				const float current_fov = fabs(Misc::Normalize_y(g_Math.CalcAngle(local_player->GetVecOrigin(), entity->GetVecOrigin()).y - viewangles.y));
				if (current_fov < lowest_fov) {
					lowest_fov = current_fov;
					closest_enemy = entity;
				}

				enemies.push_back(entity);
			}

			/// No enemies
			if (closest_enemy == nullptr)
				return false;

			const float at_target_yaw = g_Math.CalcAngle(local_player->GetVecOrigin(), closest_enemy->GetVecOrigin()).y;
			const float right_yaw = at_target_yaw - 90.f;
			const float left_yaw = at_target_yaw + 90.f;

			/// Misc functions
			auto CalcDamage = [local_player, enemies](Vector point) -> int {
				int damage = 0;
				for (auto& enemy : enemies) {
					damage += Max(Autowall::GetDamageResolver(enemy->GetVecOrigin() + Vector(0, 0, 64.f), point),
						Autowall::GetDamageResolver(enemy->GetVecOrigin() + Vector(0, 0, 49.f), point));
				}

				return damage;
			};
			auto RotateAndExtendPosition = [](Vector position, float yaw, float distance) -> Vector {
				Vector direction;
				g_Math.angleVectors(Vector(0, yaw, 0), direction);

				return position + (direction * distance);
			};
			auto RotateLBYAndYaw = [right_yaw, left_yaw, local_player](int right_damage, int left_damage, float lby_delta, float& yaw, float& lby, bool prefect_angle = false, bool symetrical = true) -> bool {
				const bool prefer_right = right_damage < left_damage;

				yaw = prefer_right ? right_yaw : left_yaw;
				if (symetrical)
					lby_delta = lby_delta * (prefer_right ? -1.f : 1.f);

				/// If not moving
				if (prefect_angle) {
					if (local_player->GetVecVelocity().Length2D() < 0.1f)
						yaw = GetLBYRotatedYaw1(yaw + lby_delta, yaw);
					else
						yaw = GetLBYRotatedYaw1(local_player->pelvisangs(), yaw);
				}

				lby = yaw + lby_delta;

				if (fabs(Misc::Normalize_y(local_player->pelvisangs() - lby)) < 35.f) {
					yaw = local_player->pelvisangs() - lby_delta;
					lby = yaw + lby_delta;
				}

				return true;
			};
			auto DoBackFreestanding = [at_target_yaw, local_player](float& yaw, float& lby) -> bool {
				yaw = at_target_yaw + 180.f;
				lby = yaw;

				return true;
			};

			const auto head_position = local_player->GetVecOrigin() + Vector(0, 0, 74.f);
			const auto back_head_position = RotateAndExtendPosition(head_position, at_target_yaw + 180.f, 17.f);
			auto right_head_position = RotateAndExtendPosition(head_position, right_yaw, 17.f);
			auto left_head_position = RotateAndExtendPosition(head_position, left_yaw, 17.f);

			int right_damage = CalcDamage(right_head_position), left_damage = CalcDamage(left_head_position), back_damage = CalcDamage(back_head_position);

			/// too much damage to both sides
			if (right_damage > damage_tolerance && left_damage > damage_tolerance) {
				/// do backwards if valid
				if (back_damage < damage_tolerance)
					return DoBackFreestanding(yaw, lby);

				return false;
			}

			/// keep searching for a better angle
			if (right_damage == left_damage) {
				/// if on top of them, prioritise backwards antiaim
				if (g_Math.NormalizePitch(g_Math.CalcAngle(local_player->GetVecOrigin(), closest_enemy->GetVecOrigin()).x) > 15.f && back_damage < damage_tolerance)
					return DoBackFreestanding(yaw, lby);

				/// do some traces a bit further out
				right_head_position = RotateAndExtendPosition(head_position, right_yaw, 50.f);
				left_head_position = RotateAndExtendPosition(head_position, left_yaw, 50.f);

				right_damage = CalcDamage(right_head_position), left_damage = CalcDamage(left_head_position);
				if (right_damage == left_damage) {
					/// just return the side closest to a wall
					right_head_position = TraceToEnd(head_position, RotateAndExtendPosition(head_position, right_yaw, 17.f));
					left_head_position = TraceToEnd(head_position, RotateAndExtendPosition(head_position, left_yaw, 17.f));

					float distance_1, distance_2;
					trace_t trace;
					Ray_t ray;
					CTraceFilter filter;
					const auto end_pos = closest_enemy->GetVecOrigin() + Vector(0, 0, 64.f);

					/// right position
					ray.Init2(right_head_position, end_pos);
					Interfaces.pTrace->TraceRay(ray, MASK_ALL, &filter, &trace);
					distance_1 = (right_head_position - trace.endpos).Length();

					/// left position
					ray.Init2(left_head_position, end_pos);
					Interfaces.pTrace->TraceRay(ray, MASK_ALL, &filter, &trace);
					distance_2 = (left_head_position - trace.endpos).Length();

					if (fabs(distance_1 - distance_2) > 15.f) {
						RotateLBYAndYaw(distance_1, distance_2, 0, yaw, lby);

						return true;
					}

					return DoBackFreestanding(yaw, lby);
				}
				else {
					RotateLBYAndYaw(right_damage, left_damage, 0, yaw, lby);
					return true;
				}
			}
			else /// found an angle that does less damage than the other
			{
				/// if on top of them, prioritise backwards antiaim
				if (g_Math.NormalizePitch(g_Math.CalcAngle(local_player->GetVecOrigin(), closest_enemy->GetVecOrigin()).x) > 15.f && back_damage < damage_tolerance)
					return DoBackFreestanding(yaw, lby);

				const bool prefer_right = (right_damage < left_damage);
				const float lby_delta = 0;

				yaw = prefer_right ? right_yaw : left_yaw;
				lby = yaw + lby_delta;

				if (fabs(g_Math.NormalizeYaw(local_player->pelvisangs() - lby)) < 35.f) {
					yaw = local_player->pelvisangs() - lby_delta;
					lby = yaw + lby_delta;
				}

				if (CalcDamage(RotateAndExtendPosition(head_position, lby, 18.f)) > 0) {
					lby = yaw + (prefer_right ? -115.f : 115.f);
					if (CalcDamage(RotateAndExtendPosition(head_position, lby, 18.f)) > 0)
						lby = yaw;
				}
				else {
					/*	if ((is_moving && !UTILS::IsPressingMovementKeys(reinterpret_cast<CUserCmd*>(GLOBAL::last_cmd))) ||
							(UTILS::GetCurtime() - m_last_move_time < 0.22 && !is_moving))
							yaw = lby + 60.f;*/
				}

				return true;
			}

			return false;
		};
		float asdasda = 0;
		static bool s, f, g;
		if (!DoFreestanding(Add, asdasda))
		{
			if (Vars.Ragebot.Antiaim.ManualAA)
			{
				if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.left))
				{
					s = true;
					f = false;
					g = false;
				}
				else if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.right))
				{
					s = false;
					f = true;
					g = false;
				}
				else if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.back))
				{
					s = false;
					f = false;
					g = true;
				}

				if (s)
					Add -= 90;
				if (f)
					Add += 90;
				if (g) {}
			}




			switch (yaw)
			{
			case 1:
				Add += 180;
				break;
			case 2:
				Add += 0;
				break;
			case 3:
				Add = 180 + g_Math.RandomFloat(-45, 45);
				break;
			case 4: //razer desync
			{

				static bool r = true, l = false;
				if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.dLeft))
				{
					l = true;
					r = false;
				}
				else if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.dRight))
				{
					l = false;
					r = true;
				}

				if (l)
					Add += jitter ? -154 : -74; //old values -152 -20
				else if (r)
					Add += jitter ? 154 : 74; //old values 175 9

						
			}
			break;

			case 5: //zaphook desync
			{

				static bool r = true, l = false;
				if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.dLeft))
				{
					l = true;
					r = false;
				}
				else if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.dRight))
				{
					l = false;
					r = true;
				}

				if (l)
					Add += jitter ? -152 : -20; 
				else if (r)
					Add += jitter ? 175 : 9; 

			}
			break;

			case 6: //dream desync
			{
				Add += Interfaces.Client_State->chokedcommands == 0 ? -59 : 180; //pDesync (really)
			}
			break;

			}

			Hacks.CurrentCmd->viewangles.y = View.y + Add;


			int pitch = Vars.Ragebot.Antiaim.Pitch;
			switch (pitch)
			{
			case 1:
				Hacks.CurrentCmd->viewangles.x = 89.f; //down
				break;
			case 2:
				Hacks.CurrentCmd->viewangles.x = -89.f; //up
				break;
			case 3:
				Hacks.CurrentCmd->viewangles.x = Hacks.SendPacket ? 179.f : 181.f; //lag down
				break;
			case 4:
				Hacks.CurrentCmd->viewangles.x = Hacks.SendPacket ? -179.f : -181.f; //lag up
				break;
			}
		}
	}

public:
	bool ShouldAA = true;

	static void doAutoStop()
	{

		auto local_player = Hacks.LocalPlayer;

		if (!(local_player) || local_player->GetHealth() <= 0 || !(local_player->GetFlags() & FL_ONGROUND) || local_player->GetMoveType() == MOVETYPE_LADDER)
			return;

		float dmg = Vars.Ragebot.Accuracy.Mindamage;
		float curDmg = Autowall::GetDamageResolver(Aimbot.Target, local_player->GetEyePosition());

		int mode = Vars.Ragebot.Accuracy.autoStopMode;

		switch (mode)
		{
		case 0:
			break;

		case 1: //quick
			if (Hacks.LocalWeapon->NextPrimaryAttack() > Interfaces.pGlobalVars->curtime)
			{

				Hacks.CurrentCmd->forwardmove *= 0.2f;
				Hacks.CurrentCmd->sidemove *= 0.34f;
				Hacks.CurrentCmd->upmove = 0;

			}
			break;

		case 2: //full

			Hacks.CurrentCmd->forwardmove = 0;
			Hacks.CurrentCmd->sidemove = 0;
			Hacks.CurrentCmd->upmove = 0;

			break;

		case 3: //slide

			Hacks.CurrentCmd->forwardmove *= .06f;
			Hacks.CurrentCmd->sidemove *= .06f;
			Hacks.CurrentCmd->upmove = 0;


		default:
			break;
		}


		/*float nextPrimaryAttack = Hacks.LocalWeapon->NextPrimaryAttack;

		if (nextPrimaryAttack > Interfaces.pGlobalVars->curtime)
			return;

		if (Aimbot.Target != Vector(0, 0, 0))
		{

		}*/
	}

	void FakeLag()
	{
		if (Vars.Ragebot.Antiaim.Fakelag.Type <= 0
			/*|| FakeWalk()*/
			|| Hacks.CurrentCmd->buttons & IN_ATTACK) {
			fakelagging = false;
			return;
		}
		static Vector pos = Hacks.LocalPlayer->GetVecOrigin();

		switch (Vars.Ragebot.Antiaim.Fakelag.Type)
		{
		case 1:
		{
			Hacks.SendPacket = Interfaces.pEngine->GetNetChannel()->m_nChokedPackets >= 14;
			fakelagging = true;
		}
		break;
		case 2:
		{
			if (!((pos - LocalInfo.LastPos).Length2DSqr() > 4096.f) && !(LocalInfo.Choked > 12)) {
				Hacks.SendPacket = false;
			}
			fakelagging = true;
		}
		break;
		case 3:
		{
			static int choke = 14;
			if (choke > 14)
			{
				choke = 2;
			}
			static int timer = 0;
			timer++;
			if (timer > choke)
			{
				timer = 0;
				choke++;
			}
			Hacks.SendPacket = Interfaces.pEngine->GetNetChannel()->m_nChokedPackets >= min(14, choke);

			fakelagging = true;
		}
		break;
		default:
			fakelagging = false;
			break;
		}

		pos = Hacks.LocalPlayer->GetVecOrigin();
	}

	void Run()
	{
		if (!Vars.Ragebot.Antiaim.Enable)
			return;
		if (Hacks.CurrentCmd->buttons & IN_USE)
			return;
		if (!(Hacks.LocalPlayer->GetMoveType() == MOVETYPE_LADDER))
		{
		}else return;

		ShouldAA = true;
		AntiAims.FakeLag();

		if (!fakelagging )
			Hacks.SendPacket = jitter2;

		Vector View = Hacks.CurrentCmd->viewangles;
		AtTargets(View);
		if (ShouldAA)
			Axis(View);

		if (Vars.Ragebot.Accuracy.slowWalk)
			doSlowWalk();

	}
} AntiAims;