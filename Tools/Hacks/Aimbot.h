#pragma once
#include "../../stdafx.h"
#include "../Utils/Hitbox.h"
#include "../Menu/Menu.h"
#include "../Utils/Playerlist.h"
#include "../Utils/HitboxLists.h"

#define TIME_TO_TICKSS( dt )	( ( int )( 0.5f + ( float )( dt ) / Interfaces.pGlobalVars->interval_per_tick ) )
#define TICKS_TO_TIMEE( t ) ( Interfaces.pGlobalVars->interval_per_tick *( t ) )
#define LC_NONE				0
#define LC_ALIVE			(1<<0)
#define LC_ORIGIN_CHANGED	(1<<8)
#define LC_ANGLES_CHANGED	(1<<9)
#define LC_SIZE_CHANGED		(1<<10)
#define LC_ANIMATION_CHANGED (1<<11)
#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.
#ifndef RAD2DEG
#define RAD2DEG( x )  ( (float)(x) * (float)(180.f / M_PI_F) )
#endif

#ifndef DEG2RAD
#define DEG2RAD( x )  ( (float)(x) * (float)(M_PI_F / 180.f) )
#endif

typedef bool(__thiscall* IsPlayingTimeDemo_t)(void*); //hitboxes que o aimbot usa msm lul #dreamgay
vector<int> Hitboxes =
{
	(int)CSGOHitboxID::HITBOX_HEAD,
	(int)CSGOHitboxID::HITBOX_PELVIS,
	(int)CSGOHitboxID::HITBOX_CHEST,
	(int)CSGOHitboxID::HITBOX_THORAX,
	(int)CSGOHitboxID::HITBOX_CHEST,
	(int)CSGOHitboxID::HITBOX_NECK,
	(int)CSGOHitboxID::HITBOX_LEFT_UPPER_ARM,
	(int)CSGOHitboxID::HITBOX_RIGHT_UPPER_ARM,
	(int)CSGOHitboxID::HITBOX_LEFT_FOREARM,
	(int)CSGOHitboxID::HITBOX_RIGHT_FOREARM,
	(int)CSGOHitboxID::HITBOX_LEFT_CALF,
	(int)CSGOHitboxID::HITBOX_RIGHT_CALF,
	(int)CSGOHitboxID::HITBOX_LEFT_FOOT,
	(int)CSGOHitboxID::HITBOX_RIGHT_FOOT,
	(int)CSGOHitboxID::HITBOX_LEFT_HAND,
	(int)CSGOHitboxID::HITBOX_RIGHT_HAND,
};

void RotateMovement(CInput::CUserCmd* cmd, float yaw)
{
	Vector viewangles;
	Interfaces.pEngine->GetViewAngles(viewangles);

	float rotation = DEG2RAD(viewangles.y - yaw);

	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);

	float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
	float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);

	cmd->forwardmove = new_forwardmove;
	cmd->sidemove = new_sidemove;
}

class GOTV
{
public:
	class CHLTVFrame;

	class CHLTVDemoRecorder
	{
	public:
		char _pad[0x540];
		bool m_bIsRecording;
		int m_nFrameCount;
		float m_nStartTick;
		int m_SequenceInfo;
		int m_nDeltaTick;
		int m_nSignonTick;
		bf_write m_MessageData; // temp buffer for all network messages
	};

	class CHLTVServer
	{
	public:
		char _pad[0x5040];
		CHLTVDemoRecorder m_DemoRecorder;
	};
};

class Aimbot
{
private:
	struct Target_t
	{
		int OverrideHitbox;
		Vector aimspot;
		CBaseEntity* ent;
	};

	Vector TickPrediction(Vector AimPoint, CBaseEntity* pTarget)
	{
		return AimPoint + (pTarget->GetVecVelocity() * Interfaces.pGlobalVars->interval_per_tick);
	}
	
	Vector GetBestPoint(PlayerList::CPlayer* Player)
	{

		for (auto Pos : Player->ShootPos)
		{
			if (Pos != Vector(0, 0, 0))
			{
				Vector New = Pos + Player->box->points[1];
				Vector Aimangles;
				Misc::CalcAngle(Hacks.LocalPlayer->GetEyePosition(), New, Aimangles);

				float damage = Autowall::GetDamage(New);
				if (damage > Vars.Ragebot.Accuracy.Mindamage)
				{
					return New;
				}
			}
		}
		return Vector(0, 0, 0);
	}
	
	void HitboxScanning(PlayerList::CPlayer* Player)
	{
		Hitbox* box = Player->box;
		CTraceFilter filter;
		trace_t tr;
		Ray_t ray;
		int Dividens[3] = { (box->points[8].x - box->points[1].x) / 5 , (box->points[8].y - box->points[1].y) / 5, (box->points[8].z - box->points[1].z) / 5 };
		int Old = Player->ScannedNumber + 5;
		for (; Player->ScannedNumber < Old; Player->ScannedNumber++)
		{
			int x = Player->ScannedNumber;
			int y = 0;
			int z = 0;
			while (x >= 5)
			{
				x -= 5;
				y++;
			}
			while (y >= 5)
			{
				y -= 5;
				z++;
			}
			if (z >= 5)
			{
				Player->ScannedNumber = 0;
				break;
			}

			Vector Pos = Vector(box->points[1].x + (x * Dividens[0]), box->points[1].y + (y * Dividens[1]), box->points[1].z + (z * Dividens[2]));
			ray.Init(Pos, Pos);
			Interfaces.pTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);
			if (tr.m_pEnt != Player->entity)
				Player->ShootPos[Player->ScannedNumber] = Vector(0, 0, 0);
		}
	}

	void GetTargets(std::vector< CBaseEntity* >& possibletargets, std::vector< Target_t > possibleaimspots)
	{
		for (auto i = 0; i <= Interfaces.pEntList->GetHighestEntityIndex(); i++)
		{
			auto pEntity = static_cast<CBaseEntity*>(Interfaces.pEntList->GetClientEntity(i));
			if (pEntity == nullptr)
				continue;
			if (pEntity == Hacks.LocalPlayer)
				continue;
			if (!pEntity->isAlive())
				continue;
			if (!(pEntity->GetHealth() > 0))
				continue;
			if (pEntity->GetTeam() == Hacks.LocalPlayer->GetTeam())
				continue;
			if (pEntity->IsDormant())
				continue;
			PlayerList::CPlayer* Player = plist.FindPlayer(pEntity);
			Player->entity = pEntity;
			if (pEntity->HasGunGameImmunity())
				continue;
			player_info_t info;
			if (!(Interfaces.pEngine->GetPlayerInfo(pEntity->GetIndex(), &info)))
				continue;

			possibletargets.emplace_back(pEntity);
		}
	}

	void GetAimSpots(std::vector< Vector >& Targets, std::vector< Target_t >& possibleaimspots, std::vector< CBaseEntity* >& possibletargets)
	{
		if ((int)possibletargets.size())
		{
			for (CBaseEntity* pEntity : possibletargets)
			{
				Targets.emplace_back(pEntity->GetVecOrigin() - Hacks.LocalPlayer->GetVecOrigin());

				std::vector< int > iArray = { Hitboxes[(int)Vars.Ragebot.Hitscan.PriorityHitbox] };;

				float damage = 0;

				damage = ScanBoxList(pEntity, iArray, possibleaimspots);
				if (damage == 0)
				{
					int iSwitch = Vars.Ragebot.Hitscan.Multibox;
					int i;
					switch (iSwitch)
					{
					case 0: iArray = { iBoxListDefault[(int)Vars.Ragebot.Hitscan.PriorityHitbox] };
							break;
					case 1:
						iArray = iBoxListSmart;
						break;
					case 2:
						iArray = iBoxListMax;
						break;
					case 3:
						iArray = iBoxListBaim;
						break;
					case 4:
						iArray = iBoxListSmartBaim;
						break;
					}
				}
				ScanBoxList(pEntity, iArray, possibleaimspots);
			}
		}
	}

	static void NormalizeVectorr(Vector& vec)
	{
		for (int i = 0; i < 3; ++i)
		{
			while (vec[i] > 180.f)
				vec[i] -= 360.f;

			while (vec[i] < -180.f)
				vec[i] += 360.f;
		}
		vec[2] = 0.f;
	}

	float ScanBoxList(CBaseEntity* pEntity, std::vector< int > iArray, std::vector< Target_t > &possibleaimspots)
	{
		float flBestDamage = 0.f;

		if (iArray.size() > 0)
		{
			for (int i = 0; i < iArray.size(); i++)
			{
				Hitbox box;
				if (!box.GetHitbox(pEntity, iArray[i]))
					continue;

				Vector Aimspot;
				float flDamage = iArray[i] == Hitboxes[i] ? box.GetBestPoint(Aimspot) : box.ScanCenter(Aimspot);

				if (flDamage > Vars.Ragebot.Accuracy.Mindamage)
				{
					if (flDamage > flBestDamage)
					{
						flBestDamage = flDamage;
						if (Misc::GetNumberOfTicksChoked(pEntity) > 5 && Vars.Ragebot.LagFix)
						{
							Aimspot -= pEntity->GetAbsOrigin();
							Aimspot += pEntity->GetNetworkOrigin();
						}

						Target_t target;
						target.aimspot = Aimspot;
						target.ent = pEntity;
						possibleaimspots.emplace_back(target);

					}
				}
			}
		}

		return flBestDamage;
	}


	void GetAtTargetsSpot(std::vector< Vector >& Targets)
	{
		if (!(int)Targets.size())
			Target = Vector(0, 0, 0);
		else
		{
			Target = Vector(8128, 8128, 8128);
			for (Vector t : Targets)
			{
				if (t.Length() < Target.Length())
					Target = t;
			}
		}
	}

	template <typename t> t clamp(t value, t min, t max) {
		if (value > max) {
			return max;
		}
		if (value < min) {
			return min;
		}
		return value;
	}


	float LerpTime()
	{
		auto cl_updaterate = Interfaces.g_ICVars->FindVar("cl_updaterate");
		auto sv_minupdaterate = Interfaces.g_ICVars->FindVar("sv_minupdaterate");
		auto sv_maxupdaterate = Interfaces.g_ICVars->FindVar("sv_maxupdaterate");

		auto cl_interp = Interfaces.g_ICVars->FindVar("cl_interp");
		auto sv_client_min_interp_ratio = Interfaces.g_ICVars->FindVar("sv_client_min_interp_ratio");
		auto sv_client_max_interp_ratio = Interfaces.g_ICVars->FindVar("sv_client_max_interp_ratio");

		auto updateRate = cl_updaterate->GetFloat();
		auto interpRatio = cl_interp->GetFloat();
		auto minInterpRatio = sv_client_min_interp_ratio->GetFloat();
		auto maxInterpRatio = sv_client_max_interp_ratio->GetFloat();
		auto minUpdateRate = sv_minupdaterate->GetFloat();
		auto maxUpdateRate = sv_maxupdaterate->GetFloat();

		auto clampedUpdateRate = clamp(updateRate, minUpdateRate, maxUpdateRate);
		auto clampedInterpRatio = clamp(interpRatio, minInterpRatio, maxInterpRatio);

		auto lerp = clampedInterpRatio / clampedUpdateRate;

		if (lerp <= cl_interp->GetFloat())
			lerp = cl_interp->GetFloat();

		return lerp;
	}

	void inline SinCosNONSENSE(float radians, float* sine, float* cosine)
	{
		*sine = sin(radians);
		*cosine = cos(radians);
	}

	void VectorAnglesNONSENSE(const Vector& forward, Vector& angles)
	{
		float tmp, yaw, pitch;

		if (forward[1] == 0 && forward[0] == 0)
		{
			yaw = 0;
			if (forward[2] > 0)
				pitch = 270;
			else
				pitch = 90;
		}
		else
		{
			yaw = (atan2(forward[1], forward[0]) * 180 / M_PI);
			if (yaw < 0)
				yaw += 360;

			tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
			pitch = (atan2(-forward[2], tmp) * 180 / M_PI);
			if (pitch < 0)
				pitch += 360;
		}

		angles[0] = pitch;
		angles[1] = yaw;
		angles[2] = 0;
	}

	void AngleVectorsNONSENSE(const Vector& angles, Vector* forward, Vector* right, Vector* up)
	{
		float sr, sp, sy, cr, cp, cy;
		SinCosNONSENSE(DEG2RAD(angles[1]), &sy, &cy);
		SinCosNONSENSE(DEG2RAD(angles[0]), &sp, &cp);
		SinCosNONSENSE(DEG2RAD(angles[2]), &sr, &cr);

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}
		if (right)
		{
			right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
			right->y = (-1 * sr * sp * sy + -1 * cr * cy);
			right->z = -1 * sr * cp;
		}
		if (up)
		{
			up->x = (cr * sp * cy + -sr * -sy);
			up->y = (cr * sp * sy + -sr * cy);
			up->z = cr * cp;
		}
	}

	inline float NormalizePitchNONSENSE(float pitch)
	{
		while (pitch > 89.f)
			pitch -= 180.f;
		while (pitch < -89.f)
			pitch += 180.f;

		return pitch;
	}

	inline float NormalizeYawNONSENSE(float yaw)
	{
		if (yaw > 180)
			yaw -= (round(yaw / 360) * 360.f);
		else if (yaw < -180)
			yaw += (round(yaw / 360) * -360.f);

		return yaw;
	}

	inline Vector NormalizeAngleNONSENSE(Vector angle)
	{
		angle.x = NormalizePitchNONSENSE(angle.x);
		angle.y = NormalizeYawNONSENSE(angle.y);

		if ((angle.z > 50) || (angle.z < 50))
			angle.z = 0;

		return angle;
	}



	bool IsResolved(const unsigned short& resolve_type)
	{
		static const unsigned int RESOLVE_TYPE_NUM = 8;
		static const unsigned short RESOLVE_TYPE_OVERRIDE = 0b00000001,
			RESOLVE_TYPE_NO_FAKE = 0b00000010,
			RESOLVE_TYPE_LBY = 0b00000100,
			RESOLVE_TYPE_LBY_UPDATE = 0b00001000,
			RESOLVE_TYPE_PREDICTED_LBY_UPDATE = 0b00010000,
			RESOLVE_TYPE_LAST_MOVING_LBY = 0b00100000,
			RESOLVE_TYPE_NOT_BREAKING_LBY = 0b01000000,
			RESOLVE_TYPE_BRUTEFORCE = 0b10000000,
			RESOLVE_TYPE_LAST_MOVING_LBY_DELTA = 0b100000000,
			RESOLVE_TYPE_ANTI_FREESTANDING = 0b1000000000;

		if (resolve_type & RESOLVE_TYPE_NO_FAKE ||
			resolve_type & RESOLVE_TYPE_LBY_UPDATE ||
			resolve_type & RESOLVE_TYPE_PREDICTED_LBY_UPDATE)
			return true;

		return false;
	}
	bool Fire(CInput::CUserCmd* cmd, Vector vecCurPos, std::vector< Target_t >& possibleaimspots)
	{
		int closest = 8129;
		if ((!Vars.Ragebot.AutoFire && !(cmd->buttons & IN_ATTACK)) || !Vars.Ragebot.Enable)
			return true;
		Vector Aimangles;
		int originaltick = Hacks.CurrentCmd->tick_count;
		int distance = -1;
		static int Cycle = 0;
		bool shot = false;
		if (!Misc::bullettime())
			return true;
		int selection = Vars.Ragebot.TargetSelection;
		if (selection != 2) {
			for (int i = 0; i < possibleaimspots.size(); i++)
			{
				Target_t Aimspot = possibleaimspots[i];
				Misc::CalcAngle(vecCurPos, Aimspot.aimspot, Aimangles);
				distance = vecCurPos.DistTo(Aimspot.aimspot);
				if (distance < closest)
				{
					int TempTick = originaltick;


					if (Vars.Ragebot.Accuracy.PositionAdjustment)
						Hacks.CurrentCmd->tick_count = TIME_TO_TICKSS(Aimspot.ent->GetSimulationTime() + LerpTime());
					else
						Hacks.CurrentCmd->tick_count = TempTick;

					cmd->viewangles = Aimangles;
					pTarget = Aimspot.ent;
					cmd->buttons |= IN_ATTACK;
					Angles = cmd->viewangles;
					shot = true;
				}
			}
		}
		else if (possibleaimspots.size() > 0)
		{
			Cycle++;
			if (Cycle >= possibleaimspots.size())
				Cycle = 0;
			Target_t Aimspot = possibleaimspots[Cycle];
			Misc::CalcAngle(vecCurPos, Aimspot.aimspot, Aimangles);
			int TempTick = originaltick;

			if (Vars.Ragebot.Accuracy.PositionAdjustment)
				Hacks.CurrentCmd->tick_count = TIME_TO_TICKSS(Aimspot.ent->GetSimulationTime() + LerpTime());
			else
				Hacks.CurrentCmd->tick_count = TempTick;
			cmd->viewangles = Aimangles;
			Angles = cmd->viewangles;
			cmd->buttons |= IN_ATTACK;
			shot = true;
		}
		return false;
	}

	bool AntiGOTV(int tick)
	{
		return true;
	}

public:

		
			   
	class PlayerResolveRecord
	{
	public:
		PlayerResolveRecord()
		{
			resolve_type = 0;
			shots_missed_moving_lby = 0;
			shots_missed_moving_lby_delta = 0;

			last_balance_adjust_trigger_time = 0.f;
			last_moving_lby_delta = 0.f;
			last_time_moving = 0.f;
			last_time_down_pitch = 0.f;
			next_predicted_lby_update = 0.f;

			has_fake = false;
			is_dormant = false, is_last_moving_lby_delta_valid = false;
			is_last_moving_lby_valid = false, is_fakewalking = false;
			is_balance_adjust_triggered = false, is_balance_adjust_playing = false;
			did_lby_flick = false, did_predicted_lby_flick = false;

			for (int i = 0; i < RESOLVE_TYPE_NUM; i++)
			{
				shots_hit[i] = 0;
				shots_fired[i] = 0;
			}
		}

	public:
		struct AntiFreestandingRecord
		{
			int right_damage = 0, left_damage = 0;
			float right_fraction = 0.f, left_fraction = 0.f;
		};

	public:

		static const unsigned int RESOLVE_TYPE_NUM = 8;
		static const unsigned short RESOLVE_TYPE_OVERRIDE = 0b00000001,
			RESOLVE_TYPE_NO_FAKE = 0b00000010,
			RESOLVE_TYPE_LBY = 0b00000100,
			RESOLVE_TYPE_LBY_UPDATE = 0b00001000,
			RESOLVE_TYPE_PREDICTED_LBY_UPDATE = 0b00010000,
			RESOLVE_TYPE_LAST_MOVING_LBY = 0b00100000,
			RESOLVE_TYPE_NOT_BREAKING_LBY = 0b01000000,
			RESOLVE_TYPE_BRUTEFORCE = 0b10000000,
			RESOLVE_TYPE_LAST_MOVING_LBY_DELTA = 0b100000000,
			RESOLVE_TYPE_ANTI_FREESTANDING = 0b1000000000;

		CAnimationLayer anim_layers[15];
		AntiFreestandingRecord anti_freestanding_record;

		Vector resolved_angles, networked_angles;
		Vector velocity, origin;

		int shots_hit[RESOLVE_TYPE_NUM], shots_fired[RESOLVE_TYPE_NUM];
		int shots_missed_moving_lby, shots_missed_moving_lby_delta;
		unsigned short resolve_type;

		float lower_body_yaw;
		float last_moving_lby;
		float last_moving_lby_delta;
		float last_balance_adjust_trigger_time;
		float last_time_moving;
		float last_time_down_pitch;
		float next_predicted_lby_update;

		bool is_dormant;
		bool is_last_moving_lby_valid;
		bool is_fakewalking;
		bool is_last_moving_lby_delta_valid;
		bool is_balance_adjust_triggered, is_balance_adjust_playing;
		bool did_lby_flick, did_predicted_lby_flick;
		bool has_fake;
	};

	struct Backtracking_Record
	{
		Backtracking_Record() {};
		explicit Backtracking_Record(CBaseEntity*);

		Vector vec_origin;
		Vector eye_angles;
		Vector abs_angles;
		Vector render_angles;
		Vector bbmin;
		Vector bbmax;
		Vector velocity;

		Vector previous_vec_origin;

		float simulation_time;
		float previous_simulation_time;
		float curtime;
		float previous_curtime;
		bool is_exploiting;
		int flags;


		CBaseEntity* player;
		CAnimationLayer anim_layers[15];
		CBaseAnimState animstate;
		matrix3x4_t bone_matrix[128];
		float pose_params[24];

		PlayerResolveRecord resolve_record;
	};

	struct PlayerAimbotInfo
	{
		int head_damage = 0, hitscan_damage = 0, tick, extrapolted_ticks;
		Vector head_position, hitscan_position;
		Backtracking_Record backtrack_record;
	};

	void DisableInterpolation()
	{

		/*for (int i = 1; i < Interfaces.pGlobalVars->maxClients; i++)
		{
			CBaseEntity* pEntity = Interfaces.pEntList->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0)
				{
					if (i != Interfaces.pEngine->GetLocalPlayer())
					{
						VarMapping_t* map = pEntity->GetVarMap();
						if (!map) return;
						for (int i = 0; i < map->m_nInterpolatedEntries; i++)
						{
							VarMapEntry_t *e = &map->m_Entries[i];
							if (Vars.Misc.asdasad)
								e->m_bNeedsToInterpolate = false;
							else
								e->m_bNeedsToInterpolate = true;
						}

					}
				}
			}
		}*/
	}

	Vector Target;
	CBaseEntity* pTarget;
	Vector Angles;
	bool Aim(CInput::CUserCmd* cmd)
	{
		Angles = cmd->viewangles;
		Vector vecCurPos = Hacks.LocalPlayer->GetEyePosition();
		std::vector< Vector > Targets;
		std::vector< Target_t > possibleaimspots;
		std::vector< CBaseEntity* > possibletargets;
		GetTargets(possibletargets, possibleaimspots);
		GetAimSpots(Targets, possibleaimspots, possibletargets);
		GetAtTargetsSpot(Targets);
		if (Hacks.LocalWeapon->IsMiscWeapon())
			return false;
		if (Vars.Ragebot.AutoScope && Hacks.LocalWeapon->isSniper() && !Hacks.LocalPlayer->m_bIsScoped() && possibleaimspots.size() > 0)
		{
			if (Hacks.LocalWeapon->hitchance() < Vars.Ragebot.Accuracy.Hitchance)
					Hacks.CurrentCmd->buttons |= IN_ATTACK2;
			//return false;
		}
		if (!Hacks.LocalPlayer->isAlive())
			return false;




		for (int i = 0; i < 64; i++) {
			auto entity = Interfaces.pEntList->GetClientEntity(i);
			if (!entity)
				continue;

			int highest_damage = 0, best_tick, last_extrapolated_ticks;
			Backtracking_Record last_backtrack_record;
			CBaseEntity* best_entity = nullptr;
			bool is_hitscan = false;
			Vector best_position;







			auto FillInfo = [&is_hitscan, &best_position, &best_entity, &last_backtrack_record, &last_extrapolated_ticks, &highest_damage, &best_tick, entity, i, this](bool hitscan) -> void {
				PlayerAimbotInfo player_aimbot_info[64];
				is_hitscan = hitscan, best_tick = player_aimbot_info[i].tick;
				best_entity = entity, last_backtrack_record = player_aimbot_info[i].backtrack_record;
				last_extrapolated_ticks = player_aimbot_info[i].extrapolted_ticks;

				if (hitscan) {
					best_position = player_aimbot_info[i].hitscan_position;
					highest_damage = player_aimbot_info[i].hitscan_damage;
				}
				else {
					best_position = player_aimbot_info[i].head_position;
					highest_damage = player_aimbot_info[i].head_damage;
				}
			};

			/// if resolved go for head
			PlayerAimbotInfo player_aimbot_info[64];
			if (IsResolved(player_aimbot_info[i].backtrack_record.resolve_record.resolve_type)) {
				FillInfo(false);
				break;
			}



			if (Hacks.LocalWeapon->hitchance() < Vars.Ragebot.Accuracy.Hitchance)
				return false;

			if (Vars.Ragebot.Accuracy.DelayShot)
			{
				if (pTarget)
				{
					Vector old_origin[65];

					Vector cur_origin = pTarget->GetVecOrigin();
					Vector v = cur_origin - old_origin[pTarget->GetIndex()];

					bool breaks_lagcomp = v.LengthSqr() < 4096.f;

					if (breaks_lagcomp)
						return false;

					old_origin[pTarget->GetIndex()] = cur_origin;
				}
			}
			

			return !Fire(cmd, vecCurPos, possibleaimspots);
		}
	}
} Aimbot;

