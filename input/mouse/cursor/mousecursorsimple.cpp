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

#include "mousecursorsimple.h"

#include "../../../main.h"
#include "../../../utility/position.h"

//------------------------------------------------------------------------------
cMouseCursorSimple::cMouseCursorSimple (eMouseCursorSimpleType type_) :
	type (type_)
{}

//------------------------------------------------------------------------------
SDL_Surface* cMouseCursorSimple::getSurface () const
{
	switch (type)
	{
	case eMouseCursorSimpleType::Hand:
		return GraphicsData.gfx_Chand;
	case eMouseCursorSimpleType::No:
		return GraphicsData.gfx_Cno;
	case eMouseCursorSimpleType::Select:
		return GraphicsData.gfx_Cselect;
	case eMouseCursorSimpleType::Move:
		return GraphicsData.gfx_Cmove;
	case eMouseCursorSimpleType::ArrowLeftDown:
		return GraphicsData.gfx_Cpfeil1;
	case eMouseCursorSimpleType::ArrowDown:
		return GraphicsData.gfx_Cpfeil2;
	case eMouseCursorSimpleType::ArrowRightDown:
		return GraphicsData.gfx_Cpfeil3;
	case eMouseCursorSimpleType::ArrowLeft:
		return GraphicsData.gfx_Cpfeil4;
	case eMouseCursorSimpleType::ArrowRight:
		return GraphicsData.gfx_Cpfeil6;
	case eMouseCursorSimpleType::ArrowLeftUp:
		return GraphicsData.gfx_Cpfeil7;
	case eMouseCursorSimpleType::ArrowUp:
		return GraphicsData.gfx_Cpfeil8;
	case eMouseCursorSimpleType::ArrowRightUp:
		return GraphicsData.gfx_Cpfeil9;
	case eMouseCursorSimpleType::Help:
		return GraphicsData.gfx_Chelp;
	case eMouseCursorSimpleType::Band:
		return GraphicsData.gfx_Cband;
	case eMouseCursorSimpleType::Transfer:
		return GraphicsData.gfx_Ctransf;
	case eMouseCursorSimpleType::Load:
		return GraphicsData.gfx_Cload;
	case eMouseCursorSimpleType::Muni:
		return GraphicsData.gfx_Cmuni;
	case eMouseCursorSimpleType::Repair:
		return GraphicsData.gfx_Crepair;
	case eMouseCursorSimpleType::Activate:
		return GraphicsData.gfx_Cactivate;
	case eMouseCursorSimpleType::MoveDraft:
		return GraphicsData.gfx_Cmove_draft;
	case eMouseCursorSimpleType::AttackOutOfRange:
		return GraphicsData.gfx_Cattackoor;
	default:
		assert (false);
	}
	return GraphicsData.gfx_Chand;
}

//------------------------------------------------------------------------------
cPosition cMouseCursorSimple::getHotPoint () const
{
	switch (type)
	{
	case eMouseCursorSimpleType::Select:
	case eMouseCursorSimpleType::Help:
	case eMouseCursorSimpleType::Move:
	case eMouseCursorSimpleType::MoveDraft:
	case eMouseCursorSimpleType::No:
	case eMouseCursorSimpleType::Transfer:
	case eMouseCursorSimpleType::Band:
	case eMouseCursorSimpleType::Load:
	case eMouseCursorSimpleType::Muni:
	case eMouseCursorSimpleType::Repair:
	case eMouseCursorSimpleType::Activate:
		return cPosition (12, 12);
	default:
		return cPosition (0, 0);
	}
	return cPosition (0, 0);
}

//------------------------------------------------------------------------------
bool cMouseCursorSimple::equal (const cMouseCursor& other) const
{
	auto other2 = dynamic_cast<const cMouseCursorSimple*>(&other);
	return other2 && other2->type == type;
}
