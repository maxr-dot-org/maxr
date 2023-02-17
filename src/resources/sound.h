/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef soundH
#define soundH

#include "output/sound/soundchunk.h"
#include "utility/serialization/serialization.h"

#include <array>
#include <string>
#include <vector>

// Sounds ////////////////////////////////////////////////////////////////////
class cSoundData
{
public:
	void load (const char* path);

public:
	// General
	cSoundChunk SNDAbsorb;
	cSoundChunk SNDActivate;
	cSoundChunk SNDArm;
	cSoundChunk SNDBuilding;
	cSoundChunk SNDChat;
	cSoundChunk SNDClearing;
	cSoundChunk SNDHudButton;
	cSoundChunk SNDHudSwitch;
	cSoundChunk SNDLandMineClear;
	cSoundChunk SNDLandMinePlace;
	cSoundChunk SNDLoad;
	cSoundChunk SNDMenuButton;
	cSoundChunk SNDObjectMenu;
	cSoundChunk SNDPanelClose;
	cSoundChunk SNDPanelOpen;
	cSoundChunk SNDQuitsch;
	cSoundChunk SNDReload;
	cSoundChunk SNDRepair;
	cSoundChunk SNDSeaMineClear;
	cSoundChunk SNDSeaMinePlace;
	cSoundChunk SNDHitSmall;
	cSoundChunk SNDHitMed;
	cSoundChunk SNDHitLarge;
	cSoundChunk SNDPlaneLand;
	cSoundChunk SNDPlaneTakeoff;

	// Explosions
	std::array<cSoundChunk, 4> EXPBig;
	std::array<cSoundChunk, 2> EXPBigWet;
	std::array<cSoundChunk, 3> EXPSmall;
	std::array<cSoundChunk, 3> EXPSmallWet;

	// Dummy
	cSoundChunk DummySound;
};

// Voices ////////////////////////////////////////////////////////////////////
class cVoiceData
{
public:
	void load (const char* path);

public:
	std::array<cSoundChunk, 2> VOIAmmoLow;
	std::array<cSoundChunk, 2> VOIAmmoEmpty;
	std::array<cSoundChunk, 2> VOIAttacking;
	std::array<cSoundChunk, 2> VOIAttackingEnemy;
	std::array<cSoundChunk, 3> VOIAttackingUs;
	std::array<cSoundChunk, 4> VOIBuildDone;
	cSoundChunk VOIClearing;
	std::array<cSoundChunk, 2> VOIClearingMines;
	std::array<cSoundChunk, 3> VOICommandoFailed;
	std::array<cSoundChunk, 2> VOIDestroyedUs;
	std::array<cSoundChunk, 2> VOIDetected;
	std::array<cSoundChunk, 3> VOILanding;
	cSoundChunk VOILayingMines;
	std::array<cSoundChunk, 2> VOINoPath;
	cSoundChunk VOINoSpeed;
	std::array<cSoundChunk, 4> VOIOK;
	cSoundChunk VOIReammo;
	cSoundChunk VOIReammoAll;
	std::array<cSoundChunk, 2> VOIRepaired;
	std::array<cSoundChunk, 2> VOIRepairedAll;
	cSoundChunk VOIResearchComplete;
	cSoundChunk VOISaved;
	cSoundChunk VOISentry;
	cSoundChunk VOIStartMore;
	cSoundChunk VOIStartNone;
	cSoundChunk VOIStartOne;
	std::array<cSoundChunk, 2> VOIStatusRed;
	std::array<cSoundChunk, 2> VOIStatusYellow;
	cSoundChunk VOISubDetected;
	std::array<cSoundChunk, 2> VOISurveying;
	cSoundChunk VOITransferDone;
	std::array<cSoundChunk, 3> VOITurnEnd20Sec;
	cSoundChunk VOIUnitDisabled;
	std::array<cSoundChunk, 2> VOIUnitDisabledByEnemy;
	std::array<cSoundChunk, 2> VOIUnitStolen;
	cSoundChunk VOIUnitStolenByEnemy;
};

struct sMusicFiles
{
	std::string start;
#if 0 // unused
	std::string credit;
#endif
	std::vector<std::string> backgrounds;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (start);
		archive & NVP (backgrounds);
	}
};

extern sMusicFiles MusicFiles;
extern cSoundData SoundData;
extern cVoiceData VoiceData;
#endif // soundH
