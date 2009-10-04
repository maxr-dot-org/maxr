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

/**
 Shows localized Yes/No dialog. The show function returns 0 for yes and 1 for no.
 */
class cDialogYesNo : public cMenu
{
public:
	cDialogYesNo(string text);

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	static void yesReleased( void *parent );
	static void noReleased( void *parent );

private:
	cMenuLabel  textLabel;
	cMenuButton yesButton;
	cMenuButton noButton;
};

/**
 * Shows dialogbox with localized OK button
 */
class cDialogOK : public cMenu
{
public:
	cDialogOK(string text);

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	static void okReleased( void *parent );

private:
	cMenuLabel  textLabel;
	cMenuButton okButton;
};

/**
 * Shows licence infobox refering to hardcoded GPL-notation and warranty information
 */
class cDialogLicence : public cMenu
{
public:
	cDialogLicence();

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	static void okReleased( void *parent );
	static void upReleased( void *parent );
	static void downReleased( void *parent );

private:
	string sLicence1;
	string sLicence2;
	string sLicence3;
	string sLicence4;
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
	string resolutions[6];
	int oldMusicVolume, oldEffectsVolume, oldVoicesVolume;
	bool oldMusicMute, oldEffectsMute, oldVoicesMute;

	cMenuLabel *titleLabel;

	cMenuLabel *volumeLabel;
	cMenuLabel *musicLabel;
	cMenuLabel *effectsLabel;
	cMenuLabel *voicesLabel;
	cMenuCheckButton *disableMusicChBox;
	cMenuCheckButton *disableEffectsChBox;
	cMenuCheckButton *disableVoicesChBox;
	cMenuSlider *musicSlider;
	cMenuSlider *effectsSlider;
	cMenuSlider *voicesSlider;

	cMenuLabel *nameLabel;
	cMenuLineEdit *nameEdit;

	cMenuCheckButton *animationChBox;
	cMenuCheckButton *shadowsChBox;
	cMenuCheckButton *alphaChBox;
	cMenuCheckButton *demageBuilChBox;
	cMenuCheckButton *demageVehChBox;
	cMenuCheckButton *tracksChBox;

	cMenuLabel *scrollSpeedLabel;
	cMenuSlider *scrollSpeedSlider;

	cMenuCheckButton *autosaveChBox;
	cMenuCheckButton *introChBox;
	cMenuCheckButton *windowChBox;

	cMenuRadioGroup *resoulutionGroup;

	cMenuButton *okButton;
	cMenuButton *cancelButton;

	void saveValues();
public:
	cDialogPreferences();
	~cDialogPreferences();

	static void okReleased( void *parent );
	static void cancelReleased( void *parent );

	static void musicVolumeChanged( void *parent );
	static void effectsVolumeChanged( void *parent );
	static void voicesVolumeChanged( void *parent );

	static void musicMuteChanged( void *parent );
	static void effectsMuteChanged( void *parent );
	static void voicesMuteChanged( void *parent );
};

class cDialogTransfer : public cMenu
{
	cBuilding *srcBuilding, *destBuilding;
	cVehicle *srcVehicle, *destVehicle;

	cMenuMaterialBar::eMaterialBarTypes transferType;
	int srcCargo, maxSrcCargo;
	int destCargo, maxDestCargo;
	int transferValue;

	cMenuButton *doneButton;
	cMenuButton *cancelButton;

	cMenuButton *incButton;
	cMenuButton *decButton;

	cMenuMaterialBar *resBar;

	cMenuImage *unitImages[2];

	cMenuLabel *unitNameLabels[2];
	cMenuLabel *unitCargoLabels[2];
	cMenuLabel *transferLabel;

	void getTransferType();
	void getNamesNCargoNImages();
	void setCargos();
public:
	cDialogTransfer( cBuilding *srcBuilding_, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_ );
	~cDialogTransfer();

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	static void doneReleased( void *parent );
	static void cancelReleased( void *parent );

	static void incReleased( void *parent );
	static void decReleased( void *parent );

	static void barClicked( void *parent );

	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

class cDialogResearch : public cMenu
{
	cPlayer *owner;
	int newResearchSettings[cResearch::kNrResearchAreas];
	int unusedResearch;

	cMenuLabel *titleLabel;

	cMenuLabel *centersLabel;
	cMenuLabel *themeLabel;
	cMenuLabel *turnsLabel;

	cMenuButton *doneButton;
	cMenuButton *cancelButton;

	cMenuButton *incButtons[cResearch::kNrResearchAreas];
	cMenuButton *decButtons[cResearch::kNrResearchAreas];

	cMenuScrollerHandler *scroller[cResearch::kNrResearchAreas];

	cMenuLabel *centerCountLabels[cResearch::kNrResearchAreas];
	cMenuLabel *themeNameLabels[cResearch::kNrResearchAreas];
	cMenuLabel *percentageLabels[cResearch::kNrResearchAreas];
	cMenuLabel *turnsLabels[cResearch::kNrResearchAreas];

	void setData();
public:
	cDialogResearch( cPlayer *owner_ );
	~cDialogResearch();

	void handleKeyInput( SDL_KeyboardEvent &key, string ch );

	static void doneReleased( void *parent );
	static void cancelReleased( void *parent );

	static void incReleased( void *parent );
	static void decReleased( void *parent );

	static void sliderClicked( void *parent );

	void handleDestroyUnit( cBuilding *destroyedBuilding = NULL, cVehicle *destroyedVehicle = NULL );
};

/** Draws a context menu item
 * @author beko
 * @param sText Text to display on item
 * @param bPressed clickstatus
 * @param x x position
 * @param y y position
 * @param *surface SDL_Surface to draw on
*/
void drawContextItem(std::string sText, bool bPressed, int x, int y, SDL_Surface *surface);

#endif
