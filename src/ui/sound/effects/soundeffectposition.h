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

#ifndef ui_sound_effects_soundeffectpositionH
#define ui_sound_effects_soundeffectpositionH

#include "ui/sound/effects/soundeffect.h"
#include "utility/position.h"

class cSoundEffectPosition : public cSoundEffect
{
public:
	cSoundEffectPosition (eSoundEffectType type, const cSoundChunk& sound, const cPosition& position);

	bool hasPosition() const override;
	const cPosition& getPosition() const override;

private:
	cPosition position;
};

#endif // ui_sound_effects_soundeffectpositionH
