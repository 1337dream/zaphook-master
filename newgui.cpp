#include "newgui.h"
#include "stdafx.h"
#include <chrono>
#include "Utils/Utils.h"
#include "cfg.h"

#include "Hooks.h"
void Unhook();

const char* KeyStrings[] = {
	"",
	"Mouse 1",
	"Mouse 2",
	"Cancel",
	"Middle Mouse",
	"Mouse 4",
	"Mouse 5",
	"",
	"Backspace",
	"Tab",
	"",
	"",
	"Clear",
	"Enter",
	"",
	"",
	"Shift",
	"Control",
	"Alt",
	"Pause",
	"Caps",
	"",
	"",
	"",
	"",
	"",
	"",
	"Escape",
	"",
	"",
	"",
	"",
	"Space",
	"Page Up",
	"Page Down",
	"End",
	"Home",
	"Left",
	"Up",
	"Right",
	"Down",
	"",
	"",
	"",
	"Print",
	"Insert",
	"Delete",
	"",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"",
	"",
	"",
	"",
	"",
	"Numpad 0",
	"Numpad 1",
	"Numpad 2",
	"Numpad 3",
	"Numpad 4",
	"Numpad 5",
	"Numpad 6",
	"Numpad 7",
	"Numpad 8",
	"Numpad 9",
	"Multiply",
	"Add",
	"",
	"Subtract",
	"Decimal",
	"Divide",
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",

};


void CConfig::SetupValue(int &value, int def, std::string category, std::string name)
{
	value = def;
	ints.push_back(new ConfigValue<int>(category, name, &value));
}

void CConfig::SetupValue(float &value, float def, std::string category, std::string name)
{
	value = def;
	floats.push_back(new ConfigValue<float>(category, name, &value));
}

void CConfig::SetupValue(bool &value, bool def, std::string category, std::string name)
{
	value = def;
	bools.push_back(new ConfigValue<bool>(category, name, &value));
}
void CConfig::SetupValue(float* &value, float* def, std::string category, std::string name)
{
	value = def;
	colors.push_back(new ConfigValue<float*>(category, name, &value));
}

void CConfig::ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}

string CConfig::GetModuleFilePath(HMODULE hModule)
{
	string ModuleName = "";
	char szFileName[MAX_PATH] = { 0 };

	if (GetModuleFileNameA(hModule, szFileName, MAX_PATH))
		ModuleName = szFileName;

	return ModuleName;
}

string CConfig::GetModuleBaseDir(HMODULE hModule)
{
	string ModulePath = GetModuleFilePath(hModule);
	return ModulePath.substr(0, ModulePath.find_last_of("\\/"));
}

void CConfig::Save(string cfg_name)
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + ("\\zaphook\\");
		file = std::string(path) + ("\\zaphook\\" + cfg_name);
	}

	CreateDirectory(folder.c_str(), NULL);

	for (auto value : ints)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : floats)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());

	for (auto value : bools)
		WritePrivateProfileString(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
}

void CConfig::Load(string cfg_name)
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		folder = std::string(path) + ("\\zaphook\\");
		file = std::string(path) + ("\\zaphook\\" + cfg_name);
	}

	CreateDirectory(folder.c_str(), NULL);

	char value_l[32] = { '\0' };

	for (auto value : ints)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atoi(value_l);
	}

	for (auto value : floats)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = atof(value_l);
	}

	for (auto value : bools)
	{
		GetPrivateProfileString(value->category.c_str(), value->name.c_str(), "", value_l, 32, file.c_str());
		*value->value = !strcmp(value_l, "true");
	}
}

CConfig Config;

vector<string> ConfigList;
typedef void(*LPSEARCHFUNC)(LPCTSTR lpszFileName);

BOOL SearchFiles(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders = FALSE)
{
	LPTSTR part;
	char tmp[MAX_PATH];
	char name[MAX_PATH];

	HANDLE hSearch = NULL;
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

	if (bInnerFolders)
	{
		if (GetFullPathName(lpszFileName, MAX_PATH, tmp, &part) == 0) return FALSE;
		strcpy(name, part);
		strcpy(part, "*.*");
		wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		if (!((hSearch = FindFirstFile(tmp, &wfd)) == INVALID_HANDLE_VALUE))
			do
			{
				if (!strncmp(wfd.cFileName, ".", 1) || !strncmp(wfd.cFileName, "..", 2))
					continue;

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char next[MAX_PATH];
					if (GetFullPathName(lpszFileName, MAX_PATH, next, &part) == 0) return FALSE;
					strcpy(part, wfd.cFileName);
					strcat(next, "\\");
					strcat(next, name);

					SearchFiles(next, lpSearchFunc, TRUE);
				}
			} while (FindNextFile(hSearch, &wfd));
			FindClose(hSearch);
	}

	if ((hSearch = FindFirstFile(lpszFileName, &wfd)) == INVALID_HANDLE_VALUE)
		return TRUE;
	do
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file[MAX_PATH];
			if (GetFullPathName(lpszFileName, MAX_PATH, file, &part) == 0) return FALSE;
			strcpy(part, wfd.cFileName);

			lpSearchFunc(wfd.cFileName);
		}
	while (FindNextFile(hSearch, &wfd));
	FindClose(hSearch);
	return TRUE;
}

void ReadConfigs(LPCTSTR lpszFileName)
{
	ConfigList.push_back(lpszFileName);
}

void RefreshConfigs()
{
	static TCHAR path[MAX_PATH];
	std::string folder, file;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		ConfigList.clear();
		string ConfigDir = std::string(path) + "\\zaphook\\*";
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}
}


void drawMenu()
{
	static int tab_count = 0;

	static bool ragestring = false;
	static bool antiaimstring = false;
	static bool visualsstring = false;
	static bool miscstring = false;
	static bool cfgstring = false;
	static bool pliststring = false;

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);


	//-----TABS GROUP-----//

	static int page = 0;
	const char* tabs[] = { "ragebot", "antiaim", "visuals", "misc", "config" };

	ImGui::PushItemWidth(100);
	ImGui::ListBox("##tablist", &page, tabs, ARRAYSIZE(tabs), 16);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	//-----TABS CHILD GROUP-----//
	ImGui::BeginGroup();
	{
		ImGui::BeginChild("###tabschild", ImVec2(0, 292), true, ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::PushItemWidth(210);

			switch (page)
			{
			case 0:
			{
				ragestring = true;
				antiaimstring = false;
				visualsstring = false;
				miscstring = false;
				cfgstring = false;
				pliststring = false;

				ImGui::BeginGroup();
				{
					ImGui::Checkbox("enable", &Vars.Ragebot.Enable);
					ImGui::Checkbox("silent", &Vars.Ragebot.Silent);
					ImGui::Checkbox("auto fire", &Vars.Ragebot.AutoFire);
					ImGui::Checkbox("auto scope", &Vars.Ragebot.AutoScope);
					ImGui::Checkbox("multipoint", &Vars.Ragebot.Hitscan.Multipoint);
					static const char* Selection[] =
					{
						"distance",
						"cycle"
					};
					ImGui::Text("target selection");
					ImGui::Combo("##target_selection", &Vars.Ragebot.TargetSelection, Selection, ARRAYSIZE(Selection));

					static const char* Priority[] =
					{
						"head",
						"neck",
						"chest",
						"pelvis"
					};
					ImGui::Text("priority hitbox");
					ImGui::Combo("##priority_hitbox", &Vars.Ragebot.Hitscan.PriorityHitbox, Priority, ARRAYSIZE(Priority));

					static const char* Multibox[] =
					{
						"off",
						"half",
						"all",
						"baim",
						"smart baim",
					};
					ImGui::Text("multibox");
					ImGui::Combo("##multi_box", &Vars.Ragebot.Hitscan.Multibox, Multibox, ARRAYSIZE(Multibox));
				}
				ImGui::EndGroup();

				ImGui::SameLine();

				ImGui::BeginGroup();
				ImGui::Text("hitchance");

				ImGui::SliderFloat("###hitchance", &Vars.Ragebot.Accuracy.Hitchance, 0, 100, "%.1f");
				ImGui::Text("minimum damage");
				ImGui::SliderFloat("##mindamage", &Vars.Ragebot.Accuracy.Mindamage, 1, 100, "%.1f");

				ImGui::Checkbox("remove recoil", &Vars.Ragebot.Accuracy.RemoveRecoil);
				ImGui::Checkbox("extrapolation", &Vars.Ragebot.LagFix);
				ImGui::Checkbox("delay shot", &Vars.Ragebot.Accuracy.DelayShot);
				ImGui::Checkbox("slow walk", &Vars.Ragebot.Accuracy.slowWalk); 
				ImGui::Checkbox("resolver yaw", &Vars.Ragebot.Accuracy.Resolver.ResolveYaw);
				ImGui::Checkbox("aim history", &Vars.Ragebot.bInterpLagComp);
				//ImGui::SliderFloat("max. fov", &Vars.Ragebot.Accuracy.maxAimbotFoV, 0, 360, "%.1f");
				/*ImGui::Checkbox("auto stop", &Vars.Ragebot.Accuracy.autoStop);

				static const char* stopModes[]
				{
						"off",
						"quick",
						"full",
						"slide"
				};

				ImGui::Combo("##autostop", &Vars.Ragebot.Accuracy.autoStopMode, stopModes, ARRAYSIZE(stopModes)); */

				ImGui::EndGroup();
			}
			break;
			case 1:
			{
				ragestring = true;
				antiaimstring = false;
				visualsstring = false;
				miscstring = false;
				cfgstring = false;
				pliststring = false;
				ImGui::BeginGroup();
				auto &style = ImGui::GetStyle();
				ImGui::Checkbox("enable antiaim", &Vars.Ragebot.Antiaim.Enable);
				ImGui::Checkbox("freestanding", &Vars.Ragebot.Antiaim.Freestanding);
				ImGui::Checkbox("manual antiaim", &Vars.Ragebot.Antiaim.ManualAA);

				style.Colors[ImGuiCol_Text] = ImColor(255, 215, 0, 255);

				ImGui::Checkbox("infinity duck", &Vars.Ragebot.Antiaim.InfinityDuck);
				style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);

				if (Vars.Ragebot.Antiaim.ManualAA)
				{
					ImGui::Text("left angle");
					ImGui::Combo("##leftanglelolopaigay", &Vars.Ragebot.Antiaim.Manual.left, KeyStrings, ARRAYSIZE(KeyStrings));
					ImGui::Text("right angle");
					ImGui::Combo("##rightanglelolopaigay", &Vars.Ragebot.Antiaim.Manual.right, KeyStrings, ARRAYSIZE(KeyStrings));
					ImGui::Text("back angle");
					ImGui::Combo("##backanglelolopaigay", &Vars.Ragebot.Antiaim.Manual.back, KeyStrings, ARRAYSIZE(KeyStrings));
				}

				static const char* PitchAA[] =
				{
					"off",
					"down",
					"up",
					"lag down",
					"lag up"
				};
				ImGui::Text("pitch");
				ImGui::Combo("##PitchAA", &Vars.Ragebot.Antiaim.Pitch, PitchAA, ARRAYSIZE(PitchAA));

				static const char* YawAA[] =
				{
					"off",
					"180",
					"forwards",
					"180 jitter",
					"razer's desync",
					"zaphook's desync",
					"old desync"
				};

				ImGui::Text("yaw");
				ImGui::Combo("##YawAA", &Vars.Ragebot.Antiaim.Yaw, YawAA, ARRAYSIZE(YawAA));

				static const char* AtTargetAA[] =
				{
					"off",
					"default",
					"dormant check"
				};

				ImGui::Text("yaw base");
				ImGui::Combo("##yawBase", &Vars.Ragebot.Antiaim.AtTarget, AtTargetAA, ARRAYSIZE(AtTargetAA));


				static const char* FakelagType[] =
				{
					"off", "maximum", "adaptive", "switch"
				};
				ImGui::Text("fakelag");
				ImGui::Combo("##fakelagtype", &Vars.Ragebot.Antiaim.Fakelag.Type, FakelagType, ARRAYSIZE(FakelagType));

				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();

				ImGui::Text("desync right key");
				ImGui::Combo(XorStr("##dsyncKey"), &Vars.Ragebot.Antiaim.Manual.dLeft, KeyStrings, ARRAYSIZE(KeyStrings));
				ImGui::Text("desync left key");
				ImGui::Combo(XorStr("##dsyncKey2"), &Vars.Ragebot.Antiaim.Manual.dRight, KeyStrings, ARRAYSIZE(KeyStrings));

				ImGui::EndGroup();
			}
			break;
			case 2:
			{
				ragestring = false;
				antiaimstring = false;
				visualsstring = true;
				miscstring = false;
				cfgstring = false;
				pliststring = false;

				ImGui::BeginGroup();

				ImGui::Checkbox("bounding box", &Vars.Visuals.Player.Box);
				ImGui::SameLine(ImGui::GetWindowWidth() - 420);
				ImGui::ColorEdit4("###boxcolor", Vars.Visuals.Colors.Box);

				ImGui::Checkbox("show skeleton", &Vars.Visuals.Player.Skeleton);
				ImGui::SameLine(ImGui::GetWindowWidth() - 420);
				ImGui::ColorEdit4("###Skeletoncolor", Vars.Visuals.Colors.Skeleton);

				ImGui::Checkbox("out of fov arrows", &Vars.Visuals.Other.FovArrows_Enable);
				ImGui::SameLine(ImGui::GetWindowWidth() - 420);
				ImGui::ColorEdit4("###OutofFovColor", Vars.Visuals.Colors.FovArrows);

				ImGui::Checkbox("show health bar", &Vars.Visuals.Player.Health);

				ImGui::Checkbox("show name", &Vars.Visuals.Player.Name);
				ImGui::SameLine(ImGui::GetWindowWidth() - 420);
				ImGui::ColorEdit4("###namecolor", Vars.Visuals.Colors.esp_name_color);

				ImGui::Checkbox("show weapon", &Vars.Visuals.Player.Weapon);
				ImGui::SameLine(ImGui::GetWindowWidth() - 420);
				ImGui::ColorEdit4("##nothingispastedlolitssoeasytodoROFlcyamediabrololimagine", Vars.Visuals.Colors.esp_weapon_color);

				ImGui::Checkbox("show armor status", &Vars.Visuals.Player.Armor);
				ImGui::Checkbox("show hitted hitbox", &Vars.Visuals.Player.LastHittedhitbox);
				ImGui::Checkbox("show money", &Vars.Visuals.Player.Money);
				ImGui::Checkbox("show flashed", &Vars.Visuals.Player.Flashed);
				ImGui::Checkbox("show zoom", &Vars.Visuals.Player.Zoom);

				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();

				ImGui::Checkbox("style's hands", &Vars.Visuals.Chams.Wireframe_Hand);
				ImGui::SameLine();
				ImGui::ColorEdit4("###wireframehandcolor", Vars.Visuals.Chams.Wireframe_Hand_Color);
				ImGui::Text("model coloring");
				ImGui::Checkbox("enable", &Vars.Visuals.Chams.Enable);
				ImGui::ColorEdit4("model color", Vars.Visuals.Chams.enemy_Visible);
				ImGui::Checkbox("color through walls", &Vars.Visuals.Chams.XQZ);

				

				static const char* hitTypes[] =
				{
					"off",
					"boxes",
					"onetap.su"
				};

				ImGui::Text("show hits");
				ImGui::PushItemWidth(150.0f);
				ImGui::Combo("##showHits", &Vars.Visuals.Other.showHits, hitTypes, ARRAYSIZE(hitTypes));

				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();

				ImGui::Checkbox("nightmode", &Vars.Visuals.Other.Nightmode);
				ImGui::Checkbox("engine radar", &Vars.Visuals.Other.Radar);
				ImGui::Checkbox("show dropped weapons", &Vars.Visuals.DroppedWeapons);
				ImGui::Checkbox("show C4", &Vars.Visuals.C4);
				ImGui::Checkbox("hitmarker", &Vars.Visuals.Other.Hitmarker);

				ImGui::Spacing();
				ImGui::Text("removals");
				ImGui::Checkbox("visual recoil", &Vars.Visuals.Effects.NoVisRecoil);
				ImGui::Checkbox("flash", &Vars.Visuals.Effects.NoFlash);
				ImGui::Checkbox("smoke", &Vars.Visuals.Effects.NoSmoke);
				ImGui::Checkbox("scope", &Vars.Visuals.Effects.NoScope);

				ImGui::EndGroup();
			}
			break;
			case 3:
			{
				ragestring = false;
				antiaimstring = false;
				visualsstring = false;
				miscstring = true;
				cfgstring = false;
				pliststring = false;

				ImGui::BeginGroup();
				auto &style = ImGui::GetStyle();
				style.Colors[ImGuiCol_Text] = ImColor(255, 215, 0, 255);

				ImGui::Checkbox("anti untrusted", &Vars.Misc.AntiUT);
				style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
				ImGui::Checkbox("automatic jump", &Vars.Misc.Bunnyhop);
				ImGui::Checkbox("automatic strafe", &Vars.Misc.Strafers.Enable);
				if (Vars.Misc.Strafers.Enable)
					ImGui::Checkbox("wasd movement", &Vars.Misc.Strafers.WASD);
				ImGui::Checkbox("clantag spammer", &Vars.Misc.clantag);
				ImGui::Checkbox("knife bot", &Vars.Misc.Knifebot);
				ImGui::Text("thirdperson key");
				ImGui::PushItemWidth(150.0f);
				ImGui::Combo(XorStr("##tpkey"), &Vars.Visuals.Other.ThirdPersonKey, KeyStrings, ARRAYSIZE(KeyStrings));
				ImGui::SliderFloat("world FOV", &Vars.Misc.FOV, -50, 50, "%.1f");

				ImGui::EndGroup();
				ImGui::SameLine();
				ImGui::BeginGroup();

				ImGui::Checkbox("viewmodel changer", &Vars.Misc.vmBool);

				if (Vars.Misc.vmBool)
				{

				/*ImGui::SliderFloat("offset x", &Vars.Misc.vmOffsetX, -5, 5, "%.1f");
				ImGui::SliderFloat("offset y", &Vars.Misc.vmOffsetY, -5, 5, "%.1f");
				ImGui::SliderFloat("offset z", &Vars.Misc.vmOffsetZ, -5, 5, "%.1f");*/
					
					
				SpoofedConvar* minSpec = new SpoofedConvar(Interfaces.g_ICVars->FindVar("sv_competitive_minspec"));
				SpoofedConvar* bob = new SpoofedConvar(Interfaces.g_ICVars->FindVar("cl_bobcycle"));
				minSpec->SetInt(0);
				bob->SetFloat(0.98);

				ImGui::Text("Usage:");
				ImGui::Text("Use your console and change");
				ImGui::Text("your viewmodel there (p software lul)");
					
				}

				ImGui::Text("background color");
				ImGui::SameLine();
				ImGui::ColorEdit4("##bgcolor", Vars.Misc.bgColor);

			}
			break;
			case 4:
			{
				ragestring = false;
				antiaimstring = false;
				visualsstring = false;
				miscstring = false;
				cfgstring = true;
				pliststring = false;

				static bool f = true;
				if (!f)
				{
					RefreshConfigs();
					f = false;
				}

				ImGui::PushItemWidth(175.f);
				static int iConfigSelect = 0;
				static char ConfigName[64] = { 0 };
				if (ConfigList.size() > 0)
				{
					ImGui::Text("Configs: %s", ConfigList[iConfigSelect].c_str());
				}

				if (ImGui::Button("save"))
				{
					Config.Save(ConfigList[iConfigSelect].c_str());
				}
				ImGui::SameLine();

				if (ImGui::Button("load"))
				{
					Config.Load(ConfigList[iConfigSelect].c_str());
				}
				ImGui::SameLine();

				if (ImGui::Button("create"))
				{
					Config.Save(ConfigName);
					RefreshConfigs();
				}
				ImGui::SameLine();

				if (ImGui::Button("refresh"))
				{
					RefreshConfigs();
				}
				ImGui::InputText("config name", ConfigName, 64);

				ImGui::Text("list of configs");

				if (ConfigList.size() > 0)
				{
					for (int g = 0; g < ConfigList.size(); g++)
					{
						if (iConfigSelect == g)
							ImGui::GetStyle().Colors[ImGuiCol_Button] = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]; //main
						else
							ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f); //main
						if (ImGui::Button(ConfigList[g].c_str()))
							iConfigSelect = g;

					}
					ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f); //main
				}

				if (ImGui::Button("unload cheat"))
				{
					std::vector<IGameEventListener2*> listeners = {};
					hlclient_hook.unhook_all();
					direct3d_hook.unhook_all();
					vguipanel_hook.unhook_all();
					vguisurf_hook.unhook_all();
					mdlrender_hook.unhook_all();
					clientmode_hook.unhook_all();
					for (auto listener : listeners)
						Interfaces.g_GameEventMgr->RemoveListener(listener);
				}


			}
			break;
			}

			ImGui::PopItemWidth();
		}
		ImGui::EndChild();
	}
	ImGui::EndGroup();
}
