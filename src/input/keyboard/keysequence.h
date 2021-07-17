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

#ifndef input_keyboard_keysequence_H
#define input_keyboard_keysequence_H

#include <string>
#include <vector>

#include "input/keyboard/keycombination.h"
#include "game/serialization/serialization.h"

class cKeySequence
{
public:
	cKeySequence();
	explicit cKeySequence (const std::string& sequence);
	explicit cKeySequence (cKeyCombination);
	cKeySequence (cKeyCombination keyCombination1, cKeyCombination keyCombination2);
	cKeySequence (cKeyCombination keyCombination1, cKeyCombination keyCombination2, cKeyCombination keyCombination3);

	bool operator== (const cKeySequence&) const;
	bool operator!= (const cKeySequence&) const;

	void addKeyCombination (cKeyCombination);
	void removeFirst();

	size_t length() const;
	const cKeyCombination& operator[] (size_t index) const;

	void reset();

	std::string toString() const;

	template <typename Archive>
	void load (Archive& archive)
	{
		std::string sequence;
		archive >> serialization::makeNvp ("text", sequence);
		*this = cKeySequence (sequence);
	}

	template <typename Archive>
	void save (Archive& archive) const
	{
		archive << serialization::makeNvp ("text", toString());
	}
	SERIALIZATION_SPLIT_MEMBER()

private:
	std::vector<cKeyCombination> keySequence;
};

#endif // input_keyboard_keysequence_H
