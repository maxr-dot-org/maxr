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

#include <sstream>
#include <iomanip>

#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/menu/widgets/label.h"
#include "game/logic/turntimeclock.h"

//------------------------------------------------------------------------------
cTurnTimeClockWidget::cTurnTimeClockWidget (const cBox<cPosition>& area) :
	cWidget (area)
{
	textLabel = addChild (std::make_unique <cLabel> (area, "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
}

//------------------------------------------------------------------------------
void cTurnTimeClockWidget::setTurnTimeClock (std::shared_ptr<const cTurnTimeClock> turnTimeClock_)
{
	turnTimeClock = std::move (turnTimeClock_);

	signalConnectionManager.disconnectAll ();

	if (turnTimeClock != nullptr)
	{
		signalConnectionManager.connect (turnTimeClock->secondChanged, std::bind (&cTurnTimeClockWidget::update, this));
		signalConnectionManager.connect (turnTimeClock->deadlinesChanged, std::bind (&cTurnTimeClockWidget::update, this));
	}
}

//------------------------------------------------------------------------------
void cTurnTimeClockWidget::update ()
{
	const auto time = turnTimeClock->hasDeadline () ? turnTimeClock->getTimeTillFirstDeadline () : turnTimeClock->getTimeSinceStart ();

	const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(time);
	const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(time)-std::chrono::duration_cast<std::chrono::seconds>(minutes);

	std::stringstream text;

	text << std::setw (2) << std::setfill ('0') << minutes.count ()
		<< ":"
		<< std::setw (2) << std::setfill ('0') << seconds.count ();

	textLabel->setText (text.str ());

	const std::chrono::seconds alertRemainingTime (30);

	// TODO: activate when red font gets implemented
	//if (turnTimeClock->hasDeadline () && time < alertRemainingTime)
	//{
	//	timeLabel->setFont (FONT_LATIN_NORMAL_RED);
	//}
	//else
	//{
	//	timeLabel->setFont (FONT_LATIN_NORMAL);
	//}
}