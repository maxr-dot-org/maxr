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

#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"

#include "game/data/map/map.h"
#include "output/video/video.h"
#include "resources/map/mappreview.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "SDLutility/autosurface.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/uidefines.h"
#include "utility/files.h"
#include "utility/language.h"
#include "utility/listhelpers.h"

//------------------------------------------------------------------------------
cWindowMapSelection::cWindowMapSelection() :
	cWindow (LoadPCX (GFXOD_PLANET_SELECT))
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Title~Choose_Planet"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal));

	//
	// Map Images
	//
	for (size_t row = 0; row < mapRows; ++row)
	{
		for (size_t column = 0; column < mapColumns; ++column)
		{
			const auto index = row * mapColumns + column;

			mapImages[index] = addChild (std::make_unique<cImage> (getPosition() + cPosition (21 + 158 * column, 86 + 198 * row), nullptr, &SoundData.SNDHudButton));
			signalConnectionManager.connect (mapImages[index]->clicked, [this, index]() { mapClicked (index); });

			mapTitles[index] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (6 + 158 * column, 48 + 198 * row), getPosition() + cPosition (155 + 158 * column, 48 + 10 + 198 * row)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal));
		}
	}

	//
	// Buttons
	//
	upButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (292, 435), ePushButtonType::ArrowUpBig));
	signalConnectionManager.connect (upButton->clicked, [this]() { upClicked(); });

	downButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (321, 435), ePushButtonType::ArrowDownBig));
	signalConnectionManager.connect (downButton->clicked, [this]() { downClicked(); });

	okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	okButton->lock();
	signalConnectionManager.connect (okButton->clicked, [this]() { okClicked(); });

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, [this]() { backClicked(); });

	loadMaps();
	updateMaps();

	updateUpDownLocked();
}

//------------------------------------------------------------------------------
void cWindowMapSelection::mapClicked (int imageIndex)
{
	selectedMapIndex = page * mapCount + imageIndex;
	updateMaps();
	okButton->unlock();
}

//------------------------------------------------------------------------------
void cWindowMapSelection::upClicked()
{
	if (page > 0)
	{
		--page;
		updateMaps();
		updateUpDownLocked();
	}
}

//------------------------------------------------------------------------------
void cWindowMapSelection::downClicked()
{
	if ((page + 1) * mapCount < maps.size())
	{
		++page;
		updateMaps();
		updateUpDownLocked();
	}
}

//------------------------------------------------------------------------------
void cWindowMapSelection::okClicked()
{
	assert (static_cast<std::size_t>(selectedMapIndex) < maps.size());
	done (maps[selectedMapIndex]);
}

//------------------------------------------------------------------------------
void cWindowMapSelection::backClicked()
{
	close();
}

//------------------------------------------------------------------------------
void cWindowMapSelection::updateUpDownLocked()
{
	if (page == 0) upButton->lock();
	else upButton->unlock();

	if (maps.size() <= (page + 1) * mapCount) downButton->lock();
	else downButton->unlock();
}

//------------------------------------------------------------------------------
void cWindowMapSelection::updateMaps()
{
	auto* font = cUnicodeFont::font.get();
	for (size_t i = 0; i < mapCount; ++i)
	{
		const auto mapIndex = page * mapCount + i;
		if (mapIndex < maps.size())
		{
			auto mapName = maps[mapIndex];
			auto preview = loadMapPreview (mapName);

			if (preview.surface == nullptr) continue;

			const auto size = preview.size;
			const int mapWinSize = 112;
			const int selectedColor = 0x00'C0'00;
			const int unselectedColor = 0x00'00'00;
			AutoSurface imageSurface (SDL_CreateRGBSurface (0, mapWinSize + 8, mapWinSize + 8, Video.getColDepth(), 0, 0, 0, 0));

			// remove extension
			mapName = mapName.substr (0, mapName.length() - 4);
			// shorter too long name and add map size suffix
			auto mapSizeTxt = " (" + std::to_string (size.x()) + "x" + std::to_string (size.y()) + ")";
			if (font->getTextWide (">" + mapName + mapSizeTxt + "<") > 140)
			{
				while (font->getTextWide (">" + mapName + "..." + mapSizeTxt + "<") > 140)
				{
					mapName.pop_back();
				}
				mapName += "...";
			}
			mapName += mapSizeTxt;

			if (selectedMapIndex == static_cast<int> (mapIndex))
			{
				SDL_FillRect (imageSurface.get(), nullptr, selectedColor);

				mapName = ">" + mapName + "<";
			}
			else
			{
				SDL_FillRect (imageSurface.get(), nullptr, unselectedColor);
			}
			SDL_Rect dest = {4, 4, mapWinSize, mapWinSize};
			SDL_BlitSurface (preview.surface.get(), nullptr, imageSurface.get(), &dest);

			mapImages[i]->setImage (imageSurface.get());
			mapTitles[i]->setText (mapName);
		}
		else
		{
			mapImages[i]->setImage (nullptr);
			mapTitles[i]->setText ("");
		}
	}
}

//------------------------------------------------------------------------------
void cWindowMapSelection::loadMaps()
{
	maps = getFilesOfDirectory (cSettings::getInstance().getMapsPath());
	if (!getUserMapsDir().empty())
	{
		std::vector<std::string> userMaps (getFilesOfDirectory (getUserMapsDir()));
		for (const auto& userMap : userMaps)
		{
			if (!Contains (maps, userMap))
			{
				maps.push_back (userMap);
			}
		}
	}
	EraseIf (maps, [](const std::string& mapName){ return mapName.compare (mapName.length() - 3, 3, "WRL") != 0 && mapName.compare (mapName.length() - 3, 3, "wrl") != 0;});
}
