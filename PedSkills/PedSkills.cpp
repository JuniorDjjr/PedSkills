#include "plugin.h"
#include "CHud.h"
#include "CGeneral.h"
#include "CTimer.h"
#include "CTheScripts.h"
#include "..\injector\assembly.hpp"
#include "IniReader/IniReader.h"
  
using namespace std;
using namespace plugin;
using namespace injector;

bool ReadIniFloat(CIniReader ini, string section, string key, float* f)
{
	*f = ini.ReadFloat(section, key, -1.0f);
	return (*f != -1.0f);
}

bool ReadIniInt(CIniReader ini, string section, string key, int* i)
{
	*i = ini.ReadInteger(section, key, -1);
	return (*i != -1);
}

bool ReadIniBool(CIniReader ini, string section, string key)
{
	return ini.ReadInteger(section, key, 0) == 1;
}

bool initialized = false;
bool bParsePreserveComments = false;

float gShotgunAccuracyMult = 1.0f;
float gPlayerAccuracyMult = 1.0f;
float gAccuracyMult = 1.0f;
float gAccuracyVelocityFactor = 1.0f;
float gAccuracyDistanceFactor = 1.0f;
float gAccuracyOnVehicleFactor = 1.0f;
float gAccuracyOnJetpackFactor = 1.0f;
float gScriptAccuracyMult = 1.0f;
float gScriptAttackMult = 1.0f;
float gWeaponRangeMult = 1.0f;
float gWeaponRangeMult2 = 2.0f;
float gWeaponRangeMult3 = 3.0f;
float gWeaponAimMult = 1.0f;
float gPlayerVehMissionDamMult = 1.0f;
float gWeaponAimMult025 = 0.25f;
float gWeaponAimMult05 = 0.5f;
float gWeaponAimMult4 = 4.0f;

int gDefaultAttackRate = 40;
int gDefaultCopsAttackRate = 30;
int gDefaultSwatAttackRate = 70;
int gDefaultFBIAttackRate = 60;
int gDefaultArmyAttackRate = 80;

int gDefaultWeaponSkill = 1;

int gDefaultFightingStyle = 4;

int gDefaultWeaponAccuracy = 60;
int gDefaultCopsWeaponAccuracy = 60;
int gDefaultSwatWeaponAccuracy = 60;
int gDefaultFBIWeaponAccuracy = 76;
int gDefaultArmyWeaponAccuracy = 84;

float gDefaultHearingRange = 15.0f;
float gDefaultGangsHearingRange = 15.0f;

float gDefaultSeeingRange = 15.0f;
float gDefaultGangsSeeingRange = 15.0f;

float gDefaultDmRadius = 15.0f;

int gDefaultDmPedsToScan = 3;

float gDefaultCopsDmRadius = 60.0f;

int gDefaultCopsDmPedsToScan = 8;

float gDefaultCopsArmour = 0.0f;
float gDefaultSwatArmour = 50.0f;
float gDefaultFBIArmour = 100.0f;
float gDefaultArmyArmour = 100.0f;

float gTest = 0.01f;

void __declspec(naked) MultShotgunAccuracy_ASM()
{
	__asm
	{
		mov  dword ptr[esp + 1Ch], 0
		fmul gShotgunAccuracyMult
		push 73FC6Ah
		ret
	}
}

void __declspec(naked) WeaponAimMult_ASM()
{
	__asm
	{
		fld  dword ptr[esp + 68h]
		fmul dword ptr[esp + 68h]
		fmul gWeaponAimMult
		push 62C3A1h
		ret
	}
}

void __declspec(naked) WeaponAimMult2_ASM()
{
	__asm
	{
		fst     dword ptr[esp + 44h]
		mov     ecx, [esp + 14h]
		fdiv gWeaponAimMult
		push 62CECAh
		ret
	}
}

void __declspec(naked) WeaponAimMult3_ASM()
{
	__asm
	{
		fld     dword ptr[esp + 1Ch]
		fmul    dword ptr[esp + 1Ch]
		fmul gWeaponAimMult
		push 62E015h
		ret
	}
}

float GetAngleFromTwoCoords(float cx, float cy, float ex, float ey)
{
	float dx, dy, q1, q2, r;
	dy = ey;
	dy -= cy;
	dx = ex;
	dx -= cx;
	//to magnitude
	q1 = dx;
	q1 *= dx;
	q2 = dy;
	q2 *= dy;
	q1 += q2;
	r = sqrt(q1);
	dx /= r;
	dy /= r;

	float result = CGeneral::GetATanOfXY(dx, dy) * 57.295776f - 90.0f;

	while (result < 0.0) {
		result += 360.0;
	}
	return result;
}

class PedData {
public:
	float targetAngleSpeedFactor;
	float targetDistanceFactor;
	float lastTargetAngle;

	PedData(CPed *ped) {
		targetAngleSpeedFactor = 0.0f;
		targetDistanceFactor = 0.0f;
		lastTargetAngle = 0.0f;
	}
};
PedExtendedData<PedData> pedData;

class PedSkills
{
public:
	PedSkills()
	{
		Events::initScriptsEvent += []
		{
			if (initialized) return;

			CIniReader ini("PedSkills.SA.ini");
			int i = 0;
			float f = 0.0f;

			// Default attack rate (40)
			if (ReadIniInt(ini, "Default skills", "AttackRate", &i))
			{
				gDefaultAttackRate = i;
				WriteMemory<uint8_t>(0x5E8260 - 1, gDefaultAttackRate, true);
			}

			// Default Cops attack rate (30)
			if (ReadIniInt(ini, "Default skills", "CopsAttackRate", &i))
			{
				gDefaultCopsAttackRate = i;
				WriteMemory<uint8_t>(0x5DDCE6 - 1, gDefaultCopsAttackRate, true);
			}

			// Default SWAT Cops attack rate (70)
			if (ReadIniInt(ini, "Default skills", "SwatAttackRate", &i))
			{
				gDefaultSwatAttackRate = i;
				WriteMemory<uint8_t>(0x5DDDC3 - 1, gDefaultSwatAttackRate, true);
			}

			// Default FBI Cops attack rate (60)
			if (ReadIniInt(ini, "Default skills", "FBIAttackRate", &i))
			{
				gDefaultFBIAttackRate = i;
				WriteMemory<uint8_t>(0x5DDE03 - 1, gDefaultFBIAttackRate, true);
			}

			// Default Army Cops attack rate (80)
			if (ReadIniInt(ini, "Default skills", "ArmyAttackRate", &i))
			{
				gDefaultArmyAttackRate = i;
				WriteMemory<uint8_t>(0x5DDE43 - 1, gDefaultArmyAttackRate, true);
			}

			// Default weapon skill (1)
			if (ReadIniInt(ini, "Default skills", "WeaponSkill", &i))
			{
				gDefaultWeaponSkill = i;
				WriteMemory<uint8_t>(0x05E83D1 - 1, gDefaultWeaponSkill, true);
			}

			// Default fighting style (4)
			if (ReadIniInt(ini, "Default skills", "FightingStyle", &i))
			{
				gDefaultFightingStyle = i;
				WriteMemory<uint8_t>(0x5E83D8 - 1, gDefaultFightingStyle, true);
			}

			// Default weapon accuracy (60)
			if (ReadIniInt(ini, "Default skills", "WeaponAccuracy", &i))
			{
				gDefaultWeaponAccuracy = i;
				WriteMemory<uint8_t>(0x5E83F2 - 1, gDefaultWeaponAccuracy, true);
			}

			// Default Cops weapon accuracy (60)
			if (ReadIniInt(ini, "Default skills", "CopsWeaponAccuracy", &i))
			{
				gDefaultCopsWeaponAccuracy = i;
				WriteMemory<uint8_t>(0x5DDCED - 1, gDefaultCopsWeaponAccuracy, true);
			}

			// Default SWAT Cops weapon accuracy (60)
			if (ReadIniInt(ini, "Default skills", "SwatWeaponAccuracy", &i))
			{
				gDefaultSwatWeaponAccuracy = i;
				WriteMemory<uint8_t>(0x5DDDCA - 1, gDefaultSwatWeaponAccuracy, true);
			}

			// Default FBI Cops weapon accuracy (76)
			if (ReadIniInt(ini, "Default skills", "WeaponAccuracy", &i))
			{
				gDefaultWeaponAccuracy = i;
				WriteMemory<uint8_t>(0x5DDE0A - 1, gDefaultFBIWeaponAccuracy, true);
			}

			// Default Army Cops weapon accuracy (84)
			if (ReadIniInt(ini, "Default skills", "ArmyWeaponAccuracy", &i))
			{
				gDefaultArmyWeaponAccuracy = i;
				WriteMemory<uint8_t>(0x5DDE4A - 1, gDefaultArmyWeaponAccuracy, true);
			}
			
			// Default hearing range (15.0)
			if (ReadIniFloat(ini, "Default skills", "HearingRange", &f))
			{
				gDefaultHearingRange = f;
				WriteMemory<float*>(0x60719B + 2, &gDefaultHearingRange, true);
			}

			// Default Gangs hearing range (40.0)
			if (ReadIniFloat(ini, "Default skills", "GangsHearingRange", &f))
			{
				gDefaultGangsHearingRange = f;
				WriteMemory<float*>(0x6072C7 + 2, &gDefaultGangsHearingRange, true);
			}

			// Default seeing range (15.0)
			if (ReadIniFloat(ini, "Default skills", "SeeingRange", &f))
			{
				gDefaultSeeingRange = f;
				WriteMemory<float*>(0x6071A7 + 2, &gDefaultSeeingRange, true);
			}

			// Default Gangs seeing range (40.0)
			if (ReadIniFloat(ini, "Default skills", "GangsSeeingRange", &f))
			{
				gDefaultGangsSeeingRange = f;
				WriteMemory<float*>(0x6072BC + 1, &gDefaultGangsSeeingRange, true);
			}

			// Default Dm radius (15.0)
			if (ReadIniFloat(ini, "Default skills", "DmRadius", &f))
			{
				gDefaultDmRadius = f;
				WriteMemory<float>(0x6071BD + 6, gDefaultDmRadius, true);
			}

			// Default Dm peds to scan (3)
			if (ReadIniInt(ini, "Default skills", "DmPedsToScan", &i))
			{
				gDefaultDmPedsToScan = i;
				WriteMemory<int>(0x6071B3 + 6, gDefaultDmPedsToScan, true);
			}

			// Default Cops Dm radius (60.0)
			if (ReadIniFloat(ini, "Default skills", "CopsDmRadius", &f))
			{
				gDefaultCopsDmRadius = f;
				WriteMemory<float>(0x5DDD33 + 6, gDefaultCopsDmRadius, true);
			}

			// Default Cops Dm peds to scan (8)
			if (ReadIniInt(ini, "Default skills", "CopsDmPedsToScan", &i))
			{
				gDefaultCopsDmPedsToScan = i;
				WriteMemory<int>(0x5DDD3D + 6, gDefaultCopsDmPedsToScan, true);
			}

			// Default SWAT Cops armour (50.0)
			if (ReadIniFloat(ini, "Default skills", "SwatArmour", &f))
			{
				gDefaultSwatArmour = f;
				WriteMemory<float>(0x5DDDB2 + 6, gDefaultSwatArmour, true);
			}

			// Default FBI Cops armour (100.0)
			if (ReadIniFloat(ini, "Default skills", "FBIArmour", &f))
			{
				gDefaultFBIArmour = f;
				WriteMemory<float>(0x5DDDF2 + 6, gDefaultFBIArmour, true);
			}

			// Default Army Cops armour (100.0)
			if (ReadIniFloat(ini, "Default skills", "ArmyArmour", &f))
			{
				gDefaultArmyArmour = f;
				WriteMemory<float>(0x5DDE32 + 6, gDefaultArmyArmour, true);
			}

			// Default Cops armour (0.0)
			if (ReadIniFloat(ini, "Default skills", "CopsArmour", &f))
			{
				gDefaultCopsArmour = f;
				if (!initialized && gDefaultCopsArmour != 0.0f) MakeInline<0x5DDCD9, 0x5DDCD9 + 6>([](reg_pack& regs) { *(float*)(regs.esi + 0x548) = gDefaultCopsArmour; });
			}

			// Multiply script set accuracy (1.0)
			if (ReadIniFloat(ini, "Default skills", "ScriptAccuracyMult", &f))
			{
				gScriptAccuracyMult = f;
				if (!initialized && gScriptAccuracyMult != 1.0f)
				{
					MakeInline<0x48069E, 0x48069E + 6>([](reg_pack& regs)
					{
						CPed *ped = (CPed *)regs.eax;
						if (ped) {
							int accuracy = (uint8_t)regs.edx;
							accuracy *= gScriptAccuracyMult;
							if (accuracy > 127) accuracy = 127;
							ped->m_nWeaponAccuracy = accuracy;
						}
					});
				}
			}

			// Multiply script set attack (1.0)
			if (ReadIniFloat(ini, "Default skills", "ScriptAttackMult", &f))
			{
				gScriptAttackMult = f;
				if (!initialized && gScriptAttackMult != 1.0f)
				{
					MakeInline<0x472779, 0x472779 + 6>([](reg_pack& regs)
					{
						CPed *ped = (CPed *)regs.eax;
						if (ped) {
							int shootRate = (uint8_t)regs.ecx;
							shootRate *= gScriptAttackMult;
							if (shootRate > 127) shootRate = 127;
							ped->m_nWeaponShootingRate = shootRate;
						}
					});
				}
			}

			// Multiply ped weapon accuracy
			if (ReadIniFloat(ini, "General", "PlayerAccuracyMult", &f)) {
				gPlayerAccuracyMult = f;
			}
			else gPlayerAccuracyMult = 1.0f;

			if (ReadIniFloat(ini, "General", "AccuracyMult", &f)) {
				gAccuracyMult = f;
			}
			else gAccuracyMult = 1.0f;

			if (ReadIniFloat(ini, "General", "AccuracyVelocityFactor", &f)) {
				gAccuracyVelocityFactor = f;
			}
			else gAccuracyVelocityFactor = 1.0f;

			if (ReadIniFloat(ini, "General", "AccuracyDistanceFactor", &f)) {
				gAccuracyDistanceFactor = f;
			}
			else gAccuracyDistanceFactor = 1.0f;

			if (ReadIniFloat(ini, "General", "AccuracyOnVehicleFactor", &f)) {
				gAccuracyOnVehicleFactor = f;
			}
			else gAccuracyOnVehicleFactor = 1.0f;

			if (ReadIniFloat(ini, "General", "AccuracyOnJetpackFactor", &f)) {
				gAccuracyOnJetpackFactor = f;
			}
			else gAccuracyOnJetpackFactor = 1.0f;


			if (!initialized && (gPlayerAccuracyMult != 1.0f || gAccuracyMult != 1.0f || gAccuracyVelocityFactor > 0.0f || gAccuracyDistanceFactor > 0.0f || gAccuracyOnVehicleFactor > 0.0f || gAccuracyOnJetpackFactor > 0.0f))
			{
				MakeInline<0x73FBFD, 0x73FBFD + 7>([](reg_pack& regs)
				{
					CPed *firingPed = (CPed *)regs.edi;

					float accuracyTargetTweak = 0.5f;
					if (!firingPed->IsPlayer())
					{
						PedData &data = pedData.Get(firingPed);
						accuracyTargetTweak += data.targetDistanceFactor;
						accuracyTargetTweak += data.targetAngleSpeedFactor;
						CPlayerPed* playerPed = FindPlayerPed(-1);
						if (playerPed->m_pIntelligence->GetTaskJetPack()) {
							if (playerPed->m_nPedFlags.bIsInTheAir) {
								accuracyTargetTweak += 1.0f * gAccuracyOnJetpackFactor;
							}
							else {
								accuracyTargetTweak += 1.0f * gAccuracyOnJetpackFactor * gAccuracyOnJetpackFactor;
							}
						}
					}
					if (firingPed->m_pVehicle) {
						accuracyTargetTweak += 1.0f * gAccuracyOnVehicleFactor;
					}
					regs.edx = firingPed->m_nWeaponAccuracy * ((firingPed->IsPlayer()) ? gPlayerAccuracyMult : (gAccuracyMult / accuracyTargetTweak));
				});
			}

			// Multiply shotgun accuracy
			if (ReadIniFloat(ini, "General", "ShotgunAccuracyMult", &f)) {
				gShotgunAccuracyMult = f;
				if (!initialized) MakeJMP(0x73FC62, MultShotgunAccuracy_ASM);
			}

			if (ReadIniFloat(ini, "General", "WeaponRangeMult", &f)) {
				gWeaponRangeMult = f;
				WriteMemory<float*>(0x73B421, &gWeaponRangeMult, true);
				gWeaponRangeMult2 = f * 2.0f;
				WriteMemory<float*>(0x73B40D, &gWeaponRangeMult2, true);
				gWeaponRangeMult3 = f * 3.0f;
				WriteMemory<float*>(0x73B41A, &gWeaponRangeMult3, true);
			}

			if (ReadIniFloat(ini, "General", "WeaponAimMult", &f)) {
				gWeaponAimMult = f;
				if (gWeaponAimMult != 1.0f) {
					gWeaponAimMult025 = f * 0.25f;
					gWeaponAimMult05 = f * 0.5f;
					if (!initialized) {
						WriteMemory<float*>(0x62C10B + 2, &gWeaponAimMult025, true);
						WriteMemory<float*>(0x62D17A + 2, &gWeaponAimMult05, true);
						//WriteMemory<float*>(0x62CEE2 + 2, &gWeaponAimMult05, true); // the 0x62CEC2 hook is equivalent
						MakeJMP(0x62C399, WeaponAimMult_ASM);
						MakeJMP(0x62CEC2, WeaponAimMult2_ASM);
						MakeJMP(0x62E00D, WeaponAimMult3_ASM);
						MakeInline<0x62C20D, 0x62C20D + 7>([](reg_pack& regs)
						{
							regs.eax = *(float*)(regs.eax + 0x4) * gWeaponAimMult; //mov eax, [eax+4]
							*(float*)(regs.esp + 0x1C) = regs.eax; //mov [esp+1Ch], eax
						});
					}
				}
			}

			if (ReadIniFloat(ini, "General", "PlayerVehMissionDamMult", &f)) {
				gPlayerVehMissionDamMult = f;
				if (gPlayerVehMissionDamMult != 1.0f) {
					if (!initialized) {
						injector::MakeInline<0x6D7DF2, 0x6D7DF2 + 6>([](injector::reg_pack& regs)
						{
							regs.eax = *(uint32_t*)(regs.edi + 0x58C); // mov     eax, [edi+58Ch]

							CVehicle* vehicle = (CVehicle*)regs.esi;
							CPlayerPed* playerPed = FindPlayerPed(-1);
							if (CTheScripts::IsPlayerOnAMission() && vehicle->m_nCreatedBy == eVehicleCreatedBy::MISSION_VEHICLE && FindPlayerPed(-1)->m_pVehicle == vehicle && !vehicle->m_nVehicleFlags.bIsRCVehicle) {

								float thisDamage = *(float*)(regs.esp + 0x84 + 0xC);

								// Fix for petrol cap (not 100% but this way we don't need to do another hook).
								if (thisDamage != vehicle->m_fHealth && thisDamage != 1000.0f) {
									float finalDamageMult = gPlayerVehMissionDamMult;
									if (playerPed->m_pVehicle && vehicle->m_pDriver != playerPed) finalDamageMult *= gPlayerVehMissionDamMult;
									*(float*)(regs.esp + 0x84 + 0xC) *= finalDamageMult;
								}
							}
						});
					}
				}
			}

			initialized = true;
		};

		Events::pedRenderEvent += [](CPed* ped) {
			PedData &data = pedData.Get(ped);
			if (data.targetAngleSpeedFactor > 0.0f) {
				data.targetAngleSpeedFactor -= 0.5f * gAccuracyVelocityFactor * (CTimer::ms_fTimeStep * 1.666667f);
				if (data.targetAngleSpeedFactor < 0.0f) data.targetAngleSpeedFactor = 0.0f;
			}

			CTaskSimpleUseGun *taskUseGun = ped->m_pIntelligence->GetTaskUseGun();
			if (taskUseGun) {

				CVector pedVector = ped->GetPosition();
				CVector* targetVector = nullptr;
				CEntity* targetEntity = taskUseGun->m_pTarget;
				if (targetEntity)
				{
					targetVector = &targetEntity->GetPosition();
				}
				else
				{
					if (taskUseGun->m_vecTarget.x != 0.0f && taskUseGun->m_vecTarget.y != 0.0f) {
						targetVector = &taskUseGun->m_vecTarget;
					}
				}
				if (targetVector)
				{
					float targetCurrentAngle = GetAngleFromTwoCoords(targetVector->x, targetVector->y, pedVector.x, pedVector.y);
					targetCurrentAngle -= 180.0f;
					targetCurrentAngle = abs(targetCurrentAngle);
					float angleDiff = abs(data.lastTargetAngle - targetCurrentAngle);
					data.lastTargetAngle = targetCurrentAngle;

					data.targetAngleSpeedFactor += angleDiff * 0.1f * gAccuracyVelocityFactor * (CTimer::ms_fTimeStep * 1.666667f);
					if (data.targetAngleSpeedFactor > 5.0f) data.targetAngleSpeedFactor = 5.0f;

					//std::string str = to_string(data.targetAngleSpeedFactor);
					//CHud::SetHelpMessage((char*)str.c_str(), 0, 1, 0);

					float distance = DistanceBetweenPoints(pedVector, *targetVector);
					distance *= 0.3f * gAccuracyDistanceFactor;
					distance = log2(distance);
					distance = clamp(distance, 0.0f, 10.0f);
					data.targetDistanceFactor = distance;
				}
			}

		};


    }
} pedSkills;
