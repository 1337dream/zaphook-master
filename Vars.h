#pragma once
#include <string>
#include <vector>
struct Variables
{



	bool Save(std::string file_name);
	bool Load(std::string file_name);
	void CreateConfig(std::string name);
	void Delete(std::string name);
	std::vector<std::string> GetConfigs();



	struct
	{
		bool Enable;
		bool Silent;
		bool LagFix;
		bool bInterpLagComp;
		bool AutoFire;
		bool AutoScope;
		int TargetSelection;
		struct
		{
			float Hitchance;
			float Mindamage;
			bool PositionAdjustment;
			bool DelayShot;
			bool AstroWalk;
			float maxAimbotFoV;
			bool RemoveRecoil;
			int aimbot_accuracy_type = 0;
			float aimbot_accuracy = 0.f;
			bool autoStop;
			int autoStopMode;
			bool slowWalk;
			struct
			{
				bool Pitch;
				bool ResolveYaw;
				int Override;
			}Resolver;
		}Accuracy;
		struct
		{
			bool Multipoint;
			float PointScale = 70.f;
			int PriorityHitbox;
			int Multibox;
			int Bodyaim;
		}Hitscan;
		struct
		{
			bool Enable;

			int Pitch;
			int Yaw;
			int AtTarget;
			bool antiaim_edging_standing_enabled = false;
			bool antiaim_edging_moving_enabled = false;
			bool antiaim_edging_jumping_enabled = false;
			int antiaim_edging_type = 0;
			float antiaim_edging_lby_delta = 0.f;
			float antiaim_edging_jrange = 0.f;

			struct
			{
				int Type;
			}Fakelag;
			bool Breaker;
			bool Freestanding;
			bool ManualAA;
			bool InfinityDuck;
			struct
			{
				int left, right, back;
				int dLeft, dRight;
			}Manual;

		}Antiaim;
	}Ragebot;

	//struct
	//{
	//	int x, y, x2, y2;
	//}test;

	struct
	{
		struct
		{
			bool Box;
			bool Health;
			bool Name;
			bool Weapon;
			bool Armor;
			bool SnapLines;
			bool Skeleton;
			bool Hostage;
			bool LastHittedhitbox;
			bool esp_enemy_ammo_enabled;
			bool Money;
			bool Flashed;
			bool Zoom;

		}Player;

		bool DroppedWeapons;
		bool C4;

		struct
		{
			int ThirdPersonKey, TpAngs;
			bool Radar;
			bool Crosshair;
			bool Hitmarker;
			bool RecoilCrosshair;
			bool FovArrows_Enable;
			bool Nightmode;
			bool BulletTracers;
			int BulletTracerType;
			bool SpectatorList;
			int showHits;
		}Other;

		struct
		{
			bool NoFlash;
			bool NoSmoke;
			bool NoScope;
			bool NoVisRecoil;
		}Effects;

		struct
		{
			int Material = 0;
			bool Visible;
			bool XQZ;
			bool Weapons;
			bool Team;
			bool Local;
			bool Enable;
			bool Wireframe_Hand;
			float Wireframe_Hand_Color[4] = { 1.f, 1.f, 1.f, 1.f };
			float local_Visible[4] = { 1.f, 1.f, 1.f, 1.f };
			float local_Invisible[4] = { 1.f, 1.f, 1.f, 1.f };
			float team_Visible[4] = { 1.f, 1.f, 1.f, 1.f };
			float team_Invisible[4] = { 1.f, 1.f, 1.f, 1.f };
			float enemy_Visible[4] = { 1.f, 1.f, 1.f, 1.f };
			float enemy_Invisible[4] = { 1.f, 1.f, 1.f, 1.f };
		}Chams;

		struct
		{
			float Box[4]= { 1.f, 1.f, 1.f, 1.f };
			float SnapLines[4]= { 1.f, 1.f, 1.f, 1.f };
			float Skeleton[4]= { 1.f, 1.f, 1.f, 1.f };
			float BulletTracers[4] = { 1.f, 1.f, 1.f, 1.f };
			float FovArrows[4] = { 1.f, 1.f, 1.f, 1.f };
			float esp_enemy_ammo_color[4] = { 1.f, 1.f, 1.f, 1.f };
			float esp_name_color[4] = { 1.f, 1.f, 1.f, 1.f };
			float esp_weapon_color[4] = { 1.f, 1.f, 1.f, 1.f };
			struct
			{
				float Visible[4]= { 1.f, 1.f, 1.f, 1.f };
				float XQZ[4]= { 1.f, 1.f, 1.f, 1.f };
				float Weapons[4]= { 1.f, 1.f, 1.f, 1.f };
				float WeaponsXQZ[4]= { 1.f, 1.f, 1.f, 1.f };
				float Team[4]= { 1.f, 1.f, 1.f, 1.f };
				float TeamXQZ[4]= { 1.f, 1.f, 1.f, 1.f };

			}Chams;
		}Colors;
	}Visuals;

	struct
	{
		bool	Playerlist;
		bool	Resolveall;
		struct
		{
			//int YAngle[64];
			int PAngle[64];
			int YAngle[64];
			bool MiscFixes;
		}AAA;

	}Players;

	struct
	{
		bool AntiUT = true;
		bool MM_Mode = false;
		bool asdasad = false;
		bool Knifebot;
		bool Zeusbot;
		bool Bunnyhop;
		bool vmBool;
		float vmOffsetX = 0.f;
		float vmOffsetY = 0.f;
		float vmOffsetZ = 0.f;
		float FOV;
		float bgColor[4] = { 0.f, 1.f, 0.f, 0.94f };
		int Config;
		char configname[128];

		struct
		{
			bool Enable;
			int Type;
			bool WASD = false;
			bool ZStrafe;
			int ZStrafeKey;
			bool CircleStrafe;
			int CircleStrafeKey;
			bool DuckInAir;
			
		}Strafers;
		bool clantag;
	}Misc;

	struct
	{
		bool IsEdging;
		bool ShouldBaim;
	} pLocal;

	struct
	{
		int CurMemeShots;
	} Globals;

	struct
	{
		bool Opened, Init;
	}Menu;
};

extern Variables Vars;