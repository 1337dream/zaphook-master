#include "stdafx.h"
#include "Tools/Menu/Menu.h"
#include "Tools/Hacks/Misc.h"
#include "Tools/Utils/Playerlist.h"
#include "Tools/Hacks/Esp.h"
#include "Tools/Hacks/Antiaim.h"
#include "Tools/Hacks/Aimbot.h"
#include "Tools/Hacks/Legit.h"
#include "Tools/Menu/SettingsManager.h"
#include "Tools/Hacks/UI/UI.h"
#include "Tools/Hacks/KnifeBot.h"
#include "newgui.h"
#include "SDK/SDK Headers/CPred.h"
#define DEV

vfunc_hook hlclient_hook;
vfunc_hook direct3d_hook;
vfunc_hook vguipanel_hook;
vfunc_hook vguisurf_hook;
vfunc_hook mdlrender_hook;
vfunc_hook clientmode_hook;



std::vector<int> CPlayerList::friends = {};
std::vector<int> CPlayerList::Players = {};

#define PI_2 PI/2
#define PI_4 PI/4
CHackManager Hacks;


std::vector<trace_info> trace_logs;
class Hitbox;

static int iShoots;

std::vector< int > AutoStraferConstants = {};
std::vector< const char* > smoke_materials =
{
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
};




typedef void(*Fucntion)(IGameEvent* event);

void Hurtss(IGameEvent* event)
{
	int iAttacker = Interfaces.pEngine->GetPlayerForUserID(event->GetInt("attacker"));
	static int iVictim = Interfaces.pEngine->GetPlayerForUserID(event->GetInt("userid"));
	if (iAttacker == Interfaces.pEngine->GetLocalPlayer() && iVictim != Interfaces.pEngine->GetLocalPlayer())
	{
		Esp::SetHitmarkerValues(Interfaces.pGlobalVars->curtime, event->GetInt("dmg_health"), iVictim);

		Esp::CapsuleOverlay(iVictim, 0.8f);

		if (Vars.Visuals.Other.Hitmarker)
		{
			if (event->GetInt("hitgroup") == 1)
				Interfaces.pSurface->PlaySoundS("buttons\\arena_switch_press_02.wav");
			else
				Interfaces.pSurface->PlaySoundS("buttons\\arena_switch_press_02.wav");
		}
	}
}

void Impactss(IGameEvent* event)
{
	if (Vars.Visuals.Other.BulletTracers)
	{
		auto* index = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetPlayerForUserID(event->GetInt("userid")));

		if (Hacks.LocalPlayer)
		{
			Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

			if (index)
				trace_logs.push_back(trace_info(index->GetEyePosition(), position, Interfaces.pGlobalVars->curtime, event->GetInt("userid")));
		}
	}
}

class CGameEventListener : public IGameEventListener2
{
private:
	std::string eventName;
	Fucntion Call;
	bool server;
public:
	CGameEventListener() : eventName(""), Call(nullptr), server(false)
	{
	}

	CGameEventListener(const std::string& gameEventName, const Fucntion Func, bool Server) : eventName(gameEventName), Call(Func), server(Server)
	{
		this->RegListener();
	}

	virtual void FireGameEvent(IGameEvent* event)
	{
		this->Call(event);
	}

	virtual int IndicateEventHandling(void)
	{
		return 0x2A;
	}

	void RegListener(void)
	{
		static bool first = true;
		if (!first)
			return;
		if (Interfaces.g_GameEventMgr->AddListener(this, this->eventName.c_str(), server))
			Hacks.listeners.push_back(this);
	}
};

#define REG_EVENT_LISTENER(p, e, n, s) p = new CGameEventListener(n, e, s)
CGameEventListener* Hurt;
CGameEventListener* Impacts;

void Init()
{
	REG_EVENT_LISTENER(Hurt, &Hurtss, "player_hurt", false);
	REG_EVENT_LISTENER(Impacts, &Impactss, "bullet_impact", false);
}

void __stdcall Hooked_PaintTraverse(unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	auto oPaintTraverse = vguipanel_hook.get_original<iPaintTraverse>(41);

	if (strstr(Interfaces.pPanel->GetName(vguiPanel), ("HudZoom")) && Vars.Visuals.Effects.NoScope)
		return;

	oPaintTraverse(Interfaces.pPanel, vguiPanel, forceRepaint, allowForce);

	const char* pszPanelName = Interfaces.pPanel->GetName(vguiPanel);

	if (!strstr(pszPanelName, "FocusOverlayPanel"))
		return;

	if (Interfaces.pEngine->IsConnected() && Interfaces.pEngine->IsInGame())
	{
		auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());



		/*if (entity->GetFlags() & IN_JUMP) {
			static Vector pos = entity->GetVecOrigin();

			if (pos.LengthSqr() > 4096.f) {
				renderer::draw_text(bbox.x + bbox.w + 3, bbox.y + position_f, surface_fonts::esp_text, std::string("LC"), false, CColor(54, 237, 72, static_cast<int>(alpha[i])));
				position_f += 8;
			}

			pos = entity->GetVecOrigin();
		}
*/
		

		if (!(local_player->GetFlags() & FL_ONGROUND) && local_player->GetHealth() > 0)
		{
			static Vector pos = local_player->GetVecOrigin();

			if (pos.LengthSqr() > 4096.f) {
				int screen_width, screen_height;
				Interfaces.pEngine->GetScreenSize(screen_width, screen_height);
				Interfaces.pSurface->DrawT(0, screen_height - 590, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "LAG COMPENSATION");
			}

			pos = local_player->GetVecOrigin();
		}

		if (local_player && local_player->GetHealth() > 0 && Vars.Ragebot.Antiaim.Manual.dLeft != 0 && Vars.Ragebot.Antiaim.Manual.dRight != 0)
		{
			int screen_width, screen_height;
			Interfaces.pEngine->GetScreenSize(screen_width, screen_height);
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
			{
				Interfaces.pSurface->DrawT(0, screen_height - 550, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "DESYNC RIGHT");
			}
			else if (r)
			{
				Interfaces.pSurface->DrawT(0, screen_height - 550, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "DESYNC LEFT");
			}

		}
		if (local_player && local_player->GetHealth() > 0 && Vars.Ragebot.Antiaim.ManualAA != 0)
		{
			int screen_width, screen_height;
			Interfaces.pEngine->GetScreenSize(screen_width, screen_height);
			static bool o = true, k = false, b = false; // o = left // k = right // b = back 
			if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.left))
			{
				o = true;
				k = false;
				b = false;
			}
			if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.right))
			{
				o = false;
				k = true;
				b = false;
			}
			else if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Manual.back))
			{
				o = false;
				k = false;
				b = true;
			}

			if (o)
			{
				Interfaces.pSurface->DrawT(0, screen_height - 530, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "LEFT");
			}
			else if (k)
			{
				Interfaces.pSurface->DrawT(0, screen_height - 530, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "RIGHT");
			}
			else if (b)
			{
				Interfaces.pSurface->DrawT(0, screen_height - 530, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "BACK"); // eu so braindead e nao sei apaga o 
			}
		}
		if (local_player && local_player->GetHealth() > 0 && Vars.Ragebot.Antiaim.Freestanding != 0)
		{
			int screen_width, screen_height;
			Interfaces.pEngine->GetScreenSize(screen_width, screen_height);
			static bool o = true;
			if (GetAsyncKeyState(Vars.Ragebot.Antiaim.Freestanding))
			{
				o = true;

			}

			if (o)
			{
				Interfaces.pSurface->DrawT(0, screen_height - 570, CColor(108, 195, 18, 255), Hacks.Font_LBY, false, "FREESTANDING");
			}
		}
			
	if (Vars.Visuals.Other.showHits && Hacks.CurrentCmd->buttons & IN_ATTACK)
	{ 
		SpoofedConvar* type1 = new SpoofedConvar(Interfaces.g_ICVars->FindVar("sv_showimpacts"));
		SpoofedConvar* type2 = new SpoofedConvar(Interfaces.g_ICVars->FindVar("cl_weapon_debug_show_accuracy"));


		switch (Vars.Visuals.Other.showHits)
		{
		case 0:
			break;

		case 1: //boxes
			type1->SetInt(1);
			type2->SetInt(0);
			break;

		case 2: //onetap.su
			type1->SetInt(0);
			type2->SetInt(1);
			break;

		default:
			break;
		}
	}




		static bool first = false;
		if (!first)
		{
			ConVar* svcheats = Interfaces.g_ICVars->FindVar("sv_cheats");
			ConVar* dFeetShadow = Interfaces.g_ICVars->FindVar("cl_foot_contact_shadows");
			SpoofedConvar* svcheatsspoof = new SpoofedConvar(svcheats);
			SpoofedConvar* dFeetShadowSpoof = new SpoofedConvar(dFeetShadow);
			dFeetShadowSpoof->SetInt(0);
			svcheatsspoof->SetInt(1);
			first = true;
		}

		if (Hacks.LocalPlayer) {
			Misc::DrawScope();
			Esp::DoEsp();
			Esp::NightMode();
		}
	}
}

void OpenMenu()
{
	static bool is_down = false;
	static bool is_clicked = false;

	if (GetAsyncKeyState(VK_INSERT))
	{
		is_clicked = false;
		is_down = true;
	}
	else if (!GetAsyncKeyState(VK_INSERT) && is_down)
	{
		is_clicked = true;
		is_down = false;
	}
	else
	{
		is_clicked = false;
		is_down = false;
	}

	if (is_clicked)
	{
		Vars.Menu.Opened = !Vars.Menu.Opened;
	}
}

inline float NormalizePitch(float pitch)
{
	while (pitch > 89.f)
		pitch -= 180.f;
	while (pitch < -89.f)
		pitch += 180.f;

	return pitch;
}

inline float NormalizeYaw(float yaw)
{
	if (yaw > 180)
		yaw -= (round(yaw / 360) * 360.f);
	else if (yaw < -180)
		yaw += (round(yaw / 360) * -360.f);

	return yaw;
}

float GetCurtime()
{
	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player)
		return 0.f;

	return local_player->GetTickBase() * Interfaces.pGlobalVars->interval_per_tick;
}




//class PlayerResolveRecord
//{
//public:
//
//	Vector resolved_angles, networked_angles;
//	Vector velocity, origin;
//	CAnimationLayer anim_layers[15];
//	static const unsigned int RESOLVE_TYPE_NUM = 8;
//	static const unsigned short RESOLVE_TYPE_OVERRIDE = 0b00000001,
//		RESOLVE_TYPE_NO_FAKE = 0b00000010,
//		RESOLVE_TYPE_LBY = 0b00000100,
//		RESOLVE_TYPE_LBY_UPDATE = 0b00001000,
//		RESOLVE_TYPE_PREDICTED_LBY_UPDATE = 0b00010000,
//		RESOLVE_TYPE_LAST_MOVING_LBY = 0b00100000,
//		RESOLVE_TYPE_NOT_BREAKING_LBY = 0b01000000,
//		RESOLVE_TYPE_BRUTEFORCE = 0b10000000,
//		RESOLVE_TYPE_LAST_MOVING_LBY_DELTA = 0b100000000,
//		RESOLVE_TYPE_ANTI_FREESTANDING = 0b1000000000;
//
//	int shots_hit[RESOLVE_TYPE_NUM], shots_fired[RESOLVE_TYPE_NUM];
//	int shots_missed_moving_lby, shots_missed_moving_lby_delta;
//	unsigned short resolve_type;
//
//	float lower_body_yaw;
//	float last_moving_lby;
//	float last_moving_lby_delta;
//	float last_balance_adjust_trigger_time;
//	float last_time_moving;
//	float last_time_down_pitch;
//	float next_predicted_lby_update;
//
//	bool is_dormant;
//	bool is_last_moving_lby_valid;
//	bool is_fakewalking;
//	bool is_last_moving_lby_delta_valid;
//	bool is_balance_adjust_triggered, is_balance_adjust_playing;
//	bool did_lby_flick, did_predicted_lby_flick;
//	bool has_fake;
//
//
//
//
//	PlayerResolveRecord()
//	{
//		resolve_type = 0;
//		shots_missed_moving_lby = 0;
//		shots_missed_moving_lby_delta = 0;
//
//		last_balance_adjust_trigger_time = 0.f;
//		last_moving_lby_delta = 0.f;
//		last_time_moving = 0.f;
//		last_time_down_pitch = 0.f;
//		next_predicted_lby_update = 0.f;
//
//		has_fake = false;
//		is_dormant = false, is_last_moving_lby_delta_valid = false;
//		is_last_moving_lby_valid = false, is_fakewalking = false;
//		is_balance_adjust_triggered = false, is_balance_adjust_playing = false;
//		did_lby_flick = false, did_predicted_lby_flick = false;
//
//		struct AntiFreestandingRecord
//		{
//			int right_damage = 0, left_damage = 0;
//			float right_fraction = 0.f, left_fraction = 0.f;
//		};
//
//		for (int i = 0; i < RESOLVE_TYPE_NUM; i++)
//		{
//			shots_hit[i] = 0;
//			shots_fired[i] = 0;
//		}
//	}
//}


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

; void UpdateResolveRecord(CBaseEntity* entity) {
	/// a copy of the previous record for comparisons and shit
	PlayerResolveRecord player_resolve_records[64];
	const auto previous_record = player_resolve_records[entity->GetIndex()];

	auto& record = player_resolve_records[entity->GetIndex()];

	record.resolved_angles = record.networked_angles;
	record.velocity = entity->GetVecVelocity();
	record.origin = entity->GetVecOrigin();
	record.lower_body_yaw = entity->pelvisangs();
	record.is_dormant = entity->IsDormant();
	record.resolve_type = 0;

	record.is_balance_adjust_triggered = false, record.is_balance_adjust_playing = false;
	for (int i = 0; i < 15; i++) /// go through each animation layer
	{
		record.anim_layers[i] = entity->GetAnimOverlayy(i);


		if (record.is_dormant)
			record.next_predicted_lby_update = FLT_MAX;




		///// last moving lby, using a high velocity check ***just in case*** of fakewalkers
		//if (is_moving_on_ground && !record.is_fakewalking && record.velocity.Length2D() > 1.f && !record.is_dormant) {
		//	record.is_last_moving_lby_valid = true;
		//	record.is_last_moving_lby_delta_valid = false;
		//	record.shots_missed_moving_lby = 0;
		//	record.shots_missed_moving_lby_delta = 0;
		//	record.last_moving_lby = record.lower_body_yaw + 45;
		//	record.last_time_moving = GetCurtime();
		//}

		/// just came out of dormancy
		if (!record.is_dormant && previous_record.is_dormant) {
			/// if moved more than 32 units
			if ((record.origin - previous_record.origin).Length2D() > 16.f)
				record.is_last_moving_lby_valid = false;
		}

		/// get last moving lby delta
		if (!record.is_last_moving_lby_delta_valid && record.is_last_moving_lby_valid && record.velocity.Length2D() < 20 && fabs(GetCurtime() - record.last_time_moving) < 1.0) {

			record.is_last_moving_lby_delta_valid = true;
		}
	}

	if (NormalizePitch(record.networked_angles.x) > 5.f)
		record.last_time_down_pitch = GetCurtime();
}


; float GetLBYRotatedYaw(float lby, float yaw)
{
	float delta = NormalizeYaw(yaw - lby);
	if (fabs(delta) < 25.f)
		return lby;

	if (delta > 0.f)
		return yaw + 25.f;

	return yaw;
}

; int GetResolveTypeIndex(unsigned short resolve_type) {
	/// gonna have to use lorge if statements cuz fuck you

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

	if (resolve_type & RESOLVE_TYPE_OVERRIDE)
		return 0;
	else if (resolve_type & RESOLVE_TYPE_NO_FAKE)
		return 1;
	else if (resolve_type & RESOLVE_TYPE_LBY)
		return 2;
	else if (resolve_type & RESOLVE_TYPE_LBY_UPDATE)
		return 3;
	else if (resolve_type & RESOLVE_TYPE_PREDICTED_LBY_UPDATE)
		return 4;
	else if (resolve_type & RESOLVE_TYPE_LAST_MOVING_LBY)
		return 5;
	else if (resolve_type & RESOLVE_TYPE_NOT_BREAKING_LBY)
		return 6;
	else if (resolve_type & RESOLVE_TYPE_BRUTEFORCE)
		return 7;
	else if (resolve_type & RESOLVE_TYPE_LAST_MOVING_LBY_DELTA)
		return 8;
	else if (resolve_type & RESOLVE_TYPE_ANTI_FREESTANDING)
		return 9;

	return 0;
}

void ResolvePitch(CBaseEntity* entity) {
	PlayerResolveRecord player_resolve_records[64];
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
	auto& resolve_record = player_resolve_records[entity->GetIndex()];
}

; void ResolveYawBruteforce(CBaseEntity* entity) {
	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player)
		return;
	PlayerResolveRecord player_resolve_records[64];

	auto& resolve_record = player_resolve_records[entity->GetIndex()];
	static const unsigned short RESOLVE_TYPE_BRUTEFORCE = 0b10000000;
	resolve_record.resolve_type |= RESOLVE_TYPE_BRUTEFORCE;

	const float at_target_yaw = g_Math.CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y;

	const int shots_missed = resolve_record.shots_fired[GetResolveTypeIndex(resolve_record.resolve_type)] -
		resolve_record.shots_hit[GetResolveTypeIndex(resolve_record.resolve_type)];
	switch (shots_missed % 3) {
	case 0:
		resolve_record.resolved_angles.y = GetLBYRotatedYaw(entity->pelvisangs(), at_target_yaw + 60.f);
		break;
	case 1:
		resolve_record.resolved_angles.y = at_target_yaw + 140.f;
		break;
	case 2:
		resolve_record.resolved_angles.y = at_target_yaw - 75.f;
		break;
	}
}

; bool AntiFreestanding(CBaseEntity* entity, float& yaw) {
	PlayerResolveRecord player_resolve_records[64];

	const auto freestanding_record = player_resolve_records[entity->GetIndex()].anti_freestanding_record;

	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player)
		return false;

	if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
		return false;

	const float at_target_yaw = g_Math.CalcAngle(local_player->GetVecOrigin(), entity->GetVecOrigin()).y;
	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0) {
		if (freestanding_record.right_fraction < freestanding_record.left_fraction)
			yaw = at_target_yaw + 125.f;
		else
			yaw = at_target_yaw - 73.f;
	}
	else {
		if (freestanding_record.left_damage > freestanding_record.right_damage)
			yaw = at_target_yaw + 130.f;
		else
			yaw = at_target_yaw - 49.f;
	}

	return true;
}

; bool IsYawSideways(CBaseEntity* entity, float yaw) {
	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player)
		return false;

	const auto at_target_yaw = g_Math.CalcAngle(local_player->GetVecOrigin(), entity->GetVecOrigin()).y;
	const float delta = fabs(NormalizeYaw(at_target_yaw - yaw));

	return delta > 20.f && delta < 160.f;
}


//void resolve(CBaseEntity* entity)
//{
//	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
//	bool is_local_player = entity == local_player;
//	bool is_teammate = local_player->GetTeam() == entity->GetTeam() && !is_local_player;
//
//	if (!entity) return;
//	if (!local_player) return;
//
//	if (is_local_player) return;
//	if (is_teammate) return;
//	if (entity->GetHealth() <= 0) return;
//	if (local_player->GetHealth() <= 0) return;
//
//	if ((SETTINGS::settings.xuymethod == 1 && GetAsyncKeyState(UTILS::INPUT::input_handler.keyBindings(SETTINGS::settings.xuykey))) || (SETTINGS::settings.xuymethod == 0 && SETTINGS::settings.overridething))
//	{
//		Vector viewangles; Interfaces.pEngine->GetViewAngles(viewangles);
//		auto at_target_yaw = g_Math.CalcAngle(entity->GetVecOrigin(), local_player->GetVecOrigin()).y;
//
//		auto delta = NormalizeYaw(viewangles.y - at_target_yaw);
//		auto rightDelta = Vector(entity->GetEyeAngles().x, at_target_yaw + 90, entity->GetEyeAngles().z);
//		auto leftDelta = Vector(entity->GetEyeAngles().x, at_target_yaw - 90, entity->GetEyeAngles().z);
//
//		if (delta > 0)
//			entity->SetEyeAngles(rightDelta);
//		else
//			entity->SetEyeAngles(leftDelta);
//		return;
//	}
//	int
//	shots_missed[entity->GetIndex()] = shots_fired[entity->GetIndex()] - shots_hit[entity->GetIndex()];
//
//	int i = entity->GetIndex();
//
//	auto player_move = entity->GetVecVelocity().Length2D() > 36 && !entity->GetFlags() & FL_ONGROUND;
//	float player_lastmove[65], player_lastmove_active[65];
//	float bruteforce_angle[65];
//	player_lastmove_active[i] = false;
//
//	float kamaz_gay = 1337.f, r3d_pidr = 228.f, stef_eblan = 007.f,
//		igor_gay = 180.f, simvol_narik = 90.f, byeter_scam = -47.f,  // хромосомы бутера
//		storm_pussy_pupsik = 46.f, liston_aka_franta_top = -777.f; // тип float для тупых)
//
//	switch (shots_missed[i] % 8)
//	{
//	case 0:    bruteforce_angle[i] = simvol_narik; break;
//	case 1:    bruteforce_angle[i] = byeter_scam; break;
//	case 2:    bruteforce_angle[i] = storm_pussy_pupsik; break;
//	case 3:    bruteforce_angle[i] = igor_gay; break;
//	case 4:    bruteforce_angle[i] = liston_aka_franta_top; break;
//	case 5:    bruteforce_angle[i] = kamaz_gay; break;
//	case 6:    bruteforce_angle[i] = r3d_pidr; break;
//	case 7:    bruteforce_angle[i] = stef_eblan; break;
//	}
//
//	if (player_move)
//	{
//		entity->GetEyeAnglesPointer()->y = entity->pelvisangs();
//		player_lastmove[i] = entity->pelvisangs();
//		player_lastmove_active[i] = true;
//	}
//	else
//	{
//		if (player_lastmove_active[i])
//		{
//			if (shots_missed[i] <= 2) entity->GetEyeAnglesPointer()->y = player_lastmove[i];
//			else entity->GetEyeAnglesPointer()->y = player_lastmove[i] + bruteforce_angle[i];
//		}
//		else
//		{
//			entity->GetEyeAnglesPointer()->y = entity->pelvisangs();
//		}
//	}
//}

bool AntiFreestandingShit(CBaseEntity* entity, float& yaw) {
	PlayerResolveRecord player_resolve_records[64];
	const auto freestanding_record = player_resolve_records[entity->GetIndex()].anti_freestanding_record;

	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player)
		return false;

	if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
		return false;

	const float at_target_yaw = g_Math.CalcAngle(local_player->GetVecOrigin(), entity->GetVecOrigin()).y;
	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0) {
		if (freestanding_record.right_fraction < freestanding_record.left_fraction)
			yaw = at_target_yaw + 125.f;
		else
			yaw = at_target_yaw - 73.f;
	}
	else {
		if (freestanding_record.left_damage > freestanding_record.right_damage)
			yaw = at_target_yaw + 130.f;
		else
			yaw = at_target_yaw - 49.f;
	}

	return true;
}


; void ResolveYaw(CBaseEntity* entity) {


	for (int i = 0; i < 64; i++) {
		PlayerResolveRecord player_resolve_records[64];
		auto& resolve_record = player_resolve_records [ entity->GetIndex ( ) ];
		

	}
}

; void DoFSNRESOLVER() {
	auto local_player = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (!local_player || local_player->GetHealth() <= 0)
		return;
	PlayerResolveRecord player_resolve_records[64];

	for (int i = 0; i < 64; i++) {
		auto entity = Interfaces.pEntList->GetClientEntity(i);
		if (!entity || entity->GetHealth() <= 0 || entity->GetTeam() == local_player->GetTeam())
			continue;


		/// make sure to do dormant check AFTER calling UpdateResolveRecord()
		if (entity->IsDormant())
			continue;

		ResolveYaw ( entity );

	}
}

void GUI_Init(IDirect3DDevice9* pDevice)
{
	ImGui_ImplDX9_Init(INIT::Window, pDevice);


	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 14.f);
	io.Fonts->Build();

	ImGuiStyle &style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.01f, 0.01f, 0.01f, 0.60f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.01f, 0.01f, 0.01f, 0.92f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.94f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.10f, 0.40f, 0.10f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.58f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 0);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 0);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.54f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.52f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.11f, 0.31f, 0.11f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.11f, 0.41f, 0.11f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.11f, 0.51f, 0.11f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.04f, 0.44f, 0.04f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.47f, 0.18f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.42f, 0.12f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
	colors[ImGuiCol_CloseButton] = ImVec4(0.44f, 0.44f, 0.44f, 0);
	colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 0);
	colors[ImGuiCol_CloseButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 0);
	colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);

	style.Alpha = 1.0f;
	style.WindowPadding = ImVec2(8, 8);
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 1.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 0.0f;
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(7, 8);
	style.TouchExtraPadding = ImVec2(0, 0);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 3.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 4.0f;
	style.GrabMinSize = 1.0f;
	style.GrabRounding = 1.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4);
	style.AntiAliasedLines = true;
	style.CurveTessellationTol = 1.25f;

	Vars.Menu.Init = true;
}


long __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
{
	static auto oEndScene = direct3d_hook.get_original<EndSceneFn>(42);

	D3DCOLOR rectColor = D3DCOLOR_XRGB(255, 0, 0);
	D3DRECT BarRect = { 1, 1, 1, 1 };

	pDevice->Clear(1, &BarRect, D3DCLEAR_TARGET | D3DCLEAR_TARGET, rectColor, 0, 0);

	if (!Vars.Menu.Init)
		GUI_Init(pDevice);

	ImGui::GetIO().MouseDrawCursor = Vars.Menu.Opened;

	ImGui_ImplDX9_NewFrame();

	POINT mp;

	GetCursorPos(&mp);

	ImGuiIO& io = ImGui::GetIO();

	io.MousePos.x = mp.x;
	io.MousePos.y = mp.y;


	if (Vars.Menu.Opened)
	{
		ImGui::SetNextWindowSize(ImVec2(681, 329));

		int sX, sY;
		Interfaces.pEngine->GetScreenSize(sX, sY);

		int midX = sX / 2, midY = sY / 2;

		ImGuiStyle &style = ImGui::GetStyle();
		ImVec4* colors = ImGui::GetStyle().Colors;
		float r = Vars.Misc.bgColor[0], g = Vars.Misc.bgColor[1], b = Vars.Misc.bgColor[2], a = Vars.Misc.bgColor[3];
		colors[ImGuiCol_WindowBg] = ImVec4(r, g, b, a);
		ImGui::SetNextWindowPos(ImVec2(midX - (midX / 2), midY - (midY / 2)));

		if (ImGui::Begin("##zaphook beta", &Vars.Menu.Opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
		{
			ImGui::BeginGroup();
			ImGui::EndGroup();
		}
		ImGui::End();

		ImGui::SetNextWindowSize(ImVec2(677, 325));
		ImGui::SetNextWindowPos(ImVec2(midX - (midX / 2) + 2, midY - (midY / 2) + 2));

		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);

		if (ImGui::Begin("##usiaccmenu", &Vars.Menu.Opened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove))
		{
			drawMenu();
		}
		ImGui::End();

		
		

		



	}
	ImGui::Render();

	return oEndScene(pDevice);
}



long __stdcall Hooked_EndScene_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	auto oEndSceneReset = direct3d_hook.get_original<EndSceneResetFn>(16);

	if (!Vars.Menu.Init)
		return oEndSceneReset(pDevice, pPresentationParameters);

	ImGui_ImplDX9_InvalidateDeviceObjects();

	auto hr = oEndSceneReset(pDevice, pPresentationParameters);

	ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}

void InitAimbot()
{
	if (Hacks.LocalWeapon->HasAmmo())
	{
		Aimbot.Aim(Hacks.CurrentCmd);
		if (Misc::Shooting() && LocalInfo.Choked < 13)
		{
			if (!Vars.Ragebot.Silent && Vars.Ragebot.Enable)
				Interfaces.pEngine->SetViewAngles(Hacks.CurrentCmd->viewangles);
			if (Vars.Ragebot.Accuracy.RemoveRecoil)
				Hacks.CurrentCmd->viewangles -= LocalInfo.PunchAns * (vec_t)2.f;
			iShoots += 1;
		}
		else
		{
			if (Hacks.LocalWeapon->isPistolNotR8())
				Hacks.CurrentCmd->buttons &= ~IN_ATTACK;
		}
		if (*Hacks.LocalWeapon->GetItemDefinitionIndex() == WEAPON_REVOLVER && Vars.Ragebot.Enable)
		{
			if (Hacks.LocalWeapon->GetPostponeFireReadyTime() - Misc::GetServerTime() > 0.05)
				Hacks.CurrentCmd->buttons |= IN_ATTACK;
		}
	}
	//else
	//{
	//	Hacks.CurrentCmd->buttons &= ~IN_ATTACK;
	//	Hacks.CurrentCmd->buttons |= IN_RELOAD;
	//}

}

static void Resolver()
{
	if (Hacks.LocalPlayer && Hacks.LocalPlayer->isAlive())
	{
		for (auto i = 0; i < 65; ++i)
		{
			auto entity = Interfaces.pEntList->GetClientEntity(i);
			if (!entity
				|| !entity->IsValid(false))
				continue;


			static float lm[64];
			if (entity->GetVecVelocity().Length2D() > 0.1f) {
				entity->GetEyeAnglesPointer()->y = entity->pelvisangs();
				lm[entity->GetIndex()] = entity->pelvisangs();
			}
			else
				entity->GetEyeAnglesPointer()->y = lm[entity->GetIndex()];
		}
	}
}

class Prediction
{
private:
	float m_flOldCurtime;
	float m_flOldFrametime;
	CMoveData m_MoveData;

	int* m_pPredictionRandomSeed;
public:
	void Start()
	{
		static int flTickBase;

		if (Hacks.CurrentCmd)
		{
			if (Hacks.CurrentCmd->hasbeenpredicted)
				flTickBase = Hacks.LocalPlayer->GetTickBase();
			else
				++flTickBase;
		}

		static bool bInit = false;
		if (!bInit) {
			m_pPredictionRandomSeed = *(int**)(Utils.FindPattern("client_panorama.dll", "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
			bInit = true;
		}

		*m_pPredictionRandomSeed = MD5_PseudoRandom(Hacks.CurrentCmd->command_number) & 0x7FFFFFFF;

		m_flOldCurtime = Interfaces.pGlobalVars->curtime;
		m_flOldFrametime = Interfaces.pGlobalVars->frametime;

		Interfaces.pGlobalVars->curtime = flTickBase * Interfaces.pGlobalVars->interval_per_tick;
		Interfaces.pGlobalVars->frametime = Interfaces.pGlobalVars->interval_per_tick;

		Interfaces.g_pGameMovement->StartTrackPredictionErrors(Hacks.LocalPlayer);

		memset(&m_MoveData, 0, sizeof(m_MoveData));
		Interfaces.g_pMoveHelper->SetHost(Hacks.LocalPlayer);
		Interfaces.g_pPred->SetupMove(Hacks.LocalPlayer, Hacks.CurrentCmd, Interfaces.g_pMoveHelper, &m_MoveData);
		Interfaces.g_pGameMovement->ProcessMovement(Hacks.LocalPlayer, &m_MoveData);
		Interfaces.g_pPred->FinishMove(Hacks.LocalPlayer, Hacks.CurrentCmd, &m_MoveData);
	}

	void End()
	{
		Interfaces.g_pGameMovement->FinishTrackPredictionErrors(Hacks.LocalPlayer);
		Interfaces.g_pMoveHelper->SetHost(0);

		*m_pPredictionRandomSeed = -1;

		Interfaces.pGlobalVars->curtime = m_flOldCurtime;
		Interfaces.pGlobalVars->frametime = m_flOldFrametime;
	}
}Predict;


typedef void(__fastcall* ClanTagFn)(const char*, const char*);
ClanTagFn dw_ClanTag;
void SetClanTag(const char* tag, const char* name)
{

	if (!dw_ClanTag)
		dw_ClanTag = reinterpret_cast<ClanTagFn>(Utils.FindPattern("engine.dll", "53 56 57 8B DA 8B F9 FF 15"));

	if (dw_ClanTag)
		dw_ClanTag(tag, name);
}
void Clan_Tag()
{
	if (!Vars.Misc.clantag)
		return;
	int server_time = ((float)Interfaces.pGlobalVars->interval_per_tick * Hacks.LocalPlayer->GetTickBase());
	static int counter = 0;
	int value = server_time % 29;
	if (counter % 29 == 0)
		switch (value)
		{
		case 0: SetClanTag("                   ", "hozinum_csgo"); break;
		case 1: SetClanTag("                  z", "hozinum_csgo"); break;
		case 2: SetClanTag("                 za", "hozinum_csgo"); break;
		case 3: SetClanTag("                zap", "hozinum_csgo"); break;
		case 4: SetClanTag("               zaph", "hozinum_csgo"); break;
		case 5: SetClanTag("              zapho", "hozinum_csgo"); break;
		case 6: SetClanTag("             zaphoo", "hozinum_csgo"); break;
		case 7: SetClanTag("            zaphook", "hozinum_csgo"); break;
		case 8: SetClanTag("           zaphook.", "hozinum_csgo"); break;
		case 9: SetClanTag("          zaphook.c", "hozinum_csgo"); break;
		case 10:SetClanTag("         zaphook.cc", "hozinum_csgo"); break;
		case 11:SetClanTag("        zaphook.cc ", "hozinum_csgo"); break;
		case 12:SetClanTag("       zaphook.cc  ", "hozinum_csgo"); break;
		case 13:SetClanTag("      zaphook.cc   ", "hozinum_csgo"); break;
		case 14:SetClanTag("     zaphook.cc    ", "hozinum_csgo"); break;
		case 15:SetClanTag("    zaphook.cc     ", "hozinum_csgo"); break;
		case 16:SetClanTag("   zaphook.cc      ", "hozinum_csgo"); break;
		case 17:SetClanTag("  zaphook.cc       ", "hozinum_csgo"); break;
		case 18:SetClanTag(" zaphook.cc        ", "hozinum_csgo"); break;
		case 19:SetClanTag("zaphook.cc         ", "hozinum_csgo"); break;
		case 20:SetClanTag("aphook.cc	       ", "hozinum_csgo"); break;
		case 21:SetClanTag("phook.cc           ", "hozinum_csgo"); break;
		case 22:SetClanTag("hook.cc            ", "hozinum_csgo"); break;
		case 23:SetClanTag("ook.cc             ", "hozinum_csgo"); break;
		case 24:SetClanTag("ok.cc              ", "hozinum_csgo"); break;
		case 25:SetClanTag("k.cc               ", "hozinum_csgo"); break;
		case 26:SetClanTag(".cc                ", "hozinum_csgo"); break;
		case 27:SetClanTag("cc                 ", "hozinum_csgo"); break;
		case 28:SetClanTag("c                  ", "hozinum_csgo"); break;
		case 29:SetClanTag("                   ", "hozinum_csgo"); break;
			break;
		}
	counter++;

}


bool __fastcall Hooked_CreateMove(void* thisptr, void*, float flInputSampleTime, CInput::CUserCmd* cmd)
{
	if (!cmd || cmd->command_number == 0)
		return false;

	Hacks.CurrentCmd = cmd;
	Hacks.LocalPlayer = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());

	uintptr_t* FPointer; __asm { MOV FPointer, EBP }
	byte* SendPacket = (byte*)(*FPointer - 0x1C);

	Hacks.SendPacket = *SendPacket;

	AntiAims.ShouldAA = false;
	Aimbot.Target = Vector(0, 0, 0);

	Vars.Globals.CurMemeShots = iShoots;

	Hacks.LocalWeapon = Hacks.LocalPlayer->GetActiveBaseCombatWeapon();

	Misc::BunnyHop();
	Misc::AutoStrafe();

	KnifeBot::Run();

	Clan_Tag();

	if (Hacks.LocalPlayer && Hacks.LocalWeapon && Hacks.LocalPlayer->isAlive())
	{
		if ( Vars.Ragebot.Antiaim.InfinityDuck ) {
			Hacks.CurrentCmd->buttons |= IN_BULLRUSH;
		}
		

		Predict.Start();

		InitAimbot();

		//Aimbot.DisableInterpolation();

		if (!Misc::Shooting())
			AntiAims.Run();
		//yo man do u know how to make like a golden text in menu? (wdym) like skeet has golden "INFINITY DUCK" & shit (nah) OH, textColored could u do this 4 me
		//if (Vars.Misc.MM_Mode)
		//{
		//	Vector LastAngle;

		//	Vector AddAngs = Hacks.CurrentCmd->viewangles - LastAngle;
		//	if (AddAngs.Length2D() > 25.f)
		//	{
		//		Misc::Normalize(AddAngs);
		//		AddAngs *= 25;
		//		Hacks.CurrentCmd->viewangles = LastAngle + AddAngs;
		//		Misc::Normalize(Hacks.CurrentCmd->viewangles);
		//	}

		//	LastAngle = Hacks.CurrentCmd->viewangles;
		//}

		Predict.End();
	}



	Misc::ClampAngles();
	Misc::MoveFix();

	LocalInfo.Choked = Interfaces.pEngine->GetNetChannel()->m_nChokedPackets;

	*SendPacket = Hacks.SendPacket;

	return false;
}

int CBaseCombatWeapon::ScopeLevel()
{
	return *(int*)((DWORD)this + 0x3390);
}

void __stdcall Hooked_OverrideView(CViewSetup* pSetup)
{
	auto oOverrideView = clientmode_hook.get_original<iOverRideViewFn>(18);

	if (Interfaces.pEngine->IsInGame() && Interfaces.pEngine->IsConnected())
	{
		if (Hacks.LocalPlayer->isAlive())
		{
			if (Hacks.LocalPlayer->m_bIsScoped())
			{
				if (Hacks.LocalWeapon->ScopeLevel() != 2)
					pSetup->fov = 90 + Vars.Misc.FOV;
			}
			else
				pSetup->fov = 90 + Vars.Misc.FOV;
		}

		//pSetup->angles = Aimbot.Angles;
	}
	Hacks.FOV = pSetup->fov;
	oOverrideView(pSetup);
}

void __stdcall hk_lockcursor()
{
	auto oLockCursor = vguisurf_hook.get_original<LockCursor_t>(67);

	if (Vars.Menu.Opened)
	{
		Interfaces.pSurface->unlockcursor();
		return;
	}

	oLockCursor(Interfaces.pSurface);
}

void DrawBeam(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMTESLA;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
	beamInfo.m_nModelIndex = -1;
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 3.0f;
	beamInfo.m_flWidth = 2.0f;
	beamInfo.m_flEndWidth = 2.0f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 2.0f;
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	Beam_t* myBeam = Interfaces.ViewRenderBeams->CreateBeamPoints(beamInfo);
	if (myBeam)
		Interfaces.ViewRenderBeams->DrawBeam(myBeam);
}

void calculate_angle(Vector src, Vector dst, Vector &angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.0f;
	if (delta.x >= 0.0) angles.y += 180.0f;
}

bool CBaseEntity::IsValid(bool need_local)
{
	if (this == nullptr)
		return false;
	if (need_local)
	{
		if (this == Hacks.LocalPlayer)
			return false;
		if (this->GetTeam() == Hacks.LocalPlayer->GetTeam())
			return false;
	}
	if (!this->isAlive())
		return false;
	if (!(this->GetHealth() > 0))
		return false;
	if (this->HasGunGameImmunity())
		return false;

	return true;
}

int CBaseEntity::GetActivity(int sequence)
{
	auto hdr = Interfaces.g_pModelInfo->GetStudioModel(this->GetModel());

	if (!hdr)
		return -1;

	static DWORD fn = NULL;

	if (!fn)
		fn = (DWORD)Utils.FindPattern("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 8B F1 83");

	static auto GetSequenceActivity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(fn);

	return GetSequenceActivity(this, hdr, sequence);
}

void __stdcall Hooked_Frame_Stage_Notify(ClientFrameStage_t curStage)
{
	auto oFrameStageNotify = hlclient_hook.get_original<iFrameStageNotifyFn>(37);

	Hacks.LocalPlayer = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetLocalPlayer());
	if (Hacks.LocalPlayer)
	{
		LocalInfo.PunchAns = *Hacks.LocalPlayer->GetPunchAnglePtr();
		LocalInfo.Flags = Hacks.LocalPlayer->GetFlags();
	}
	Vector*pPunchAngle = nullptr, *pViewPunchAngle = nullptr, vPunchAngle, vViewPunchAngle;
	if (Interfaces.pEngine->IsInGame() && Interfaces.pEngine->IsConnected())
	{
		if (Hacks.LocalPlayer)
		{
			Hacks.LocalWeapon = Hacks.LocalPlayer->GetActiveBaseCombatWeapon();
			if (curStage == FRAME_RENDER_START)
			{
				if (Hacks.LocalPlayer && Hacks.LocalPlayer->isAlive())
				{
					Esp::ThirdPerson();
					Hacks.LocalPlayer->SetAngle2(Vector(0, Hacks.LocalPlayer->GetAnimState()->m_flGoalFeetYaw, 0));
					//Aimbot.DisableInterpolation();
				}

				Misc::PVSFix();

				if (Vars.Visuals.Effects.NoVisRecoil)
				{
					pPunchAngle = Hacks.LocalPlayer->GetPunchAnglePtr();
					pViewPunchAngle = Hacks.LocalPlayer->GetViewPunchAnglePtr();

					if (pPunchAngle && pViewPunchAngle)
					{
						vPunchAngle = *pPunchAngle;
						pPunchAngle->Init();
						vViewPunchAngle = *pViewPunchAngle;
						pViewPunchAngle->Init();
					}
				}
				for (auto matName : smoke_materials)
				{
					IMaterial* mat = Interfaces.pMaterialSystem->FindMaterial(matName, "Other textures");
					mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, Vars.Visuals.Effects.NoSmoke);
				}

			}

			if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			{

				Misc::AnimFix();

				if (Vars.Ragebot.Accuracy.Resolver.ResolveYaw) {
					DoFSNRESOLVER();
				}
			}

			if (curStage == FRAME_NET_UPDATE_START)
			{
				if (Interfaces.pEngine->IsInGame() && Interfaces.pEngine->IsConnected())
				{
					if (Vars.Visuals.Other.BulletTracers)
					{
						float Red, Green, Blue;
						Red = Vars.Visuals.Colors.BulletTracers[0] * 255;
						Green = Vars.Visuals.Colors.BulletTracers[1] * 255;
						Blue = Vars.Visuals.Colors.BulletTracers[2] * 255;
						for (unsigned int i = 0; i < trace_logs.size(); i++)
						{
							auto *shooter = Interfaces.pEntList->GetClientEntity(Interfaces.pEngine->GetPlayerForUserID(trace_logs[i].userid));
							if (!shooter)
								return;
							DrawBeam(trace_logs[i].start, trace_logs[i].position, Color(Red, Green, Blue, Vars.Visuals.Colors.BulletTracers[3] * 255));
							trace_logs.erase(trace_logs.begin() + i);
						}

					}

					/*if (Vars.Ragebot.Accuracy.AstroWalk)
					{
						CBaseEntity* local_player = Hacks.LocalPlayer;

						if (!(local_player))
							return;
						if (local_player->GetHealth() <= 0)
							return;

						Vector velocity = local_player->GetVecVelocity();
						Vector direction = local_player->GetEyeAngles();
						float speed = velocity.Length();

						direction.y = Hacks.CurrentCmd->viewangles.y - direction.y;

						Vector negated_direction = direction * -speed;
						if (!(GetAsyncKeyState(VK_SHIFT)))
							return;

						if (velocity.Length() >= .34f)
						{
							Hacks.CurrentCmd->forwardmove = negated_direction.x;
							Hacks.CurrentCmd->sidemove = negated_direction.y;


						}*/

					}
				}
			}

		}
	
	oFrameStageNotify(curStage);

	if (curStage == FRAME_RENDER_START && Hacks.LocalPlayer && Hacks.LocalPlayer->isAlive() && Vars.Ragebot.Antiaim.Yaw > 0)
	{
		Vector angs;
		Interfaces.pEngine->GetViewAngles(angs);
		*(Vector*)((DWORD)Hacks.LocalPlayer + 0x31D8) = angs;
	}

	if (pPunchAngle && pViewPunchAngle)
	{
		*pPunchAngle = vPunchAngle;
		*pViewPunchAngle = vViewPunchAngle;
	}
}


typedef void(__thiscall* orgGameFunc_InitKeyValues)(void* thisptr, const char* name);
orgGameFunc_InitKeyValues pInitKeyValues;
void InitKeyValues(KeyValues* pKeyValues, const char* name)
{
	auto dwInitKeyValues = Utils.FindPattern(XorStr("client_panorama.dll"), XorStr("8B 0E 33 4D FC 81 E1 ? ? ? ? 31 0E 88 46 03 C1 F8 08 66 89 46 12 8B C6")) - 0x45;
	//printf(XorStr("InitKeyValues: 0x%X\n"), (DWORD)offys.dwInitKeyValues);

	if (!pInitKeyValues)
		pInitKeyValues = reinterpret_cast<orgGameFunc_InitKeyValues>(dwInitKeyValues);

	pInitKeyValues(pKeyValues, name);
}


typedef void(__thiscall* orgGameFunc_LoadFromBuffer)(void* thisptr, const char* resourceName, const char* pBuffer, /*IBaseFileSystem**/void* pFileSystem, const char* pPathID, void* pfnEvaluateSymbolProc);
orgGameFunc_LoadFromBuffer pLoadFromBuffer;
void GameFunc_LoadFromBuffer(KeyValues* pKeyValues, const char* resourceName, const char* pBuffer, /*IBaseFileSystem**/void* pFileSystem = nullptr, const char* pPathID = NULL, void* pfnEvaluateSymbolProc = nullptr)
{
	/*55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04*/
	//Add Pattern Scan
	//static DWORD funcAddr = Utils::xFindPattern(client_dll, (PBYTE)"\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x34\x53\x8B\x5D\x0C\x89\x4C\x24\x04", "xxxxxxxxxxxxxxxxx");

	auto dwLoadFromBuffer = Utils.FindPattern(XorStr("client_panorama.dll"), XorStr("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04"));

	if (!pLoadFromBuffer)
		pLoadFromBuffer = reinterpret_cast<orgGameFunc_LoadFromBuffer>(dwLoadFromBuffer);

	pLoadFromBuffer(pKeyValues, resourceName, pBuffer, pFileSystem, pPathID, pfnEvaluateSymbolProc);


}


IMaterial* Create_Material(bool Ignore, bool Lit, bool Wireframe)
{
	static int iCreated = 0;

	static const char szTmp[] =
	{
		"\"%s\"\
		\n{\
		\n\t\"$basetexture\" \"vgui/white_additive\"\
		\n\t\"$envmap\" \"\"\
		\n\t\"$model\" \"1\"\
		\n\t\"$receiveflashlight\" \"1\"\
		\n\t\"$singlepassflashlight\" \"1\"\
		\n\t\"$flat\" \"1\"\
		\n\t\"$nocull\" \"0\"\
		\n\t\"$selfillum\" \"1\"\
		\n\t\"$halflambert\" \"1\"\
		\n\t\"$nofog\" \"0\"\
		\n\t\"$ignorez\" \"%i\"\
		\n\t\"$znearer\" \"0\"\
		\n\t\"$wireframe\" \"%i\"\
		\n}\n"
	};

	char szMaterial[512];
	char szBaseType[64];

	if (Lit)
	{
		static const char pszBaseType[] = "UnlitGeneric";
		sprintf_s(szMaterial, sizeof(szMaterial), szTmp, pszBaseType, (Ignore) ? 1 : 0, (Wireframe) ? 1 : 0);
		strcpy_s(szBaseType, pszBaseType);
	}
	else
	{
		static const char pszBaseType[] = "VertexLitGeneric";
		sprintf_s(szMaterial, sizeof(szMaterial), szTmp, pszBaseType, (Ignore) ? 1 : 0, (Wireframe) ? 1 : 0);
		strcpy_s(szBaseType, pszBaseType);
	}

	char szName[512];
	sprintf_s(szName, sizeof(szName), "custom_material_%i.vmt", iCreated);

	++iCreated;

	KeyValues* keyValues = new KeyValues();
	InitKeyValues(keyValues, szBaseType);
	GameFunc_LoadFromBuffer(keyValues, szName, szMaterial, nullptr, NULL, nullptr);

	IMaterial *createdMaterial = Interfaces.pMaterialSystem->CreateMaterial(szName, keyValues);
	createdMaterial->IncrementReferenceCount();

	return createdMaterial;
}

static IMaterial* FlatXQZ;
static IMaterial* FlatVisible;

static IMaterial* DefaultXQZ;
static IMaterial* DefaultVisible;

void SetupChamsMaterial()
{
	std::ofstream("csgo\\materials\\regular_ref.vmt") << R"#("VertexLitGeneric"
{
	    "$basetexture"				    "vgui/white_additive"
		"$ignorez"					    "0"
		"$envmap"						"env_cubemap"
        "$envmaptint"                   "[.10 .10 .10]"
		"$phong"						"1"
		"$phongexponent"				"10"
		"$phongboost"					"3.0"
		"$rimlight"					    "1"
		"$nofog"						"1"
		"$model"						"1"
		"$nocull"						"0"
        "$lightwarptexture"             "metalic"
		"$selfillum"					"1"
		"$halflambert"				    "1"
}
)#";
	std::ofstream("csgo\\materials\\ignorez_ref.vmt") << R"#("VertexLitGeneric"
{
	 "$basetexture"				    "vgui/white_additive"
		"$ignorez"					    "1"
		"$envmap"						"env_cubemap"
        "$envmaptint"                   "[.10 .10 .10]"
		"$phong"						"1"
		"$phongexponent"				"10"
		"$phongboost"					"3.0"
		"$rimlight"					    "1"
		"$nofog"						"1"
		"$model"						"1"
		"$nocull"						"0"
        "$lightwarptexture"             "metalic"
		"$selfillum"					"1"
		"$halflambert"				    "1"
}
)#";

	FlatXQZ = Create_Material(true, true, false);
	FlatVisible = Create_Material(false, true, false);

	DefaultXQZ = Create_Material(true, false, false);
	DefaultVisible = Create_Material(false, false, false);
}

#define TEXTURE_GROUP_MODEL							"Model textures"

bool Do_Chams(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t& pInfo, matrix3x4* pCustomBoneToWorld)
{
	/*static IMaterial* Covered_Lit;
	static IMaterial* Covered_Lit2;
	static IMaterial* Visable_Lit;
	static IMaterial* Visable_Lit2;
	static bool ff = false;
	if (!ff)
	{
		SetupChamsMaterial();
		ff = true;
	}

	switch (Vars.Visuals.Chams.Material)
	{
	case 0:
		Covered_Lit2 = FlatXQZ;
		Visable_Lit2 = FlatVisible;
		break;
	case 1:
		Covered_Lit = DefaultXQZ;
		Visable_Lit = DefaultVisible;
		Covered_Lit2 = DefaultXQZ;
		Visable_Lit2 = DefaultVisible;
		break;
	case 2:
		Covered_Lit = Interfaces.pMaterialSystem->FindMaterial("ignorez_ref", TEXTURE_GROUP_MODEL);
		Visable_Lit = Interfaces.pMaterialSystem->FindMaterial("regular_ref", TEXTURE_GROUP_MODEL);
		Covered_Lit2 = Interfaces.pMaterialSystem->FindMaterial("ignorez_ref", TEXTURE_GROUP_MODEL);
		Visable_Lit2 = Interfaces.pMaterialSystem->FindMaterial("regular_ref", TEXTURE_GROUP_MODEL);
		break;
	}

	CBaseEntity* Model_Entity = Interfaces.pEntList->GetClientEntity(pInfo.entity_index);
	auto Model_Name = Interfaces.g_pModelInfo->GetModelName(const_cast< model_t* >(pInfo.pModel));*/
	//if (Vars.Visuals.Chams.XQZ)
	//{
	//	if (Model_Entity->IsPlayer())
	//	{
	//		if (Model_Entity->GetTeam() == Hacks.LocalPlayer->GetTeam() && !Vars.Visuals.Chams.Team)
	//			return false;

	//		if (Model_Entity->GetTeam() == Hacks.LocalPlayer->GetTeam())
	//		{
	//			if (Model_Entity->HasGunGameImmunity())
	//				Covered_Lit->AlphaModulate(0.75f);
	//			else
	//				Covered_Lit->AlphaModulate(Vars.Visuals.Colors.Chams.TeamXQZ[3]);

	//			Visable_Lit->AlphaModulate(Vars.Visuals.Colors.Chams.Team[3]);

	//			Covered_Lit->ColorModulate(Vars.Visuals.Colors.Chams.TeamXQZ[0], Vars.Visuals.Colors.Chams.TeamXQZ[1], Vars.Visuals.Colors.Chams.TeamXQZ[2]); // t colour
	//			Visable_Lit->ColorModulate(Vars.Visuals.Colors.Chams.Team[0], Vars.Visuals.Colors.Chams.Team[1], Vars.Visuals.Colors.Chams.Team[2]); // t colour Visable
	//		}
	//		else
	//		{
	//			if (Model_Entity->HasGunGameImmunity())
	//				Covered_Lit->AlphaModulate(0.75f);
	//			else
	//				Covered_Lit->AlphaModulate(Vars.Visuals.Colors.Chams.XQZ[3]);

	//			Visable_Lit->AlphaModulate(Vars.Visuals.Colors.Chams.Visible[3]);

	//			Covered_Lit->ColorModulate(Vars.Visuals.Colors.Chams.XQZ[0], Vars.Visuals.Colors.Chams.XQZ[1], Vars.Visuals.Colors.Chams.XQZ[2]); // ct colour
	//			Visable_Lit->ColorModulate(Vars.Visuals.Colors.Chams.Visible[0], Vars.Visuals.Colors.Chams.Visible[1], Vars.Visuals.Colors.Chams.Visible[2]); // ct colour Visable
	//		}

	//		if (!Model_Entity->isAlive())
	//			return false;

	//		if (!Vars.Visuals.Chams.Visible)
	//		{
	//			Interfaces.g_pModelRender->ForcedMaterialOverride(Covered_Lit, OVERRIDE_NORMAL);
	//			auto oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	//		}

	//		Interfaces.g_pModelRender->ForcedMaterialOverride(Visable_Lit, OVERRIDE_NORMAL);
	//		auto oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

	//		return true;
	//	}
	//	if (Model_Entity->IsWeapon() && Vars.Visuals.Chams.Weapons)
	//	{
	//		Covered_Lit2->ColorModulate(Vars.Visuals.Colors.Chams.WeaponsXQZ[0], Vars.Visuals.Colors.Chams.WeaponsXQZ[1], Vars.Visuals.Colors.Chams.WeaponsXQZ[2]); // t colour
	//		Visable_Lit2->ColorModulate(Vars.Visuals.Colors.Chams.Weapons[0], Vars.Visuals.Colors.Chams.Weapons[1], Vars.Visuals.Colors.Chams.Weapons[2]); // t colour Visable
	//		Visable_Lit2->AlphaModulate(Vars.Visuals.Colors.Chams.WeaponsXQZ[3]);
	//		Covered_Lit2->AlphaModulate(Vars.Visuals.Colors.Chams.Weapons[3]);

	//		if (!Vars.Visuals.Chams.Visible)
	//		{
	//			Interfaces.g_pModelRender->ForcedMaterialOverride(Covered_Lit2, OVERRIDE_NORMAL);
	//			auto oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	//		}

	//		Interfaces.g_pModelRender->ForcedMaterialOverride(Visable_Lit2, OVERRIDE_NORMAL);
	//		auto oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);

	//		return true;
	//	}
	//}

	if (Vars.Visuals.Effects.NoFlash)
	{
		IMaterial* Flash = Interfaces.pMaterialSystem->FindMaterial("effects\\flashbang", "ClientEffect textures");
		IMaterial* FlashWhite = Interfaces.pMaterialSystem->FindMaterial("effects\\flashbang_white", "ClientEffect textures");
		Flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		FlashWhite->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		return false;
	}
	return false;
}


void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t& pInfo, matrix3x4* pCustomBoneToWorld)
{
	auto oDrawModelExecute = mdlrender_hook.get_original<IDrawModelExecute>(21);

	const char* ModelName = Interfaces.g_pModelInfo->GetModelName((model_t*)pInfo.pModel);
	if (Vars.Visuals.Chams.Wireframe_Hand)
	{
		static IMaterial* mat;

		static bool ff = false;
		if (!ff)
		{
			mat = Create_Material(true, false, true);
			ff = true;
		}

		if (strstr(ModelName, "arms"))
		{
			if (mat)
			{
				mat->ColorModulate(Vars.Visuals.Chams.Wireframe_Hand_Color[0] * 255, Vars.Visuals.Chams.Wireframe_Hand_Color[1] * 255, Vars.Visuals.Chams.Wireframe_Hand_Color[2] * 255);
				Interfaces.g_pModelRender->ForcedMaterialOverride(mat);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
		}
	}

	if (Vars.Visuals.Chams.Enable)
	{
		static IMaterial* matPlayer;
		static IMaterial* matPlayerXQZ;
		static bool ff = false;
		
			if (!ff)
			{
				matPlayer = Create_Material(false, false, false);
				matPlayerXQZ = Create_Material(true, false, false);
				ff = true;
			}

			float r = Vars.Visuals.Chams.enemy_Visible[0], g = Vars.Visuals.Chams.enemy_Visible[1], b = Vars.Visuals.Chams.enemy_Visible[2], a = Vars.Visuals.Chams.enemy_Visible[3];
			float Ir = Vars.Visuals.Chams.enemy_Invisible[0], Ig = Vars.Visuals.Chams.enemy_Invisible[1], Ib = Vars.Visuals.Chams.enemy_Invisible[2], Ia = Vars.Visuals.Chams.enemy_Invisible[3];

		if (strstr(ModelName, "models/player"))
		{

			CBaseEntity* e = Interfaces.pEntList->GetClientEntity(Interfaces.pEntList->GetMaxEntities());
			if (e == Hacks.LocalPlayer)
				return;

					if (Vars.Visuals.Chams.XQZ)
					{

						matPlayerXQZ->ColorModulate(r, g, b);
						matPlayerXQZ->AlphaModulate(a);
						Interfaces.g_pModelRender->ForcedMaterialOverride(matPlayerXQZ);
						
					} else {
					
						matPlayer->ColorModulate(r, g, b);
						matPlayer->AlphaModulate(a);
						Interfaces.g_pModelRender->ForcedMaterialOverride(matPlayer);
					}

		}
			//oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		
		
	}

	if (Vars.Visuals.Other.Nightmode)
	{
		if (strstr(ModelName, "models/props"))
		{
			Interfaces.g_pRenderView->SetBlend(0.5f);
		}
	}

	auto ent = Interfaces.pEntList->GetClientEntity(pInfo.entity_index);

	if (ent && ent->isAlive() && ent == Hacks.LocalPlayer && Hacks.LocalPlayer->m_bIsScoped())
	{
		Interfaces.g_pRenderView->SetBlend(0.65f);
	}

	if (!Do_Chams(thisptr, edx, ctx, state, pInfo, pCustomBoneToWorld))
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces.g_pModelRender->ForcedMaterialOverride(nullptr, OVERRIDE_NORMAL);

}

LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_LBUTTONDOWN:
		Hacks.PressedKeys[VK_LBUTTON] = true;
		break;
	case WM_LBUTTONUP:
		Hacks.PressedKeys[VK_LBUTTON] = false;
		break;
	case WM_RBUTTONDOWN:
		Hacks.PressedKeys[VK_RBUTTON] = true;
		break;
	case WM_RBUTTONUP:
		Hacks.PressedKeys[VK_RBUTTON] = false;
		break;
	case WM_MBUTTONDOWN:
		Hacks.PressedKeys[VK_MBUTTON] = true;
		break;
	case WM_MBUTTONUP:
		Hacks.PressedKeys[VK_MBUTTON] = false;
		break;
	case WM_XBUTTONDOWN:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			Hacks.PressedKeys[VK_XBUTTON1] = true;
		}
		else if (button == XBUTTON2)
		{
			Hacks.PressedKeys[VK_XBUTTON2] = true;
		}
		break;
	}
	case WM_XBUTTONUP:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
		{
			Hacks.PressedKeys[VK_XBUTTON1] = false;
		}
		else if (button == XBUTTON2)
		{
			Hacks.PressedKeys[VK_XBUTTON2] = false;
		}
		break;
	}
	case WM_KEYDOWN:
		Hacks.PressedKeys[wParam] = true;
		break;
	case WM_KEYUP:
		Hacks.PressedKeys[wParam] = false;
		break;
	default: break;
	}

	OpenMenu();

	if (Vars.Menu.Init && Vars.Menu.Opened && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(INIT::OldWindow, hWnd, uMsg, wParam, lParam);
}