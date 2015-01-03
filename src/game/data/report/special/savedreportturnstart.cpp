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

#include "game/data/report/special/savedreportturnstart.h"
#include "netmessage.h"
#include "game/data/player/player.h"
#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "sound.h"

//------------------------------------------------------------------------------
cSavedReportTurnStart::cSavedReportTurnStart (const cPlayer& player, int turn_) :
	turn (turn_),
	unitReports (player.getCurrentTurnUnitReports()),
	researchAreas (player.getCurrentTurnResearchAreasFinished())
{}

//------------------------------------------------------------------------------
cSavedReportTurnStart::cSavedReportTurnStart (cNetMessage& message)
{
	turn = message.popInt32();

	researchAreas.resize (message.popInt32());
	for (auto iter = researchAreas.begin(); iter != researchAreas.end(); ++iter)
	{
		*iter = message.popInt32();
	}

	unitReports.resize (message.popInt32());
	for (auto iter = unitReports.begin(); iter != unitReports.end(); ++iter)
	{
		iter->type = message.popID();
		iter->count = message.popInt32();
	}
}

//------------------------------------------------------------------------------
cSavedReportTurnStart::cSavedReportTurnStart (const tinyxml2::XMLElement& element)
{
	turn = element.IntAttribute ("turn");

	auto unitElement = element.FirstChildElement ("Unit");
	while (unitElement)
	{
		sTurnstartReport entry;
		entry.count = unitElement->IntAttribute ("count");
		entry.type.generate (unitElement->Attribute ("id"));
		unitReports.push_back (entry);

		unitElement = unitElement->NextSiblingElement ("Unit");
	}

	auto researchElement = element.FirstChildElement ("ResearchArea");
	while (researchElement)
	{
		researchAreas.push_back (researchElement->IntAttribute ("id"));

		researchElement = researchElement->NextSiblingElement ("ResearchArea");
	}
}

//------------------------------------------------------------------------------
void cSavedReportTurnStart::pushInto (cNetMessage& message) const
{
	for (auto iter = unitReports.crbegin(); iter != unitReports.crend(); ++iter)
	{
		const auto& entry = *iter;
		message.pushInt32 (entry.count);
		message.pushID (entry.type);
	}
	message.pushInt32 (unitReports.size());

	for (auto iter = researchAreas.crbegin(); iter != researchAreas.crend(); ++iter)
	{
		message.pushInt32 (*iter);
	}
	message.pushInt32 (researchAreas.size());

	message.pushInt32 (turn);

	cSavedReport::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportTurnStart::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("turn", iToStr (turn).c_str());

	for (auto iter = unitReports.crbegin(); iter != unitReports.crend(); ++iter)
	{
		auto unitElement = element.GetDocument()->NewElement ("Unit");
		element.LinkEndChild (unitElement);
		unitElement->SetAttribute ("id", iter->type.getText().c_str());
		unitElement->SetAttribute ("count", iToStr (iter->count).c_str());
	}

	for (auto iter = researchAreas.crbegin(); iter != researchAreas.crend(); ++iter)
	{
		auto unitElement = element.GetDocument()->NewElement ("ResearchArea");
		element.LinkEndChild (unitElement);
		unitElement->SetAttribute ("id", iToStr (*iter).c_str());
	}
	cSavedReport::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportTurnStart::getType() const
{
	return eSavedReportType::TurnStart;
}

//------------------------------------------------------------------------------
std::string cSavedReportTurnStart::getMessage() const
{
	std::string message = lngPack.i18n ("Text~Comp~Turn_Start") + " " + iToStr (turn);

	if (!unitReports.empty())
	{
		int totalUnitsCount = 0;
		message += "\n";
		for (size_t i = 0; i < unitReports.size(); ++i)
		{
			const auto& entry = unitReports[i];

			if (i > 0) message += ", ";
			totalUnitsCount += entry.count;
			message += entry.count > 1 ? (iToStr (entry.count) + " " + entry.type.getUnitDataOriginalVersion()->name) : (entry.type.getUnitDataOriginalVersion()->name);
		}
		if (totalUnitsCount == 1) message += " " + lngPack.i18n ("Text~Comp~Finished") + ".";
		else if (totalUnitsCount > 1) message += " " + lngPack.i18n ("Text~Comp~Finished2") + ".";
	}

	if (!researchAreas.empty())
	{
		message += "\n";
		message += lngPack.i18n ("Text~Others~Research") + " " + lngPack.i18n ("Text~Comp~Finished") + ": ";

		const std::string themeNames[8] =
		{
			lngPack.i18n ("Text~Others~Attack"),
			lngPack.i18n ("Text~Others~Shots"),
			lngPack.i18n ("Text~Others~Range"),
			lngPack.i18n ("Text~Others~Armor"),
			lngPack.i18n ("Text~Others~Hitpoints"),
			lngPack.i18n ("Text~Others~Speed"),
			lngPack.i18n ("Text~Others~Scan"),
			lngPack.i18n ("Text~Others~Costs")
		};

		for (size_t i = 0; i < researchAreas.size(); ++i)
		{
			const auto researchArea = researchAreas[i];
			if (researchArea >= 0 && researchArea < 8)
			{
				if (i > 0) message += ", ";
				message += themeNames[researchArea];
			}
		}
	}

	return message;
}

//------------------------------------------------------------------------------
bool cSavedReportTurnStart::isAlert() const
{
	return false;
}

//------------------------------------------------------------------------------
void cSavedReportTurnStart::playSound (cSoundManager& soundManager) const
{
	if (!researchAreas.empty())
	{
		soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIResearchComplete));
	}
	else
	{
		int relevantCount = 0;
		for (size_t i = 0; i < unitReports.size(); ++i)
		{
			relevantCount += unitReports[i].count;
			if (relevantCount > 1) break;
		}

		if (relevantCount == 0)
		{
			soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartNone));
		}
		else if (relevantCount == 1)
		{
			soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartOne));
		}
		else if (relevantCount > 1)
		{
			soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartMore));
		}
	}
}
