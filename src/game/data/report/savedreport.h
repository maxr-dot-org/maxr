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

#ifndef game_data_reports_savedreportH
#define game_data_reports_savedreportH

#include <string>
#include <memory>

#include "utility/serialization/binaryarchive.h"
#include "utility/serialization/textarchive.h"
#include "utility/serialization/xmlarchive.h"

class cSoundManager;
class cPosition;
struct sID;
class cUnitsData;

enum class eSavedReportType
{
	// fixed numbers for save backward-compatibility
	// Simple reports
	MetalInsufficient    = 0,
	FuelInsufficient     = 1,
	GoldInsufficient     = 2,
	EnergyInsufficient   = 3,
	TeamInsufficient     = 4,

	MetalLow             = 5,
	FuelLow              = 6,
	GoldLow              = 7,
	EnergyLow            = 8,
	TeamLow              = 9,

	EnergyToLow          = 10,
	EnergyIsNeeded       = 11,

	BuildingDisabled     = 12,

	Producing_PositionBlocked = 13,
	Producing_InsufficientMaterial = 34,

	TurnWait             = 14,
	TurnAutoMove         = 15,

	// Special reports
	HostCommand          = 16,

	ResourceChanged      = 17,

	LostConnection       = 18,

	PlayerEndedTurn      = 19,
	PlayerDefeated       = 20,
	PlayerLeft           = 21,
	Upgraded             = 22,
	TurnStart            = 23,
	PlayerWins           = 35,
	SuddenDeath          = 36,

	// Unit reports
	Attacked             = 24,
	AttackingEnemy       = 25,
	CapturedByEnemy      = 26,
	Destroyed            = 27,
	Detected             = 28,
	Disabled             = 29,
	PathInterrupted      = 30,
	SurveyorAiConfused   = 31,

	// Chat report
	Chat                 = 33
};

class cSavedReport
{
public:
	virtual ~cSavedReport() {}

	virtual eSavedReportType getType() const = 0;

	virtual std::string getMessage(const cUnitsData& unitsData) const = 0;

	virtual bool isAlert() const = 0;

	virtual bool hasUnitId() const;
	virtual const sID& getUnitId() const;

	virtual bool hasPosition() const;
	virtual const cPosition& getPosition() const;

	virtual void playSound (cSoundManager& soundManager) const;

	static std::unique_ptr<cSavedReport> createFrom(cBinaryArchiveOut& archive);
	static std::unique_ptr<cSavedReport> createFrom(cXmlArchiveOut& archive, const std::string& name);

	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cXmlArchiveIn& archive) { serializeThis(archive); }
private:
	template <typename T>
	static std::unique_ptr<cSavedReport> createFromImpl(T& archive);
	template <typename T>
	void serializeThis(T& archive)
	{
		archive & serialization::makeNvp("type", getType());
	}
};

#endif // game_data_reports_savedreportH
