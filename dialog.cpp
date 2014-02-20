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
#include <SDL.h>
#include <SDL_mixer.h>
#include <sstream>
#include <algorithm>

#include "dialog.h"

#include "buildings.h"
#include "client.h"
#include "clientevents.h"
#include "events.h"
#include "files.h"
#include "hud.h"
#include "loaddata.h"
#include "log.h"
#include "pcx.h"
#include "player.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "vehicles.h"
#include "video.h"

#include "input/mouse/mouse.h"

using namespace std;

#if defined (_MSC_VER)
// The purpose of this warning is to avoid to use not fully constructed object.
// As we use 'this' only to save back reference
// and use it later once object is fully initialized
// just ignore the warning.
# pragma warning (disable:4355) // 'this': used in base member initializer list
// appear in cDestructMenu constructor
// appear in cDialogPreferences constructor
#endif



cDialogYesNo::cDialogYesNo (const string& text) :
	cMenu (LoadPCX (GFXOD_DIALOG2), MNU_BG_ALPHA),
	textLabel (position.x + 40, position.y + 40, text),
	yesButton (position.x + 155, position.y + 185, lngPack.i18n ("Text~Others~Yes"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	noButton (position.x +  67, position.y + 185, lngPack.i18n ("Text~Others~No"),  cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	textLabel.setBox (232, 142);
	//textLabel.setBox(position.w - 40, position.h - 150);
	menuItems.push_back (&textLabel);

	yesButton.setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (&yesButton);

	noButton.setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (&noButton);
}

void cDialogYesNo::handleKeyInput (const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
		case SDLK_RETURN:
			switch (key.state)
			{
				case SDL_PRESSED:  if (!yesButton.getIsClicked()) yesButton.clicked (this);  break;
				case SDL_RELEASED: if (yesButton.getIsClicked())  yesButton.released (this); break;
			}
			break;
		case SDLK_ESCAPE:
			switch (key.state)
			{
				case SDL_PRESSED:  if (!noButton.getIsClicked()) noButton.clicked (this);  break;
				case SDL_RELEASED: if (noButton.getIsClicked())  noButton.released (this); break;
			}
			break;
		default:
			break;
	}
}

cDialogOK::cDialogOK (const string& text) :
	cMenu (LoadPCX (GFXOD_DIALOG2), MNU_BG_ALPHA),
	textLabel (position.x + 40, position.y + 40, text),
	okButton (position.x + 111, position.y + 185, lngPack.i18n ("Text~Others~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	textLabel.setBox (232, 142);
	menuItems.push_back (&textLabel);

	okButton.setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (&okButton);
}

void cDialogOK::handleKeyInput (const SDL_KeyboardEvent& key)
{
	if (key.keysym.sym == SDLK_RETURN)
	{
		switch (key.state)
		{
			case SDL_PRESSED:  if (!okButton.getIsClicked()) okButton.clicked (this);  break;
			case SDL_RELEASED: if (okButton.getIsClicked())  okButton.released (this); break;
		}
	}
}

cDestructMenu::cDestructMenu() :
	cMenu (LoadPCX (GFXOD_DESTRUCTION), MNU_BG_ALPHA),
	armButton (position.x + 88, position.y + 14, lngPack.i18n ("Text~Others~Hot"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	cancelButton (position.x + 88, position.y + 46, lngPack.i18n ("Text~Others~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	destroyButton (position.x + 15, position.y + 13, this)
{
	cancelButton.setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (&cancelButton);

	armButton.setReleasedFunction (&armReleased);
	menuItems.push_back (&armButton);

	destroyButton.setReleasedFunction (&cMenu::doneReleased);
	destroyButton.setReleaseSound (SoundData.SNDMenuButton);
	menuItems.push_back (&destroyButton);
}

void cDestructMenu::armReleased (void* parent)
{
	cDestructMenu* menu = reinterpret_cast<cDestructMenu*> (parent);
	menu->armButton.setLocked (true);
	menu->destroyButton.setLocked (false);
}

cDialogLicence::cDialogLicence() :
	cMenu (LoadPCX (GFXOD_DIALOG4), MNU_BG_ALPHA),
	maxrLabel (position.x + position.w / 2, position.y +  30, "\"M.A.X.R.\""),
	headerLabel (position.x + position.w / 2, position.y +  30 + font->getFontHeight(), "(C) 2007 by its authors"),
	textLabel (position.x +  35, position.y +  30 + 3 * font->getFontHeight()),
	okButton (position.x + 111, position.y + 185, lngPack.i18n ("Text~Others~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	upButton (position.x + 241, position.y + 187, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL),
	downButton (position.x + 261, position.y + 187, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL)
{
	generateLicenceTexts();

	maxrLabel.setCentered (true);
	menuItems.push_back (&maxrLabel);

	headerLabel.setCentered (true);
	menuItems.push_back (&headerLabel);

	textLabel.setBox (232, 142);
	menuItems.push_back (&textLabel);

	okButton.setReleasedFunction (&cMenu::doneReleased);
	menuItems.push_back (&okButton);

	upButton.setReleasedFunction (&upReleased);
	menuItems.push_back (&upButton);

	downButton.setReleasedFunction (&downReleased);
	menuItems.push_back (&downButton);

	offset = 0;
	resetText();
}

void cDialogLicence::generateLicenceTexts()
{
	sLicence1 =
		"  This program is free software; you can redistribute it and/or modify "
		"it under the terms of the GNU General Public License as published by "
		"the Free Software Foundation; either version 2 of the License, or "
		"(at your option) any later version.";
	sLicence2 =
		"  This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.";
	sLicence3 =
		"  You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";

	//open AUTHOR
	string sAuthors;
#ifdef WIN32
	sAuthors = "AUTHORS.txt";
#elif __amigaos4
	sAuthors = cSettings::getInstance().getDataDir() + PATH_DELIMITER + "AUTHORS.txt";
#elif MAC
	sAuthors = "AUTHORS";
#else
	sAuthors = cSettings::getInstance().getDataDir() + PATH_DELIMITER + "AUTHORS";
#endif

	sLicence4 = "";
	char line[72];

	FILE* const fp = fopen (sAuthors.c_str(), "r");
	if (fp != NULL)
	{
		//read authors from file
		while (fgets (line, 72, fp))    //snip entrys longer 72
		{
			sLicence4 += line;
		}
		fclose (fp);
	}
	else sLicence4 = "Couldn't read AUTHORS"; //missing file - naughty
}

void cDialogLicence::resetText()
{
	switch (offset)
	{
		case 0:
			textLabel.setText (sLicence1);
			upButton.setLocked (true);
			break;
		case 1:
			textLabel.setText (sLicence2);
			upButton.setLocked (false);
			break;
		case 2:
			textLabel.setText (sLicence3);
			textLabel.setFontType (FONT_LATIN_NORMAL);
			downButton.setLocked (false);
			headerLabel.setText ("(C) 2007 by its authors");
			break;
		case 3:
			textLabel.setText (sLicence4);
			textLabel.setFontType (FONT_LATIN_SMALL_WHITE);
			downButton.setLocked (true);
			headerLabel.setText ("AUTHORS:");
			break;
	}
	draw();
}

void cDialogLicence::handleKeyInput (const SDL_KeyboardEvent& key)
{
	if (key.keysym.sym == SDLK_RETURN)
	{
		switch (key.state)
		{
			case SDL_PRESSED:  if (!okButton.getIsClicked()) okButton.clicked (this);  break;
			case SDL_RELEASED: if (okButton.getIsClicked())  okButton.released (this); break;
		}
	}
}

void cDialogLicence::upReleased (void* parent)
{
	cDialogLicence* menu = reinterpret_cast<cDialogLicence*> (parent);
	if (menu->offset > 0) menu->offset--;
	menu->resetText();
}

void cDialogLicence::downReleased (void* parent)
{
	cDialogLicence* menu = reinterpret_cast<cDialogLicence*> (parent);
	if (menu->offset < 3) menu->offset++;
	menu->resetText();
}

cDialogPreferences::cDialogPreferences (cPlayer* player_) : cMenu (LoadPCX (GFXOD_DIALOG5), MNU_BG_ALPHA),
	player (player_),
	titleLabel (position.x + position.w / 2, position.y + 15, lngPack.i18n ("Text~Settings~Preferences")),
	volumeLabel (position.x + 25, position.y + 56, lngPack.i18n ("Text~Settings~Volume") + ":"),
	musicLabel (position.x + 25, position.y + 56 + 20, lngPack.i18n ("Text~Settings~Music")),
	effectsLabel (position.x + 25, position.y + 56 + 20 * 2, lngPack.i18n ("Text~Settings~Effects")),
	voicesLabel (position.x + 25, position.y + 56 + 20 * 3, lngPack.i18n ("Text~Settings~Voices")),
	disableMusicChBox (position.x + 210, position.y + 73, lngPack.i18n ("Text~Settings~Disable"), cSettings::getInstance().isMusicMute(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	disableEffectsChBox (position.x + 210, position.y + 73 + 20, lngPack.i18n ("Text~Settings~Disable"), cSettings::getInstance().isSoundMute(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	disableVoicesChBox (position.x + 210, position.y + 73 + 20 * 2, lngPack.i18n ("Text~Settings~Disable"), cSettings::getInstance().isVoiceMute(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	musicSlider (position.x + 140, position.y + 81, 0, 128, this),
	effectsSlider (position.x + 140, position.y + 81 + 20, 0, 128, this),
	voicesSlider (position.x + 140, position.y + 81 + 20 * 2, 0, 128, this),
	nameLabel (position.x + 25, position.y + 158, lngPack.i18n ("Text~Title~Player_Name")),
	nameEdit (position.x + 112, position.y + 154, 185, 18, this),
	animationChBox (position.x + 25, position.y + 193, lngPack.i18n ("Text~Settings~Animation"), cSettings::getInstance().isAnimations(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	shadowsChBox (position.x + 25, position.y + 193 + 20, lngPack.i18n ("Text~Settings~Shadows"), cSettings::getInstance().isShadows(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	alphaChBox (position.x + 25, position.y + 193 + 20 * 2, lngPack.i18n ("Text~Settings~Alphaeffects"), cSettings::getInstance().isAlphaEffects(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	demageBuilChBox (position.x + 210, position.y + 193, lngPack.i18n ("Text~Settings~ShowDamage"), cSettings::getInstance().isDamageEffects(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	demageVehChBox (position.x + 210, position.y + 193 + 20, lngPack.i18n ("Text~Settings~ShowDamageVehicle"), cSettings::getInstance().isDamageEffectsVehicles(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	tracksChBox (position.x + 210, position.y + 193 + 20 * 2, lngPack.i18n ("Text~Settings~Tracks"), cSettings::getInstance().isMakeTracks(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	scrollSpeedLabel (position.x + 25, position.y + 232 + 25, lngPack.i18n ("Text~Settings~Scrollspeed")),
	scrollSpeedSlider (position.x + 140, position.y + 261, 0, 250, this),
	autosaveChBox (position.x + 25, position.y + 294, lngPack.i18n ("Text~Settings~Autosave"), cSettings::getInstance().shouldAutosave(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	introChBox (position.x + 25, position.y + 294 + 20, lngPack.i18n ("Text~Settings~Intro"), cSettings::getInstance().shouldShowIntro(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	windowChBox (position.x + 25, position.y + 294 + 20 * 2, lngPack.i18n ("Text~Settings~Window"), Video.getWindowMode(), false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD),
	resoulutionGroup(),
	okButton (position.x + 208, position.y + 383, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	cancelButton (position.x + 118, position.y + 383, lngPack.i18n ("Text~Others~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	// blit black titlebar behind textfield for playername
	SDL_Rect src = { 108, 12, 186, 18 };
	SDL_Rect dest = { 108, 154, 0, 0 };
	SDL_BlitSurface (background, &src, background, &dest);

	titleLabel.setCentered (true);
	menuItems.push_back (&titleLabel);
	menuItems.push_back (&volumeLabel);
	menuItems.push_back (&musicLabel);
	menuItems.push_back (&effectsLabel);
	menuItems.push_back (&voicesLabel);

	disableMusicChBox.setClickedFunction (&musicMuteChanged);
	menuItems.push_back (&disableMusicChBox);
	disableEffectsChBox.setClickedFunction (&effectsMuteChanged);
	menuItems.push_back (&disableEffectsChBox);
	disableVoicesChBox.setClickedFunction (&voicesMuteChanged);
	menuItems.push_back (&disableVoicesChBox);

	musicSlider.setValue ((float) cSettings::getInstance().getMusicVol());
	musicSlider.setMoveCallback (&musicVolumeChanged);
	menuItems.push_back (&musicSlider);
	menuItems.push_back (musicSlider.scroller.get());

	effectsSlider.setValue ((float) cSettings::getInstance().getSoundVol());
	effectsSlider.setMoveCallback (&effectsVolumeChanged);
	menuItems.push_back (&effectsSlider);
	menuItems.push_back (effectsSlider.scroller.get());

	voicesSlider.setValue ((float) cSettings::getInstance().getVoiceVol());
	voicesSlider.setMoveCallback (&voicesVolumeChanged);
	menuItems.push_back (&voicesSlider);
	menuItems.push_back (voicesSlider.scroller.get());

	menuItems.push_back (&nameLabel);

	nameEdit.setText (cSettings::getInstance().getPlayerName());
	menuItems.push_back (&nameEdit);

	menuItems.push_back (&animationChBox);
	menuItems.push_back (&shadowsChBox);
	menuItems.push_back (&alphaChBox);
	menuItems.push_back (&demageBuilChBox);
	menuItems.push_back (&demageVehChBox);
	menuItems.push_back (&tracksChBox);

	menuItems.push_back (&scrollSpeedLabel);
	scrollSpeedSlider.setValue ((float) cSettings::getInstance().getScrollSpeed());
	menuItems.push_back (&scrollSpeedSlider);
	menuItems.push_back (scrollSpeedSlider.scroller.get());

	menuItems.push_back (&autosaveChBox);
	menuItems.push_back (&introChBox);
	menuItems.push_back (&windowChBox);

	//BEGIN SCREEN RESOLUTION CHECKBOXES
	//FIXME: need dropdown box item for this. This is very dirty code fixed to 9 possible resolution values. Odd things might occur if less than 9 useable screen resolutions are found here.
	//HINT: This works only as long as avail video modes have a neat follow up list starting with 0 so make sure that the modes vector doesn't get confused

	int resolutionMode = Video.validateMode (Video.getResolutionX(), Video.getResolutionY());  //set flagged box to current resolution if found

	if (Video.getVideoSize() <= 0)
	{
		Log.write ("No resolutions could be detected. Can't display any options here.",  cLog::eLOG_TYPE_ERROR);
	}
	else //we got resolutions from SDL so we use the detected ones here and overwrite the default ones from above (but not our default minimal resolution)
	{
		for (int i = 0, x = 0; x < 3; x++)
		{
			for (int y = 0; y < 4; y++, i++)
			{
				if (i >= Video.getVideoSize())
				{
					Log.write ("Oops, looks like we read less resolutions than I should offer. This might result in some glitches in my dialog. I want a drop down box here!",  cLog::eLOG_TYPE_WARNING);
					cMenuCheckButton* button = new cMenuCheckButton (position.x + 150 + 80 * x, position.y + 290 + 20 * y, "<empty>", resolutionMode == i, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
					resoulutionGroup.addButton (button);

				}
				else
				{
					Log.write ("Offering display resolution " + Video.getVideoMode (i) + " to user", cLog::eLOG_TYPE_DEBUG);
					cMenuCheckButton* button = new cMenuCheckButton (position.x + 150 + 80 * x, position.y + 290 + 20 * y, Video.getVideoMode (i), resolutionMode == i, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD);
					resoulutionGroup.addButton (button);

				}
			}
		}

		if (Video.getVideoSize() > 12)   //notice: we skip mode 10. to much on screen. bad luck until we get a drop down box or similar
		{
			Log.write ("Read more possible resolutions than I can display on dialog. Stopped.",  cLog::eLOG_TYPE_WARNING);
		}
	}

	menuItems.push_back (&resoulutionGroup);

	//END SCREEN RESOLUTION CHECKBOXES

	okButton.setReleasedFunction (&okReleased);
	menuItems.push_back (&okButton);

	cancelButton.setReleasedFunction (&cancelReleased);
	menuItems.push_back (&cancelButton);

	// save old volumes
	oldMusicVolume = cSettings::getInstance().getMusicVol();
	oldEffectsVolume = cSettings::getInstance().getSoundVol();
	oldVoicesVolume = cSettings::getInstance().getVoiceVol();

	oldMusicMute = cSettings::getInstance().isMusicMute();
	oldEffectsMute = cSettings::getInstance().isSoundMute();
	oldVoicesMute = cSettings::getInstance().isVoiceMute();
}

void cDialogPreferences::saveValues()
{
	cSettings::getInstance().setPlayerName (nameEdit.getText().c_str());
	if (player) player->setName (cSettings::getInstance().getPlayerName());

	cSettings::getInstance().setAutosave (autosaveChBox.isChecked());
	cSettings::getInstance().setAnimations (animationChBox.isChecked());
	cSettings::getInstance().setAlphaEffects (alphaChBox.isChecked());
	cSettings::getInstance().setDamageEffects (demageBuilChBox.isChecked());
	cSettings::getInstance().setDamageEffectsVehicles (demageVehChBox.isChecked());
	cSettings::getInstance().setShowIntro (introChBox.isChecked());
	cSettings::getInstance().setMakeTracks (tracksChBox.isChecked());
	Video.setWindowMode (windowChBox.isChecked());
	cSettings::getInstance().saveWindowMode();
	cSettings::getInstance().setShadows (shadowsChBox.isChecked());

	cSettings::getInstance().setScrollSpeed ((int) scrollSpeedSlider.getValue());

	// save resolution
	int oldScreenW = Video.getResolutionX();
	int oldScreenH = Video.getResolutionY();

	for (int i = 0; i < 12; i++)
	{
		if (resoulutionGroup.buttonIsChecked (i))
		{
			string sTmp = Video.getVideoMode (i);
			int wTmp = atoi (sTmp.substr (0, sTmp.find_first_of ('x')).c_str());
			int hTmp = atoi (sTmp.substr (sTmp.find_first_of ('x') + 1, sTmp.size()).c_str());
			Video.setResolution (wTmp, hTmp, true);
			cSettings::getInstance().saveResolution();

			if (Video.getResolutionX() != oldScreenW || Video.getResolutionY() != oldScreenH)
			{
				//TODO: Tidy up here with oldScreen blabla
				//Video.setResolution(oldScreenW, oldScreenH);
				cDialogOK okDialog (lngPack.i18n ("Text~Comp~ResolutionChange"));
				switchTo(okDialog);
				break;
			}
		}
	}
}

void cDialogPreferences::okReleased (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	menu->saveValues();
	menu->end = true;
}

void cDialogPreferences::cancelReleased (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);

	// restore old volumes
	cSettings::getInstance().setMusicVol (menu->oldMusicVolume);
	cSettings::getInstance().setSoundVol (menu->oldEffectsVolume);
	cSettings::getInstance().setVoiceVol (menu->oldVoicesVolume);
	if (cSettings::getInstance().isSoundEnabled()) Mix_VolumeMusic (cSettings::getInstance().getMusicVol());
	if (cSettings::getInstance().isSoundEnabled()) Mix_Volume (SoundLoopChannel, cSettings::getInstance().getSoundVol());

	bool wasMusicMute = cSettings::getInstance().isMusicMute();
	cSettings::getInstance().setMusicMute (menu->oldMusicMute);
	if (wasMusicMute && !menu->oldMusicMute) StartMusic();
	cSettings::getInstance().setSoundMute (menu->oldEffectsMute);
	cSettings::getInstance().setVoiceMute (menu->oldVoicesMute);

	menu->terminate = true;
}

void cDialogPreferences::musicVolumeChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	cSettings::getInstance().setMusicVol ((int) menu->musicSlider.getValue());
	if (cSettings::getInstance().isSoundEnabled()) Mix_VolumeMusic (cSettings::getInstance().getMusicVol());
}

void cDialogPreferences::effectsVolumeChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	cSettings::getInstance().setSoundVol ((int) menu->effectsSlider.getValue());
	if (cSettings::getInstance().isSoundEnabled()) Mix_Volume (SoundLoopChannel, cSettings::getInstance().getSoundVol());
}

void cDialogPreferences::voicesVolumeChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	cSettings::getInstance().setVoiceVol ((int) menu->voicesSlider.getValue());
}

void cDialogPreferences::musicMuteChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	bool wasMute = cSettings::getInstance().isMusicMute();
	cSettings::getInstance().setMusicMute (menu->disableMusicChBox.isChecked());
	if (cSettings::getInstance().isMusicMute()) StopMusic();
	if (!cSettings::getInstance().isMusicMute() && wasMute) StartMusic();
}

void cDialogPreferences::effectsMuteChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	cSettings::getInstance().setSoundMute (menu->disableEffectsChBox.isChecked());
}

void cDialogPreferences::voicesMuteChanged (void* parent)
{
	cDialogPreferences* menu = reinterpret_cast<cDialogPreferences*> (parent);
	cSettings::getInstance().setVoiceMute (menu->disableVoicesChBox.isChecked());
}

cDialogTransfer::cDialogTransfer (cGameGUI& gameGUI_, cUnit& srcUnit_, cUnit& destUnit_) :
	cMenu (LoadPCX (GFXOD_DIALOG_TRANSFER), MNU_BG_ALPHA),
	gameGUI (&gameGUI_),
	srcUnit (&srcUnit_),
	destUnit (&destUnit_),
	doneButton (position.x + 159, position.y + 200, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	cancelButton (position.x + 71, position.y + 200, lngPack.i18n ("Text~Others~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	incButton (position.x + 279, position.y + 159, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL),
	decButton (position.x + 17, position.y + 159, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL),
	transferLabel (position.x + 157, position.y + 49, "", FONT_LATIN_BIG),
	arrowImage (position.x + 140, position.y + 77)
{
	menuItems.push_back (&arrowImage);

	getTransferType();

	doneButton.setReleasedFunction (&doneReleased);
	menuItems.push_back (&doneButton);

	cancelButton.setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (&cancelButton);

	incButton.setReleasedFunction (&incReleased);
	menuItems.push_back (&incButton);

	decButton.setReleasedFunction (&decReleased);
	menuItems.push_back (&decButton);

	resBar = new cMenuMaterialBar (position.x + 43, position.y + 159, 0, 0, 223, transferType, false, false),
	resBar->setClickedFunction (&barClicked);
	menuItems.push_back (resBar.get());

	unitNameLabels[0] = new cMenuLabel (position.x + 70, position.y + 105, "", FONT_LATIN_SMALL_WHITE);
	unitNameLabels[0]->setCentered (true);
	menuItems.push_back (unitNameLabels[0].get());

	unitNameLabels[1] = new cMenuLabel (position.x + 240, position.y + 105, "", FONT_LATIN_SMALL_WHITE);
	unitNameLabels[1]->setCentered (true);
	menuItems.push_back (unitNameLabels[1].get());

	unitCargoLabels[0] = new cMenuLabel (position.x + 30, position.y + 60, "", FONT_LATIN_SMALL_WHITE);
	unitCargoLabels[0]->setCentered (true);
	menuItems.push_back (unitCargoLabels[0].get());

	unitCargoLabels[1] = new cMenuLabel (position.x + 280, position.y + 60, "", FONT_LATIN_SMALL_WHITE);
	unitCargoLabels[1]->setCentered (true);
	menuItems.push_back (unitCargoLabels[1].get());

	transferLabel.setCentered (true);
	menuItems.push_back (&transferLabel);

	unitImages[0] = new cMenuImage (position.x + 39, position.y + 26);
	menuItems.push_back (unitImages[0].get());

	unitImages[1] = new cMenuImage (position.x + 208, position.y + 26);
	menuItems.push_back (unitImages[1].get());

	getNamesNCargoNImages();
	setCargos();
}

cDialogTransfer::~cDialogTransfer()
{
	gameGUI->mouseInputMode = normalInput;
}

/*static*/ sUnitData::eStorageResType cDialogTransfer::getCommonStorageType (const cUnit& unit1, const cUnit& unit2)
{
	const sUnitData::eStorageResType type1 = unit1.data.storeResType;
	const sUnitData::eStorageResType type2 = unit2.data.storeResType;
	if (type1 == type2) return type1;

	if (unit1.isAVehicle() && unit2.isABuilding()) return type1;
	if (unit1.isABuilding() && unit2.isAVehicle()) return type2;
	return sUnitData::STORE_RES_NONE;
}

void cDialogTransfer::getTransferType()
{
	sUnitData::eStorageResType commonTransferType = getCommonStorageType (*srcUnit, *destUnit);

	switch (commonTransferType)
	{
		case sUnitData::STORE_RES_METAL:
			transferType = cMenuMaterialBar::MAT_BAR_TYPE_METAL_HORI_SMALL;
			break;
		case sUnitData::STORE_RES_OIL:
			transferType = cMenuMaterialBar::MAT_BAR_TYPE_OIL_HORI_SMALL;
			break;
		case sUnitData::STORE_RES_GOLD:
			transferType = cMenuMaterialBar::MAT_BAR_TYPE_GOLD_HORI_SMALL;
			break;
		case sUnitData::STORE_RES_NONE:
			end = true;
			break;
	}
}

void cDialogTransfer::getNamesNCargoNImages()
{
	const int UNIT_IMAGE_SIZE = 64;

	SDL_Surface* unitImage1 = SDL_CreateRGBSurface (0, UNIT_IMAGE_SIZE, UNIT_IMAGE_SIZE, Video.getColDepth(), 0, 0, 0, 0);
	SDL_FillRect (unitImage1, NULL, 0xFF00FF);
	SDL_SetColorKey (unitImage1, SDL_TRUE, 0xFF00FF);

	SDL_Surface* unitImage2 = SDL_CreateRGBSurface (0, UNIT_IMAGE_SIZE, UNIT_IMAGE_SIZE, Video.getColDepth(), 0, 0, 0, 0);
	SDL_FillRect (unitImage2, NULL, 0xFF00FF);
	SDL_SetColorKey (unitImage2, SDL_TRUE, 0xFF00FF);

	SDL_Rect dest = {0, 0, 0, 0};

	if (srcUnit->isABuilding())
	{
		cBuilding* srcBuilding = static_cast<cBuilding*> (srcUnit);
		float zoomFactor = UNIT_IMAGE_SIZE / (srcBuilding->data.isBig ? 128.0f : 64.0f);
		srcBuilding->render (gameGUI, unitImage1, dest, zoomFactor, false, false);

		unitNameLabels[0]->setText (srcBuilding->data.name);
		if (destUnit->isAVehicle())
		{
			switch (destUnit->data.storeResType)
			{
				case sUnitData::STORE_RES_METAL:
					maxSrcCargo = srcBuilding->SubBase->MaxMetal;
					srcCargo = srcBuilding->SubBase->Metal;
					break;
				case sUnitData::STORE_RES_OIL:
					maxSrcCargo = srcBuilding->SubBase->MaxOil;
					srcCargo = srcBuilding->SubBase->Oil;
					break;
				case sUnitData::STORE_RES_GOLD:
					maxSrcCargo = srcBuilding->SubBase->MaxGold;
					srcCargo = srcBuilding->SubBase->Gold;
					break;
				case sUnitData::STORE_RES_NONE:
					break;
			}
		}
		else
		{
			maxSrcCargo = srcBuilding->data.storageResMax;
			srcCargo = srcBuilding->data.storageResCur;
		}
	}
	else
	{
		assert (srcUnit->isAVehicle());

		cVehicle* srcVehicle = static_cast<cVehicle*> (srcUnit);
		float zoomFactor = UNIT_IMAGE_SIZE / (srcVehicle->data.isBig ? 128.0f : 64.0f);
		srcVehicle->render (gameGUI->getClient(), unitImage1, dest, zoomFactor, false);
		srcVehicle->drawOverlayAnimation (gameGUI->getClient(), unitImage1, dest, zoomFactor);

		unitNameLabels[0]->setText (srcVehicle->data.name);
		maxSrcCargo = srcVehicle->data.storageResMax;
		srcCargo = srcVehicle->data.storageResCur;
	}

	if (destUnit->isABuilding())
	{
		cBuilding* destBuilding = static_cast<cBuilding*> (destUnit);
		float zoomFactor = UNIT_IMAGE_SIZE / (destBuilding->data.isBig ? 128.0f : 64.0f);
		destBuilding->render (gameGUI, unitImage2, dest, zoomFactor, false, false);

		unitNameLabels[1]->setText (destBuilding->data.name);
		if (srcUnit->isAVehicle())
		{
			switch (srcUnit->data.storeResType)
			{
				case sUnitData::STORE_RES_METAL:
					maxDestCargo = destBuilding->SubBase->MaxMetal;
					destCargo = destBuilding->SubBase->Metal;
					break;
				case sUnitData::STORE_RES_OIL:
					maxDestCargo = destBuilding->SubBase->MaxOil;
					destCargo = destBuilding->SubBase->Oil;
					break;
				case sUnitData::STORE_RES_GOLD:
					maxDestCargo = destBuilding->SubBase->MaxGold;
					destCargo = destBuilding->SubBase->Gold;
					break;
				case sUnitData::STORE_RES_NONE:
					break;
			}
		}
		else
		{
			maxDestCargo = destBuilding->data.storageResMax;
			destCargo = destBuilding->data.storageResCur;
		}
	}
	else
	{
		assert (destUnit->isAVehicle());

		cVehicle* destVehicle = static_cast<cVehicle*> (destUnit);
		float zoomFactor = UNIT_IMAGE_SIZE / (destVehicle->data.isBig ? 128.0f : 64.0f);
		destVehicle->render (gameGUI->getClient(), unitImage2, dest, zoomFactor, false);
		destVehicle->drawOverlayAnimation (gameGUI->getClient(), unitImage2, dest, zoomFactor);

		unitNameLabels[1]->setText (destVehicle->data.name);
		maxDestCargo = destVehicle->data.storageResMax;
		destCargo = destVehicle->data.storageResCur;
	}

	unitImages[0]->setImage (unitImage1);
	unitImages[1]->setImage (unitImage2);
	transferValue = maxDestCargo;
}

// TODO: move function into a better file ?
static void FlipSurfaceHorizontally (SDL_Surface* surface)
{
	assert (surface);
	if (SDL_MUSTLOCK (surface)) SDL_LockSurface (surface);

	// Assume surface format uses Uint32*
	// TODO: check surface format (or support more format).
	Uint32* p = static_cast<Uint32*> (surface->pixels);

	for (int h = 0; h != surface->h; ++h)
		for (int w = 0; w != surface->w / 2; ++w)
			std::swap (p[h * surface->w + w], p [ (h + 1) * surface->w - w - 1]);

	if (SDL_MUSTLOCK (surface)) SDL_UnlockSurface (surface);
}

void cDialogTransfer::setCargos()
{
	transferValue = std::min (transferValue, srcCargo);
	transferValue = std::max (transferValue, -destCargo);
	transferValue = std::min (transferValue, maxDestCargo - destCargo);
	transferValue = std::max (transferValue, srcCargo - maxSrcCargo);

	unitCargoLabels[0]->setText (iToStr (srcCargo - transferValue));
	unitCargoLabels[1]->setText (iToStr (destCargo + transferValue));

	// Change arrow direction.
	// TODO: find a better way to (un)hide image.
	if (transferValue >= 0) arrowImage.setImage (NULL);
	else
	{
		// Set right to left arrow image.
		// little hack: flip part of the image that represent the arrow
		const unsigned int w = 40;
		const unsigned int h = 20;
		SDL_Surface* arrowSurface = SDL_CreateRGBSurface (0, w, h, Video.getColDepth(), 0, 0, 0, 0);
		const Sint16 x = arrowImage.getPosition().x - position.x; // 140
		const Sint16 y = arrowImage.getPosition().y - position.y; //  77
		SDL_Rect src = {x, y, w, h};
		SDL_BlitSurface (background, &src, arrowSurface, NULL);
		FlipSurfaceHorizontally (arrowSurface);

		arrowImage.setImage (arrowSurface);
	}
	transferLabel.setText (iToStr (abs (transferValue)));

	resBar->setCurrentValue ((int) (223.f * (destCargo + transferValue) / maxDestCargo));
}

void cDialogTransfer::handleKeyInput (const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
		case SDLK_RETURN:
			if (key.state == SDL_PRESSED && !doneButton.getIsClicked()) doneButton.clicked (this);
			else if (key.state == SDL_RELEASED && doneButton.getIsClicked()) doneButton.released (this);
			break;
		case SDLK_ESCAPE:
			if (key.state == SDL_PRESSED && !cancelButton.getIsClicked()) cancelButton.clicked (this);
			else if (key.state == SDL_RELEASED && cancelButton.getIsClicked()) cancelButton.released (this);
			break;
		default:
			break;
	}
}

void cDialogTransfer::doneReleased (void* parent)
{
	cDialogTransfer* menu = reinterpret_cast<cDialogTransfer*> (parent);

	if (menu->transferValue != 0)
	{
		cClient* client = menu->gameGUI->getClient();
		const cUnit* srcUnit = menu->srcUnit;
		const cUnit* destUnit = menu->destUnit;
		const sUnitData::eStorageResType transfertType = getCommonStorageType (*srcUnit, *destUnit);
		sendWantTransfer (*client, srcUnit->isAVehicle(), srcUnit->iID, destUnit->isAVehicle(), destUnit->iID, menu->transferValue, transfertType);
	}

	menu->end = true;
}

void cDialogTransfer::incReleased (void* parent)
{
	cDialogTransfer* menu = reinterpret_cast<cDialogTransfer*> (parent);
	menu->transferValue++;
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::decReleased (void* parent)
{
	cDialogTransfer* menu = reinterpret_cast<cDialogTransfer*> (parent);
	menu->transferValue--;
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::barClicked (void* parent)
{
	cDialogTransfer* menu = reinterpret_cast<cDialogTransfer*> (parent);
	menu->transferValue = Round ((cMouse::getInstance().getPosition().x() - menu->resBar->getPosition().x) * (menu->maxDestCargo / 223.0f) - menu->destCargo);
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (&destroyedUnit == srcUnit || &destroyedUnit == destUnit) terminate = true;
}

void drawContextItem (const string& sText, bool bPressed, int x, int y, SDL_Surface* surface)
{
	SDL_Rect dest = {Sint16 (x), Sint16 (y), 42, 21};
	SDL_Rect src = {0, 0, 42, 21}; //default button deselected
	if (bPressed) src.y += 21;

	SDL_BlitSurface (GraphicsData.gfx_context_menu, &src, surface, &dest);
	font->showTextCentered (dest.x + dest.w / 2, dest.y + (dest.h / 2 - font->getFontHeight (FONT_LATIN_SMALL_WHITE) / 2) + 1, sText, FONT_LATIN_SMALL_WHITE);
}

cDialogResearch::cDialogResearch (cClient& client_) :
	cMenu (LoadPCX (GFXOD_DIALOG_RESEARCH), MNU_BG_ALPHA),
	client (&client_), owner (&client_.getActivePlayer()),
	titleLabel (position.x + position.w / 2, position.y + 19, lngPack.i18n ("Text~Title~Labs")),
	centersLabel (position.x + 58, position.y + 52, lngPack.i18n ("Text~Comp~Labs")),
	themeLabel (position.x + 200, position.y + 52, lngPack.i18n ("Text~Comp~Themes")),
	turnsLabel (position.x + 313, position.y + 52, lngPack.i18n ("Text~Comp~Turns")),
	doneButton (position.x + 193, position.y + 294, lngPack.i18n ("Text~Others~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	cancelButton (position.x + 91, position.y + 294, lngPack.i18n ("Text~Others~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	owner->researchFinished = false;

	titleLabel.setCentered (true);
	menuItems.push_back (&titleLabel);

	centersLabel.setCentered (true);
	menuItems.push_back (&centersLabel);

	themeLabel.setCentered (true);
	menuItems.push_back (&themeLabel);

	turnsLabel.setCentered (true);
	menuItems.push_back (&turnsLabel);

	doneButton.setReleasedFunction (&doneReleased);
	menuItems.push_back (&doneButton);

	cancelButton.setReleasedFunction (&cMenu::cancelReleased);
	menuItems.push_back (&cancelButton);

	const string themeNames[8] =
	{
		lngPack.i18n ("Text~Others~Attack"),
		lngPack.i18n ("Text~Others~Shots_7"),
		lngPack.i18n ("Text~Others~Range"),
		lngPack.i18n ("Text~Others~Armor_7"),
		lngPack.i18n ("Text~Others~Hitpoints_7"),
		lngPack.i18n ("Text~Others~Speed"),
		lngPack.i18n ("Text~Others~Scan"),
		lngPack.i18n ("Text~Others~Costs")
	};

	SDL_Rect attackSymbol = { 27, 109, 10, 14 };
	SDL_Rect shotsSymbol = { 37, 109, 15, 7 };
	SDL_Rect rangeSymbol = { 52, 109, 13, 13 };
	SDL_Rect armorSymbol = { 65, 109, 11, 14 };
	SDL_Rect hitpointsSymbol = { 11, 109, 7, 11 };
	SDL_Rect speedSymbol = { 0, 109, 11, 12 };
	SDL_Rect scanSymbol = { 76, 109, 13, 13 };
	SDL_Rect costsSymbol = { 112, 109, 13, 10 };
	SDL_Rect dest;

	dest.x = 167;
	dest.y = 70;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &attackSymbol, background, &dest);
	dest.x = 165;
	dest.y = 102;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &shotsSymbol, background, &dest);
	dest.x = 166;
	dest.y = 127;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &rangeSymbol, background, &dest);
	dest.x = 167;
	dest.y = 154;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &armorSymbol, background, &dest);
	dest.x = 169;
	dest.y = 184;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &hitpointsSymbol, background, &dest);
	dest.x = 167;
	dest.y = 212;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &speedSymbol, background, &dest);
	dest.x = 166;
	dest.y = 239;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &scanSymbol, background, &dest);
	dest.x = 166;
	dest.y = 268;
	SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &costsSymbol, background, &dest);

	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		centerCountLabels[i] = new cMenuLabel (position.x + 43, position.y + 71 + 28 * i, "0");
		centerCountLabels[i]->setCentered (true);
		menuItems.push_back (centerCountLabels[i].get());

		themeNameLabels[i] = new cMenuLabel (position.x + 183, position.y + 71 + 28 * i, themeNames[i]);
		menuItems.push_back (themeNameLabels[i].get());

		percentageLabels[i] = new cMenuLabel (position.x + 258, position.y + 71 + 28 * i, "+" + iToStr (owner->researchLevel.getCurResearchLevel (i)) + "%");
		percentageLabels[i]->setCentered (true);
		menuItems.push_back (percentageLabels[i].get());

		turnsLabels[i] = new cMenuLabel (position.x + 313, position.y + 71 + 28 * i, "");
		turnsLabels[i]->setCentered (true);
		menuItems.push_back (turnsLabels[i].get());

		incButtons[i] = new cMenuButton (position.x + 143, position.y + 70 + 28 * i, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL);
		incButtons[i]->setReleasedFunction (&incReleased);
		menuItems.push_back (incButtons[i].get());

		decButtons[i] = new cMenuButton (position.x + 71, position.y + 70 + 28 * i, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL);
		decButtons[i]->setReleasedFunction (&decReleased);
		menuItems.push_back (decButtons[i].get());

		scroller[i] = new cMenuScrollerHandler (position.x + 90, position.y + 70 + 28 * i, 51, owner->workingResearchCenterCount);
		scroller[i]->setClickedFunction (&sliderClicked);
		menuItems.push_back (scroller[i].get());
	}

	unusedResearch = owner->workingResearchCenterCount;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		newResearchSettings[i] = owner->researchCentersWorkingOnArea[i];
		unusedResearch -= newResearchSettings[i];
	}

	setData();
}

void cDialogResearch::setData()
{
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		centerCountLabels[i]->setText (iToStr (newResearchSettings[i]));
		scroller[i]->setValue (newResearchSettings[i]);

		turnsLabels[i]->setText (iToStr (owner->researchLevel.getRemainingTurns (i, newResearchSettings[i])));

		incButtons[i]->setLocked (unusedResearch <= 0);
		decButtons[i]->setLocked (newResearchSettings[i] <= 0);
	}
}

void cDialogResearch::handleKeyInput (const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
		case SDLK_RETURN:
			if (key.state == SDL_PRESSED && !doneButton.getIsClicked()) doneButton.clicked (this);
			else if (key.state == SDL_RELEASED && doneButton.getIsClicked()) doneButton.released (this);
			break;
		case SDLK_ESCAPE:
			if (key.state == SDL_PRESSED && !cancelButton.getIsClicked()) cancelButton.clicked (this);
			else if (key.state == SDL_RELEASED && cancelButton.getIsClicked()) cancelButton.released (this);
			break;
		default:
			break;
	}
}

void cDialogResearch::doneReleased (void* parent)
{
	cDialogResearch* menu = reinterpret_cast<cDialogResearch*> (parent);
	sendWantResearchChange (*menu->client, menu->newResearchSettings, menu->owner->getNr());
	menu->end = true;
}

void cDialogResearch::incReleased (void* parent)
{
	cDialogResearch* menu = reinterpret_cast<cDialogResearch*> (parent);
	if (menu->unusedResearch > 0)
	{
		menu->unusedResearch--;
		for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		{
			const auto& mousePosition = cMouse::getInstance().getPosition();
			if(menu->incButtons[i]->overItem(mousePosition.x(), mousePosition.y()))
			{
				menu->newResearchSettings[i]++;
				break;
			}
		}
		menu->setData();
		menu->draw();
	}
}

void cDialogResearch::decReleased (void* parent)
{
	cDialogResearch* menu = reinterpret_cast<cDialogResearch*> (parent);
	if (menu->unusedResearch < menu->owner->workingResearchCenterCount)
	{
		menu->unusedResearch++;
		for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		{
			const auto& mousePosition = cMouse::getInstance().getPosition();
			if(menu->decButtons[i]->overItem(mousePosition.x(), mousePosition.y()))
			{
				menu->newResearchSettings[i]--;
				break;
			}
		}
		menu->setData();
		menu->draw();
	}
}

void cDialogResearch::sliderClicked (void* parent)
{
	cDialogResearch* menu = reinterpret_cast<cDialogResearch*> (parent);
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		const auto& mousePosition = cMouse::getInstance().getPosition();
		if(menu->scroller[i]->overItem(mousePosition.x(), mousePosition.y()))
		{
			int posX = mousePosition.x() - menu->scroller[i]->getPosition().x;
			int wantResearch = Round ((float) menu->owner->workingResearchCenterCount / menu->scroller[i]->getPosition().w * posX);
			if (wantResearch <= menu->newResearchSettings[i])
			{
				menu->unusedResearch += menu->newResearchSettings[i] - wantResearch;
				menu->newResearchSettings[i] = wantResearch;
			}
			else
			{
				int wantIncrement = wantResearch - menu->newResearchSettings[i];
				int possibleIncrement = (wantIncrement >= menu->unusedResearch) ? menu->unusedResearch : wantIncrement;
				menu->newResearchSettings[i] += possibleIncrement;
				menu->unusedResearch -= possibleIncrement;
			}
			break;
		}
	}
	menu->setData();
	menu->draw();
}

void cDialogResearch::handleDestroyUnit (cUnit& destroyedUnit)
{
	if (destroyedUnit.isAVehicle()) return;
	if (destroyedUnit.owner != owner) return;
	if (!destroyedUnit.data.canResearch) return;
	if (static_cast<cBuilding&> (destroyedUnit).IsWorking) terminate = true;
}
