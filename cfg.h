#pragma once
#include <vector>
#include <string>
#include <minwindef.h>
#include "Vars.h"
using namespace std;

template <typename T>
class ConfigItem
{
	std::string category, name;
	T* value;
public:
	ConfigItem(std::string category, std::string name, T* value)
	{
		this->category = category;
		this->name = name;
		this->value = value;
	}
};

template <typename T>
class ConfigValue
{
public:
	ConfigValue(std::string category_, std::string name_, T* value_)
	{
		category = category_;
		name = name_;
		value = value_;
	}

	std::string category, name;
	T* value;
};

class CConfig
{
protected:
	std::vector<ConfigValue<int>*> ints;
	std::vector<ConfigValue<bool>*> bools;
	std::vector<ConfigValue<float>*> floats;
	std::vector<ConfigValue<float*>*> colors;
private:
	void SetupValue(int&, int, std::string, std::string);
	void SetupValue(bool&, bool, std::string, std::string);
	void SetupValue(float&, float, std::string, std::string);
	void SetupValue(float*&, float*, std::string, std::string);
	string GetModuleFilePath(HMODULE hModule);
	string GetModuleBaseDir(HMODULE hModule);

public:
	vector<string> ConfigList;
	void ReadConfigs(LPCTSTR lpszFileName);
	CConfig()
	{
		Setup();
	}

	void Setup()
	{
		SetupValue(Vars.Ragebot.Enable, FALSE, "RAGEBOT", "ENABLE");
		SetupValue(Vars.Ragebot.Silent, FALSE, "RAGEBOT", "SILENTAIM");
		SetupValue(Vars.Ragebot.AutoFire, FALSE, "RAGEBOT", "AUTOFIRE");
		SetupValue(Vars.Ragebot.AutoScope, FALSE, "RAGEBOT", "AUTOSCOPE");
		SetupValue(Vars.Ragebot.Accuracy.RemoveRecoil, FALSE, "RAGEBOT", "ACCURACY_NORECOIL");
		SetupValue(Vars.Ragebot.Accuracy.DelayShot, FALSE, "RAGEBOT", "ACCURACY_DELAYSHOT");
		SetupValue(Vars.Misc.Knifebot, FALSE, "RAGEBOT", "ACCURACY_KNIFEBOT");
		SetupValue(Vars.Ragebot.Accuracy.slowWalk, FALSE, "RAGEBOT", "MINIMAL_WALK");
		SetupValue(Vars.Ragebot.Accuracy.autoStopMode, 0, "RAGEBOT", "AUTOSTOP_MODE");
		SetupValue(Vars.Ragebot.Accuracy.Hitchance, 45, "RAGEBOT", "ACCURACY_HITCHANCE");
		SetupValue(Vars.Ragebot.Accuracy.Mindamage, 10, "RAGEBOT", "ACCURACY_MINDAMAGE");
		SetupValue(Vars.Ragebot.Accuracy.maxAimbotFoV, 0, "RAGEBOT", "ACCURACY_FOV");
		SetupValue(Vars.Ragebot.Accuracy.Resolver.ResolveYaw, FALSE, "RAGEBOT", "ACCURACY_RESOLVER_YAW");
		SetupValue(Vars.Ragebot.Accuracy.Resolver.ResolveYaw, FALSE, "RAGEBOT", "ACCURACY_RESOLVER_YAW");
		SetupValue(Vars.Ragebot.Hitscan.PriorityHitbox, NULL, "RAGEBOT", "HITSCAN_PRIORITYHITBOX");
		SetupValue(Vars.Ragebot.Hitscan.Multibox, 2, "RAGEBOT", "HITSCAN_MULTIBOX");
		SetupValue(Vars.Ragebot.bInterpLagComp, FALSE, "RAGEBOT", "DISABLEINTERP");
		SetupValue(Vars.Ragebot.Hitscan.Multipoint, FALSE, "RAGEBOT", "HITSCAN_MULTIPOINT");

		SetupValue(Vars.Ragebot.Antiaim.Enable, FALSE, "ANTIAIM", "ENABLE");
		SetupValue(Vars.Ragebot.Antiaim.Freestanding, FALSE, "ANTIAIM", "FRESTANDING");
		SetupValue(Vars.Ragebot.Antiaim.ManualAA, FALSE, "ANTIAIM", "MANUAL_ANTIAIM");
		
		SetupValue(Vars.Ragebot.Antiaim.Manual.dLeft, 0, "ANTIAIM", "DESYNC_BIND_L");
		SetupValue(Vars.Ragebot.Antiaim.Manual.dRight, 0, "ANTIAIM", "DESYNC_BIND_R");
		SetupValue(Vars.Ragebot.Antiaim.InfinityDuck, FALSE, "ANTIAIM", "INFINITY_DUCK");
		SetupValue(Vars.Ragebot.Antiaim.Manual.left, 0, "ANTIAIM", "MANUAL_LEFT");
		SetupValue(Vars.Ragebot.Antiaim.Manual.right, 0, "ANTIAIM", "MANUAL_RIGHT");
		SetupValue(Vars.Ragebot.Antiaim.Manual.back, 0, "ANTIAIM", "MANUAL_BACK");
		SetupValue(Vars.Ragebot.Antiaim.Pitch, 1, "ANTIAIM", "PITCH");
		SetupValue(Vars.Ragebot.Antiaim.Yaw, 4, "ANTIAIM", "YAW");
		SetupValue(Vars.Ragebot.Antiaim.Fakelag.Type, 0, "ANTIAIM", "FAKELAGS");
		SetupValue(Vars.Ragebot.Antiaim.AtTarget, 0, "ANTIAIM", "ARRATGET");

		SetupValue(Vars.Misc.AntiUT, TRUE, "MOVEMENT", "ANTI_UNTRUSTED");
		SetupValue(Vars.Misc.Bunnyhop, FALSE, "MOVEMENT", "BUNNYHOP");
		SetupValue(Vars.Misc.Strafers.Enable, FALSE, "MOVEMENT", "AUTOSTRAFE");

		SetupValue(Vars.Visuals.Player.Box, FALSE, "ESP", "BOX");
		SetupValue(Vars.Visuals.Colors.Box[0], 0.38f, "ESP", "BOX_COLOR_R");
		SetupValue(Vars.Visuals.Colors.Box[1], 0.95f, "ESP", "BOX_COLOR_G");
		SetupValue(Vars.Visuals.Colors.Box[2], 0.25f, "ESP", "BOX_COLOR_B");
		SetupValue(Vars.Visuals.Colors.Box[3], 1, "ESP", "BOX_COLOR_A");

		SetupValue(Vars.Visuals.Player.Name, FALSE, "ESP", "NAME");
		SetupValue(Vars.Visuals.Colors.esp_name_color[0], 1, "ESP", "NAME_COLOR_R");
		SetupValue(Vars.Visuals.Colors.esp_name_color[1], 1, "ESP", "NAME_COLOR_G");
		SetupValue(Vars.Visuals.Colors.esp_name_color[2], 1, "ESP", "NAME_COLOR_B");
		SetupValue(Vars.Visuals.Colors.esp_name_color[3], 1, "ESP", "NAME_COLOR_A");

		SetupValue(Vars.Visuals.Player.Weapon, FALSE, "ESP", "WEAPON");
		SetupValue(Vars.Visuals.Colors.esp_weapon_color[0], 1, "ESP", "WEAPON_COLOR_R");
		SetupValue(Vars.Visuals.Colors.esp_weapon_color[1], 1, "ESP", "WEAPON_COLOR_G");
		SetupValue(Vars.Visuals.Colors.esp_weapon_color[2], 1, "ESP", "WEAPON_COLOR_B");
		SetupValue(Vars.Visuals.Colors.esp_weapon_color[3], 1, "ESP", "WEAPON_COLOR_A");

		SetupValue(Vars.Visuals.Player.SnapLines, FALSE, "ESP", "SNAPLINES");
		SetupValue(Vars.Visuals.Colors.SnapLines[0], 1, "ESP", "SNAPLINES_COLOR_R");
		SetupValue(Vars.Visuals.Colors.SnapLines[1], 1, "ESP", "SNAPLINES_COLOR_G");
		SetupValue(Vars.Visuals.Colors.SnapLines[2], 1, "ESP", "SNAPLINES_COLOR_B");
		SetupValue(Vars.Visuals.Colors.SnapLines[3], 1, "ESP", "SNAPLINES_COLOR_A");

		SetupValue(Vars.Visuals.Chams.Wireframe_Hand, FALSE, "ESP", "CHAMS_HAND_WIREFRAME");
		SetupValue(Vars.Visuals.Chams.Wireframe_Hand_Color[0], 1, "ESP", "CHAMS_HAND_WIREFRAME_COLOR_R");
		SetupValue(Vars.Visuals.Chams.Wireframe_Hand_Color[1], 1, "ESP", "CHAMS_HAND_WIREFRAME_COLOR_G");
		SetupValue(Vars.Visuals.Chams.Wireframe_Hand_Color[2], 1, "ESP", "CHAMS_HAND_WIREFRAME_COLOR_B");
		SetupValue(Vars.Visuals.Chams.Wireframe_Hand_Color[3], 1, "ESP", "CHAMS_HAND_WIREFRAME_COLOR_A");

		SetupValue(Vars.Visuals.Chams.Enable, FALSE, "ESP", "CHAMS_GENERAL_ENABLE");
		SetupValue(Vars.Visuals.Chams.enemy_Visible[0], 0.24f, "ESP", "CHAMS_MODEL_COLOR_R");
		SetupValue(Vars.Visuals.Chams.enemy_Visible[1], 0.96f, "ESP", "CHAMS_MODEL_COLOR_G");
		SetupValue(Vars.Visuals.Chams.enemy_Visible[2], 0.24f, "ESP", "CHAMS_MODEL_COLOR_B");
		SetupValue(Vars.Visuals.Chams.enemy_Visible[3], 1.f, "ESP", "CHAMS_MODEL_COLOR_A");
		
		SetupValue(Vars.Misc.bgColor[0], 0.f, "MISC", "GUI_BACKGROUND_COLOR_R");
		SetupValue(Vars.Misc.bgColor[1], 1.f, "MISC", "GUI_BACKGROUND_COLOR_G");
		SetupValue(Vars.Misc.bgColor[2], 0.f, "MISC", "GUI_BACKGROUND_COLOR_B");
		SetupValue(Vars.Misc.bgColor[3], 0.94f, "MISC", "GUI_BACKGROUND_COLOR_A");

		SetupValue(Vars.Visuals.Player.Skeleton, FALSE, "ESP", "SKELETON");
		SetupValue(Vars.Visuals.Colors.Skeleton[0], 1, "ESP", "SKELETON_COLOR_R");
		SetupValue(Vars.Visuals.Colors.Skeleton[1], 1, "ESP", "SKELETON_COLOR_G");
		SetupValue(Vars.Visuals.Colors.Skeleton[2], 1, "ESP", "SKELETON_COLOR_B");
		SetupValue(Vars.Visuals.Colors.Skeleton[3], 1, "ESP", "SKELETON_COLOR_A");

		SetupValue(Vars.Visuals.Other.BulletTracers, FALSE, "ESP", "BULLETTRACER");
		SetupValue(Vars.Visuals.Colors.BulletTracers[0], 1, "ESP", "BULLETTRACER_COLOR_R");
		SetupValue(Vars.Visuals.Colors.BulletTracers[1], 0, "ESP", "BULLETTRACER_COLOR_G");
		SetupValue(Vars.Visuals.Colors.BulletTracers[2], 1, "ESP", "BULLETTRACER_COLOR_B");
		SetupValue(Vars.Visuals.Colors.BulletTracers[3], 1, "ESP", "BULLETTRACER_COLOR_A");

		SetupValue(Vars.Visuals.Other.FovArrows_Enable, FALSE, "ESP", "FOVARROWS");
		SetupValue(Vars.Visuals.Colors.FovArrows[0], 1, "ESP", "FOVARROWS_COLOR_R");
		SetupValue(Vars.Visuals.Colors.FovArrows[1], 0, "ESP", "FOVARROWS_COLOR_G");
		SetupValue(Vars.Visuals.Colors.FovArrows[2], 0, "ESP", "FOVARROWS_COLOR_B");
		SetupValue(Vars.Visuals.Colors.FovArrows[3], 1, "ESP", "FOVARROWS_COLOR_A");

		SetupValue(Vars.Visuals.Player.Health, FALSE, "ESP", "HEALTH");
		SetupValue(Vars.Visuals.Player.Weapon, TRUE, "ESP", "WEAPON");
		SetupValue(Vars.Visuals.Player.Armor, FALSE, "ESP", "ARMOR");
		SetupValue(Vars.Visuals.Player.Money, FALSE, "ESP", "MONEY");
		SetupValue(Vars.Visuals.Player.Flashed, FALSE, "ESP", "FLASHED");
		SetupValue(Vars.Visuals.Player.Zoom, FALSE, "ESP", "ZOOM");
		SetupValue(Vars.Visuals.Player.LastHittedhitbox, FALSE, "ESP", "HITBOX");

		SetupValue(Vars.Visuals.Other.ThirdPersonKey, 0, "ESP", "THIRDPERSON");
		SetupValue(Vars.Visuals.Other.showHits, 0, "ESP", "SHOWHITS");
		SetupValue(Vars.Visuals.Other.Nightmode, FALSE, "ESP", "NIGHTMODE");
		SetupValue(Vars.Visuals.Other.Radar, FALSE, "ESP", "RADAR");
		SetupValue(Vars.Visuals.C4, FALSE, "ESP", "BOMB");
		SetupValue(Vars.Visuals.Other.Hitmarker, FALSE, "ESP", "HITMARKER");
		SetupValue(Vars.Misc.FOV, 0, "ESP", "FOV");
		SetupValue(Vars.Visuals.DroppedWeapons, FALSE, "ESP", "DROPPEDWEAPONS");

		SetupValue(Vars.Misc.clantag, FALSE, "MISC", "CLANTAG");

		SetupValue(Vars.Visuals.Effects.NoVisRecoil, FALSE, "ESP", "NOVISRECOIL");
		SetupValue(Vars.Visuals.Effects.NoSmoke, FALSE, "ESP", "NOSMOKE");
		SetupValue(Vars.Visuals.Effects.NoScope, FALSE, "ESP", "NOSCOPE");
		SetupValue(Vars.Visuals.Effects.NoFlash, FALSE, "ESP", "NOFLASH");
	}

	void Save(string cfg_name);
	void Load(string cfg_name);
};

extern CConfig Config;