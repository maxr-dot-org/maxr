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
#include "main.h"
#include "pcx.h"
#include "utility/files.h"
#include "game/data/map/map.h"
#include "utility/autosurface.h"
#include "video.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/image.h"

//------------------------------------------------------------------------------
cWindowMapSelection::cWindowMapSelection() :
	cWindow (LoadPCX (GFXOD_PLANET_SELECT)),
	selectedMapIndex (-1),
	page (0)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 13), getPosition() + cPosition (getArea().getMaxCorner().x(), 23)), lngPack.i18n ("Text~Title~Choose_Planet"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	//
	// Map Images
	//
	for (size_t row = 0; row < mapRows; ++row)
	{
		for (size_t column = 0; column < mapColumns; ++column)
		{
			const auto index = row * mapColumns + column;

			mapImages[index] = addChild (std::make_unique<cImage> (getPosition() + cPosition (21 + 158 * column, 86 + 198 * row), nullptr, &SoundData.SNDHudButton));
			signalConnectionManager.connect (mapImages[index]->clicked, std::bind (&cWindowMapSelection::mapClicked, this, mapImages[index]));

			mapTitles[index] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (6 + 158 * column, 48 + 198 * row), getPosition() + cPosition (155 + 158 * column, 48 + 10 + 198 * row)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
		}
	}

	//
	// Buttons
	//
	upButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (292, 435), ePushButtonType::ArrowUpBig));
	signalConnectionManager.connect (upButton->clicked, std::bind (&cWindowMapSelection::upClicked, this));

	downButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (321, 435), ePushButtonType::ArrowDownBig));
	signalConnectionManager.connect (downButton->clicked, std::bind (&cWindowMapSelection::downClicked, this));

	okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~OK")));
	okButton->lock();
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowMapSelection::okClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (50, 440), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowMapSelection::backClicked, this));

	loadMaps();
	updateMaps();

	updateUpDownLocked();
}

//------------------------------------------------------------------------------
cWindowMapSelection::~cWindowMapSelection()
{}

//------------------------------------------------------------------------------
void cWindowMapSelection::mapClicked (const cImage* mapImage)
{
	for (size_t i = 0; i < mapImages.size(); ++i)
	{
		if (mapImage == mapImages[i])
		{
			selectedMapIndex = static_cast<int> (page * mapCount + i);
			break;
		}
	}
	updateMaps();
	if (selectedMapIndex != -1) okButton->unlock();
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
	done();
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
	for (size_t i = 0; i < mapCount; ++i)
	{
		const auto mapIndex = page * mapCount + i;
		if (mapIndex < maps.size())
		{
			auto mapName = maps[mapIndex];

			int size;
			AutoSurface mapSurface (cStaticMap::loadMapPreview (mapName, &size));

			if (mapSurface == nullptr) continue;

			const int mapWinSize = 112;
			const int selectedColor = 0x00C000;
			const int unselectedColor = 0x000000;
			AutoSurface imageSurface (SDL_CreateRGBSurface (0, mapWinSize + 8, mapWinSize + 8, Video.getColDepth(), 0, 0, 0, 0));

			if (selectedMapIndex == static_cast<int> (mapIndex))
			{
				SDL_FillRect (imageSurface.get(), nullptr, selectedColor);

				if (font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
				{
					while (font->getTextWide (">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
					{
						mapName.erase (mapName.length() - 1, mapName.length());
					}
					mapName = ">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<";
				}
				else mapName = ">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<";
			}
			else
			{
				SDL_FillRect (imageSurface.get(), nullptr, unselectedColor);

				if (font->getTextWide (">" + mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
				{
					while (font->getTextWide (">" + mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")<") > 140)
					{
						mapName.erase (mapName.length() - 1, mapName.length());
					}
					mapName = mapName + "... (" + iToStr (size) + "x" + iToStr (size) + ")";
				}
				else mapName = mapName.substr (0, mapName.length() - 4) + " (" + iToStr (size) + "x" + iToStr (size) + ")";
			}
			SDL_Rect dest = {4, 4, mapWinSize, mapWinSize};
			SDL_BlitSurface (mapSurface.get(), nullptr, imageSurface.get(), &dest);

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
		for (size_t i = 0; i != userMaps.size(); ++i)
		{
			if (std::find (maps.begin(), maps.end(), userMaps[i]) == maps.end())
			{
				maps.push_back (userMaps[i]);
			}
		}
	}
	for (size_t i = 0; i != maps.size(); ++i)
	{
		const std::string& map = maps[i];
		if (map.compare (map.length() - 3, 3, "WRL") != 0 && map.compare (map.length() - 3, 3, "wrl") != 0)
		{
			maps.erase (maps.begin() + i);
			i--;
		}
	}
}

//------------------------------------------------------------------------------
std::string cWindowMapSelection::getSelectedMapName() const
{
	if (selectedMapIndex < 0 || selectedMapIndex >= static_cast<int> (maps.size())) return "";

	return maps[selectedMapIndex];
}

//------------------------------------------------------------------------------
bool cWindowMapSelection::loadSelectedMap (cStaticMap& staticMap)
{
	if (selectedMapIndex < 0 || selectedMapIndex >= static_cast<int> (maps.size())) return false;

	return staticMap.loadMap (maps[selectedMapIndex]);
}
