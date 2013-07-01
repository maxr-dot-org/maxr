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
#ifndef dialogH
#define dialogH

#include "defines.h"
#include "menus.h"
#include "autoptr.h"

/**
 Shows localized Yes/No dialog. The show function returns 0 for yes and 1 for no.
 */
class cDialogYesNo : public cMenu
{
public:
	explicit cDialogYesNo (const std::string& text);

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);

private:
	cMenuLabel textLabel;
	cMenuButton yesButton;
	cMenuButton noButton;
};

/**
 * Shows dialogbox with localized OK button
 */
class cDialogOK : public cMenu
{
public:
	explicit cDialogOK (const std::string& text);

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);

private:
	cMenuLabel textLabel;
	cMenuButton okButton;
};

class cDestructMenu : public cMenu
{
public:
	cDestructMenu();

private:
	static void armReleased (void* parent);

private:
	cMenuButton armButton;
	cMenuButton cancelButton;
	cMenuDestroyButton destroyButton;
};

/**
 * Shows licence infobox refering to hardcoded GPL-notation and warranty information
 */
class cDialogLicence : public cMenu
{
public:
	cDialogLicence();

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);

private:
	static void upReleased (void* parent);
	static void downReleased (void* parent);

private:
	std::string sLicence1;
	std::string sLicence2;
	std::string sLicence3;
	std::string sLicence4;
	int offset;

	cMenuLabel maxrLabel;
	cMenuLabel headerLabel;
	cMenuLabel textLabel;

	cMenuButton okButton;
	cMenuButton upButton;
	cMenuButton downButton;

	void generateLicenceTexts();
	void resetText();
};

/**
* Shows localized preferences dialog
*/
class cDialogPreferences : public cMenu
{
	cPlayer* player;
	int oldMusicVolume, oldEffectsVolume, oldVoicesVolume;
	bool oldMusicMute, oldEffectsMute, oldVoicesMute;

	cMenuLabel titleLabel;

	cMenuLabel volumeLabel;
	cMenuLabel musicLabel;
	cMenuLabel effectsLabel;
	cMenuLabel voicesLabel;
	cMenuCheckButton disableMusicChBox;
	cMenuCheckButton disableEffectsChBox;
	cMenuCheckButton disableVoicesChBox;
	cMenuSlider musicSlider;
	cMenuSlider effectsSlider;
	cMenuSlider voicesSlider;

	cMenuLabel nameLabel;
	cMenuLineEdit nameEdit;

	cMenuCheckButton animationChBox;
	cMenuCheckButton shadowsChBox;
	cMenuCheckButton alphaChBox;
	cMenuCheckButton demageBuilChBox;
	cMenuCheckButton demageVehChBox;
	cMenuCheckButton tracksChBox;

	cMenuLabel scrollSpeedLabel;
	cMenuSlider scrollSpeedSlider;

	cMenuCheckButton autosaveChBox;
	cMenuCheckButton introChBox;
	cMenuCheckButton windowChBox;

	cMenuRadioGroup resoulutionGroup;

	cMenuButton okButton;
	cMenuButton cancelButton;

	void saveValues();
public:
	explicit cDialogPreferences (cPlayer* player_);

private:
	static void okReleased (void* parent);
	static void cancelReleased (void* parent);

	static void musicVolumeChanged (void* parent);
	static void effectsVolumeChanged (void* parent);
	static void voicesVolumeChanged (void* parent);

	static void musicMuteChanged (void* parent);
	static void effectsMuteChanged (void* parent);
	static void voicesMuteChanged (void* parent);
};

class cDialogTransfer : public cMenu
{
	cGameGUI* gameGUI;
	cUnit* srcUnit;
	cUnit* destUnit;

	cMenuMaterialBar::eMaterialBarTypes transferType;
	int srcCargo, maxSrcCargo;
	int destCargo, maxDestCargo;
	int transferValue;

	cMenuButton doneButton;
	cMenuButton cancelButton;

	cMenuButton incButton;
	cMenuButton decButton;

	AutoPtr<cMenuMaterialBar> resBar;

	AutoPtr<cMenuImage> unitImages[2];

	AutoPtr<cMenuLabel> unitNameLabels[2];
	AutoPtr<cMenuLabel> unitCargoLabels[2];
	cMenuLabel transferLabel;
	cMenuImage arrowImage;

	static sUnitData::eStorageResType getCommonStorageType (const cUnit& unit1, const cUnit& unit2);
	void getTransferType();
	void getNamesNCargoNImages();
	void setCargos();
public:
	cDialogTransfer (cGameGUI& gameGUI_, cUnit& srcUnit_, cUnit& destUnit_);
	~cDialogTransfer();

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void doneReleased (void* parent);

	static void incReleased (void* parent);
	static void decReleased (void* parent);

	static void barClicked (void* parent);
};

class cDialogResearch : public cMenu
{
	cClient* client;
	cPlayer* owner;
	int newResearchSettings[cResearch::kNrResearchAreas];
	int unusedResearch;

	cMenuLabel titleLabel;

	cMenuLabel centersLabel;
	cMenuLabel themeLabel;
	cMenuLabel turnsLabel;

	cMenuButton doneButton;
	cMenuButton cancelButton;

	AutoPtr<cMenuButton> incButtons[cResearch::kNrResearchAreas];
	AutoPtr<cMenuButton> decButtons[cResearch::kNrResearchAreas];

	AutoPtr<cMenuScrollerHandler> scroller[cResearch::kNrResearchAreas];

	AutoPtr<cMenuLabel> centerCountLabels[cResearch::kNrResearchAreas];
	AutoPtr<cMenuLabel> themeNameLabels[cResearch::kNrResearchAreas];
	AutoPtr<cMenuLabel> percentageLabels[cResearch::kNrResearchAreas];
	AutoPtr<cMenuLabel> turnsLabels[cResearch::kNrResearchAreas];

	void setData();
public:
	explicit cDialogResearch (cClient& client_, cPlayer* owner_);

private:
	virtual void handleKeyInput (SDL_KeyboardEvent& key, const std::string& ch);
	virtual void handleDestroyUnit (cUnit& destroyedUnit);

private:
	static void doneReleased (void* parent);

	static void incReleased (void* parent);
	static void decReleased (void* parent);

	static void sliderClicked (void* parent);
};

/** Draws a context menu item
 * @author beko
 * @param sText Text to display on item
 * @param bPressed clickstatus
 * @param x x position
 * @param y y position
 * @param *surface SDL_Surface to draw on
*/
void drawContextItem (const std::string& sText, bool bPressed, int x, int y, SDL_Surface* surface);

#endif
