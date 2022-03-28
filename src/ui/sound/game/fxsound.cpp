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

#include "fxsound.h"

#include "game/data/map/map.h"
#include "game/logic/fxeffects.h"
#include "resources/buildinguidata.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/sound/effects/soundeffectposition.h"
#include "ui/sound/soundmanager.h"
#include "utility/random.h"

namespace
{
	//--------------------------------------------------------------------------
	cPosition toMapPosition (const cPosition& pixelPos)
	{
		return {pixelPos.x() / sGraphicTile::tilePixelWidth, pixelPos.y() / sGraphicTile::tilePixelHeight};
	}

	//----------------------------------------------------------------------
	void playSoundMuzzle (cSoundManager& soundManager, const cFxMuzzle& fx)
	{
		const auto& id = fx.getId();
		const auto& attack = id.isABuilding() ? UnitsUiData.getBuildingUI (id)->Attack : UnitsUiData.getVehicleUI (id)->Attack;
		const auto& mapPosition = toMapPosition (fx.getPixelPosition());
		soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, attack, mapPosition));
	}

	//--------------------------------------------------------------------------
	class cFxSoundVisitor : public IFxVisitor
	{
		cSoundManager& soundManager;
	public:
		explicit cFxSoundVisitor (cSoundManager& soundManager) : soundManager (soundManager) {}

		//----------------------------------------------------------------------
		void visit (const cFxMuzzleBig& fx) override { playSoundMuzzle (soundManager, fx); }
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleMed& fx) override { playSoundMuzzle (soundManager, fx); }
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleMedLong& fx) override { playSoundMuzzle (soundManager, fx); }
		//----------------------------------------------------------------------
		void visit (const cFxMuzzleSmall& fx) override { playSoundMuzzle (soundManager, fx); }

		//----------------------------------------------------------------------
		void visit (const cFxExploAir& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmall), mapPosition));
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploBig& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			if (fx.isOnWater())
			{
				soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPBigWet), mapPosition));
			}
			else
			{
				soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPBig), mapPosition));
			}
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploSmall& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmall), mapPosition));
		}
		//----------------------------------------------------------------------
		void visit (const cFxExploWater& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, getRandom (SoundData.EXPSmallWet), mapPosition));
		}
		//----------------------------------------------------------------------
		void visit (const cFxAbsorb& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDAbsorb, mapPosition));
		}
		//----------------------------------------------------------------------
		void visit (const cFxRocket& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			const auto& id = fx.getId();
			if (id.isABuilding())
				soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsUiData.getBuildingUI (id)->Attack, mapPosition));
			else
				soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, UnitsUiData.getVehicleUI (id)->Attack, mapPosition));
		}
		//----------------------------------------------------------------------
		void visit (const cFxCorpse&) override { /*Empty*/ }
		//----------------------------------------------------------------------
		void visit (const cFxSmoke&) override { /*Empty*/ }
		//----------------------------------------------------------------------
		void visit (const cFxDarkSmoke&) override { /*Empty*/ }
		//----------------------------------------------------------------------
		void visit (const cFxHit& fx) override
		{
			const auto& mapPosition = toMapPosition (fx.getPixelPosition());
			if (fx.isTargetHit())
			{
				if (fx.isBig())
					soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitLarge, mapPosition));
				else
					soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitMed, mapPosition));
			}
			else
			{
				soundManager.playSound (std::make_shared<cSoundEffectPosition> (eSoundEffectType::EffectExplosion, SoundData.SNDHitSmall, mapPosition));
			}
		}
		//----------------------------------------------------------------------
		void visit (const cFxTracks&) override { /*Empty*/ }
	};
}

//------------------------------------------------------------------------------
void playEffectSound (cSoundManager& soundManager, const cFx& fx)
{
	cFxSoundVisitor visitor{soundManager};
	fx.accept (visitor);
}
