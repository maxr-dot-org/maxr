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
#include "dialog.h"
#include "mouse.h"
#include "unifonts.h"
#include "sound.h"
#include "pcx.h"
#include "files.h"
#include "log.h"
#include "loaddata.h"
#include "events.h"
#include "client.h"
#include "input.h"
#include "clientevents.h"
#include "sound.h"
#include "settings.h"
#include "video.h"

cDialogYesNo::cDialogYesNo(string text) :
	cMenu(LoadPCX(GFXOD_DIALOG2), MNU_BG_ALPHA),
	textLabel(position.x +  40, position.y +  40, text),
	yesButton(position.x + 155, position.y + 185, lngPack.i18n("Text~Button~Yes"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	noButton( position.x +  67, position.y + 185, lngPack.i18n("Text~Button~No"),  cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	textLabel.setBox(232, 142);
	//textLabel.setBox(position.w - 40, position.h - 150);
	menuItems.Add(&textLabel);

	yesButton.setReleasedFunction(&yesReleased);
	menuItems.Add(&yesButton);

	noButton.setReleasedFunction(&noReleased);
	menuItems.Add(&noButton);
}

void cDialogYesNo::handleKeyInput(SDL_KeyboardEvent& key, string ch)
{
	switch ( key.keysym.sym )
	{
	case SDLK_RETURN:
		switch (key.state)
		{
			case SDL_PRESSED:  if (!yesButton.getIsClicked()) yesButton.clicked(this);  break;
			case SDL_RELEASED: if (yesButton.getIsClicked())  yesButton.released(this); break;
		}
		break;
	case SDLK_ESCAPE:
		switch (key.state)
		{
			case SDL_PRESSED:  if (!noButton.getIsClicked()) noButton.clicked(this);  break;
			case SDL_RELEASED: if (noButton.getIsClicked())  noButton.released(this); break;
		}
		break;
	}
}


void cDialogYesNo::yesReleased(void* parent)
{
	cDialogYesNo* menu = static_cast<cDialogYesNo*>((cMenu*)parent);
	menu->end = true;
}

void cDialogYesNo::noReleased(void* parent)
{
	cDialogYesNo* menu = static_cast<cDialogYesNo*>((cMenu*)parent);
	menu->terminate = true;
}

cDialogOK::cDialogOK(string text) :
	cMenu(LoadPCX(GFXOD_DIALOG2), MNU_BG_ALPHA),
	textLabel(position.x + 40, position.y + 40, text),
	okButton(position.x + 111, position.y + 185, lngPack.i18n("Text~Button~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL)
{
	textLabel.setBox(232, 142);
	menuItems.Add(&textLabel);

	okButton.setReleasedFunction(&okReleased);
	menuItems.Add(&okButton);
}

void cDialogOK::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	if ( key.keysym.sym == SDLK_RETURN )
	{
		switch (key.state)
		{
			case SDL_PRESSED:  if (!okButton.getIsClicked()) okButton.clicked(this);  break;
			case SDL_RELEASED: if (okButton.getIsClicked())  okButton.released(this); break;
		}
	}
}

void cDialogOK::okReleased( void *parent )
{
	cDialogOK* menu = static_cast<cDialogOK*>((cMenu*)parent);
	menu->end = true;
}

cDestructMenu::cDestructMenu() :
	cMenu(LoadPCX(GFXOD_DESTRUCTION), MNU_BG_ALPHA),
	armButton(position.x + 88, position.y + 14, lngPack.i18n("Text~Button~Hot"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	cancelButton(position.x + 88, position.y + 46, lngPack.i18n("Text~Button~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	destroyButton( position.x + 15, position.y + 13, this )
{
	cancelButton.setReleasedFunction( &cancelReleased );
	menuItems.Add(&cancelButton);

	armButton.setReleasedFunction( &armReleased );
	menuItems.Add(&armButton);

	destroyButton.setReleasedFunction( &destroyReleased );
	destroyButton.setReleaseSound( SoundData.SNDMenuButton );
	menuItems.Add(&destroyButton);
	
}

void cDestructMenu::cancelReleased( void *parent )
{
	cDestructMenu* menu = static_cast<cDestructMenu*>((cMenu*)parent);
	menu->terminate = true;
}

void cDestructMenu::armReleased( void *parent )
{
	cDestructMenu* menu = static_cast<cDestructMenu*>((cMenu*)parent);
	menu->armButton.setLocked(true);
	menu->destroyButton.setLocked(false);
}

void cDestructMenu::destroyReleased( void *parent )
{
	cDestructMenu* menu = static_cast<cDestructMenu*>((cMenu*)parent);
	menu->end = true;
}

cDialogLicence::cDialogLicence() :
	cMenu(LoadPCX(GFXOD_DIALOG4), MNU_BG_ALPHA),
	maxrLabel(  position.x + position.w / 2, position.y +  30, "\"M.A.X.R.\"" ),
	headerLabel(position.x + position.w / 2, position.y +  30 + font->getFontHeight(), "(C) 2007 by its authors"),
	textLabel(  position.x +  35,            position.y +  30 + 3 * font->getFontHeight()),
	okButton(   position.x + 111,            position.y + 185, lngPack.i18n ("Text~Button~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL),
	upButton(   position.x + 241,            position.y + 187, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL),
	downButton( position.x + 261,            position.y + 187, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL)
{
	generateLicenceTexts();

	maxrLabel.setCentered(true);
	menuItems.Add(&maxrLabel);

	headerLabel.setCentered(true);
	menuItems.Add(&headerLabel);

	textLabel.setBox(232, 142);
	menuItems.Add(&textLabel);

	okButton.setReleasedFunction(&okReleased);
	menuItems.Add(&okButton);

	upButton.setReleasedFunction(&upReleased);
	menuItems.Add(&upButton);

	downButton.setReleasedFunction(&downReleased);
	menuItems.Add(&downButton);

	offset = 0;
	resetText();
}

void cDialogLicence::generateLicenceTexts()
{
	sLicence1 = " \
	This program is free software; you can redistribute it and/or modify \
	it under the terms of the GNU General Public License as published by \
	the Free Software Foundation; either version 2 of the License, or \
	(at your option) any later version.";
	sLicence2 = "\
	This program is distributed in the hope that it will be useful, \
	but WITHOUT ANY WARRANTY; without even the implied warranty of \
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the \
	GNU General Public License for more details.";
	sLicence3="\
	You should have received a copy of the GNU General Public License \
	along with this program; if not, write to the Free Software \
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA";

	//open AUTHOR
	string sAuthors;
#ifdef WIN32
		sAuthors = "AUTHORS.txt";
#elif __amigaos4
		sAuthors = SettingsData.sDataDir + PATH_DELIMITER + "AUTHORS.txt";
#elif MAC
		sAuthors = "AUTHORS";
#else
		sAuthors = SettingsData.sDataDir + PATH_DELIMITER + "AUTHORS";
#endif

	sLicence4 = "";
	char line[72];

	FILE* const fp = fopen( sAuthors.c_str(), "r" );
	if ( fp != NULL  )
	{	//read authors from file
		while( fgets( line, 72, fp ) ) //snip entrys longer 72
		{
			sLicence4 += line;
		}
		fclose( fp );
	}
	else sLicence4 = "Couldn't read AUTHORS"; //missing file - naughty
}

void cDialogLicence::resetText()
{
	switch ( offset )
	{
	case 0:
		textLabel.setText(sLicence1);
		upButton.setLocked(true);
		break;
	case 1:
		textLabel.setText(sLicence2);
		upButton.setLocked(false);
		break;
	case 2:
		textLabel.setText(sLicence3);
		textLabel.setFontType(FONT_LATIN_NORMAL);
		downButton.setLocked(false);
		headerLabel.setText("(C) 2007 by its authors");
		break;
	case 3:
		textLabel.setText(sLicence4);
		textLabel.setFontType(FONT_LATIN_SMALL_WHITE);
		downButton.setLocked(true);
		headerLabel.setText("AUTHORS:");
		break;
	}
	draw();
}

void cDialogLicence::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	if ( key.keysym.sym == SDLK_RETURN )
	{
		switch (key.state)
		{
			case SDL_PRESSED:  if (!okButton.getIsClicked()) okButton.clicked(this);  break;
			case SDL_RELEASED: if (okButton.getIsClicked())  okButton.released(this); break;
		}
	}
}

void cDialogLicence::okReleased( void *parent )
{
	cDialogLicence* menu = static_cast<cDialogLicence*>((cMenu*)parent);
	menu->end = true;
}

void cDialogLicence::upReleased( void *parent )
{
	cDialogLicence* menu = static_cast<cDialogLicence*>((cMenu*)parent);
	if ( menu->offset > 0 ) menu->offset--;
	menu->resetText();
}

void cDialogLicence::downReleased( void *parent )
{
	cDialogLicence* menu = static_cast<cDialogLicence*>((cMenu*)parent);
	if ( menu->offset < 3 ) menu->offset++;
	menu->resetText();
}

cDialogPreferences::cDialogPreferences() : cMenu ( LoadPCX ( GFXOD_DIALOG5 ), MNU_BG_ALPHA )
{
	// blit black titlebar behind textfield for playername
	SDL_Rect src = { 108, 12, 186, 18 };
	SDL_Rect dest = { 108, 154, 0, 0 };
	SDL_BlitSurface ( background, &src, background, &dest );

	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+15, lngPack.i18n( "Text~Settings~Preferences" ) );
	titleLabel->setCentered ( true );
	menuItems.Add ( titleLabel );

	volumeLabel = new cMenuLabel ( position.x+25, position.y+56, lngPack.i18n( "Text~Settings~Volume" ) + ":" );
	menuItems.Add ( volumeLabel );

	musicLabel = new cMenuLabel ( position.x+25, position.y+56+20, lngPack.i18n( "Text~Settings~Music" ) );
	menuItems.Add ( musicLabel );
	disableMusicChBox = new cMenuCheckButton ( position.x+210, position.y+73, lngPack.i18n( "Text~Settings~Disable" ), SettingsData.MusicMute, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	disableMusicChBox->setClickedFunction ( &musicMuteChanged );
	menuItems.Add ( disableMusicChBox );
	musicSlider = new cMenuSlider ( position.x+140, position.y+81, 0, 128, this );
	musicSlider->setValue ( (float)SettingsData.MusicVol );
	musicSlider->setMoveCallback ( &musicVolumeChanged );
	menuItems.Add ( musicSlider );
	menuItems.Add ( musicSlider->scroller );

	effectsLabel = new cMenuLabel ( position.x+25, position.y+56+20*2, lngPack.i18n( "Text~Settings~Effects" ) );
	menuItems.Add ( effectsLabel );
	disableEffectsChBox = new cMenuCheckButton ( position.x+210, position.y+73+20, lngPack.i18n( "Text~Settings~Disable" ), SettingsData.SoundMute, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	disableEffectsChBox->setClickedFunction ( &effectsMuteChanged );
	menuItems.Add ( disableEffectsChBox );
	effectsSlider = new cMenuSlider ( position.x+140, position.y+81+20, 0, 128, this );
	effectsSlider->setValue ( (float)SettingsData.SoundVol );
	effectsSlider->setMoveCallback ( &effectsVolumeChanged );
	menuItems.Add ( effectsSlider );
	menuItems.Add ( effectsSlider->scroller );

	voicesLabel = new cMenuLabel ( position.x+25, position.y+56+20*3, lngPack.i18n( "Text~Settings~Voices" ) );
	menuItems.Add ( voicesLabel );
	disableVoicesChBox = new cMenuCheckButton ( position.x+210, position.y+73+20*2, lngPack.i18n( "Text~Settings~Disable" ), SettingsData.VoiceMute, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	disableVoicesChBox->setClickedFunction ( &voicesMuteChanged );
	menuItems.Add ( disableVoicesChBox );
	voicesSlider = new cMenuSlider ( position.x+140, position.y+81+20*2, 0, 128, this );
	voicesSlider->setValue ( (float)SettingsData.VoiceVol );
	voicesSlider->setMoveCallback ( &voicesVolumeChanged );
	menuItems.Add ( voicesSlider );
	menuItems.Add ( voicesSlider->scroller );

	nameLabel = new cMenuLabel ( position.x+25, position.y+158, lngPack.i18n( "Text~Title~Player_Name" ) );
	menuItems.Add ( nameLabel );
	nameEdit = new cMenuLineEdit ( position.x+112, position.y+154, 185, 18, this );
	nameEdit->setText ( SettingsData.sPlayerName );
	menuItems.Add ( nameEdit );

	animationChBox = new cMenuCheckButton ( position.x+25, position.y+193, lngPack.i18n( "Text~Settings~Animation" ), SettingsData.bAnimations, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( animationChBox );

	shadowsChBox = new cMenuCheckButton ( position.x+25, position.y+193+20, lngPack.i18n( "Text~Settings~Shadows" ), SettingsData.bShadows, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( shadowsChBox );

	alphaChBox = new cMenuCheckButton ( position.x+25, position.y+193+20*2, lngPack.i18n( "Text~Settings~Alphaeffects" ), SettingsData.bAlphaEffects, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( alphaChBox );

	demageBuilChBox = new cMenuCheckButton ( position.x+210, position.y+193, lngPack.i18n( "Text~Settings~ShowDamage" ), SettingsData.bDamageEffects, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( demageBuilChBox );

	demageVehChBox = new cMenuCheckButton ( position.x+210, position.y+193+20, lngPack.i18n( "Text~Settings~ShowDamageVehicle" ), SettingsData.bDamageEffectsVehicles, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( demageVehChBox );

	tracksChBox = new cMenuCheckButton ( position.x+210, position.y+193+20*2, lngPack.i18n( "Text~Settings~Tracks" ), SettingsData.bMakeTracks, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( tracksChBox );

	scrollSpeedLabel = new cMenuLabel ( position.x+25, position.y+232+25, lngPack.i18n( "Text~Settings~Scrollspeed" ) );
	menuItems.Add ( scrollSpeedLabel );
	scrollSpeedSlider = new cMenuSlider ( position.x+140, position.y+261, 0, 250, this );
	scrollSpeedSlider->setValue ( (float)SettingsData.iScrollSpeed );
	menuItems.Add ( scrollSpeedSlider );
	menuItems.Add ( scrollSpeedSlider->scroller );

	autosaveChBox = new cMenuCheckButton ( position.x+25, position.y+294, lngPack.i18n( "Text~Settings~Autosave" ), SettingsData.bAutoSave, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( autosaveChBox );

	introChBox = new cMenuCheckButton ( position.x+25, position.y+294+20, lngPack.i18n( "Text~Settings~Intro" ), SettingsData.bIntro, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( introChBox );

	windowChBox = new cMenuCheckButton ( position.x+25, position.y+294+20*2, lngPack.i18n( "Text~Settings~Window" ), SettingsData.bWindowMode, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( windowChBox );

	//BEGIN SCREEN RESOLUTION CHECKBOXES
	//FIXME: need dropdown box item for this. This is very dirty code fixed to 9 possible resolution values. Odd things might occur if less than 9 useable screen resolutions are found here. 
	//HINT: This works only as long as avail video modes have a neat follow up list starting with 0 so make sure that the modes vector doesn't get confused
	
	int resolutionMode = Video.validateMode(SettingsData.iScreenW, SettingsData.iScreenH); //set flagged box to current resolution if found
	resoulutionGroup = new cMenuRadioGroup;
	
	if (Video.getVideoSize() <= 0)
	{
	    Log.write("No resolutions could be detected. Can't display any options here.",  cLog::eLOG_TYPE_ERROR);
	}
	else //we got resolutions from SDL so we use the detected ones here and overwrite the default ones from above (but not our default minimal resolution)
	{
	  for ( int i = 0, x = 0; x < 3; x++ )
	  {
	    for ( int y = 0; y < 3; y++, i++ )
	    {
	      if(i >= Video.getVideoSize())
	      {
		Log.write("Oops, looks like we read less resolutions than I should offer. This might result in some glitches in my dialog. I want a drop down box here!",  cLog::eLOG_TYPE_WARNING);
		cMenuCheckButton *button = new cMenuCheckButton ( position.x+150+80*x, position.y+290+20*y, "<empty>", resolutionMode == i, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
		resoulutionGroup->addButton ( button );

	      }
	      else
	      {
		Log.write("Offering display resolution "+Video.getVideoMode(i)+" to user", cLog::eLOG_TYPE_DEBUG);
		cMenuCheckButton *button = new cMenuCheckButton ( position.x+150+80*x, position.y+290+20*y, Video.getVideoMode(i), resolutionMode == i, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
		resoulutionGroup->addButton ( button );
		
	      }
	    }
	  }
	  
	  if(Video.getVideoSize() > 9) //notice: we skip mode 10. to much on screen. bad luck until we get a drop down box or similar
	  {
	    Log.write("Read more possible resolutions than I can display on dialog. Stopped.",  cLog::eLOG_TYPE_WARNING);
	  } 
	}
	
	menuItems.Add ( resoulutionGroup );
	
	//END SCREEN RESOLUTION CHECKBOXES

	okButton = new cMenuButton ( position.x+208, position.y+383, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	cancelButton = new cMenuButton ( position.x+118, position.y+383, lngPack.i18n ("Text~Button~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	cancelButton->setReleasedFunction ( &cancelReleased );
	menuItems.Add ( cancelButton );

	// save old volumes
	oldMusicVolume = SettingsData.MusicVol;
	oldEffectsVolume = SettingsData.SoundVol;
	oldVoicesVolume = SettingsData.VoiceVol;

	oldMusicMute = SettingsData.MusicMute;
	oldEffectsMute = SettingsData.SoundMute;
	oldVoicesMute = SettingsData.VoiceMute;
}

cDialogPreferences::~cDialogPreferences()
{
	delete titleLabel;

	delete volumeLabel;
	delete musicLabel;
	delete effectsLabel;
	delete voicesLabel;
	delete disableMusicChBox;
	delete disableEffectsChBox;
	delete disableVoicesChBox;
	delete musicSlider;
	delete effectsSlider;
	delete voicesSlider;

	delete nameLabel;
	delete nameEdit;

	delete animationChBox;
	delete shadowsChBox;
	delete alphaChBox;
	delete demageBuilChBox;
	delete demageVehChBox;
	delete tracksChBox;

	delete scrollSpeedLabel;
	delete scrollSpeedSlider;

	delete autosaveChBox;
	delete introChBox;
	delete windowChBox;

	delete resoulutionGroup;

	delete okButton;
	delete cancelButton;
}

void cDialogPreferences::saveValues()
{

	SettingsData.sPlayerName = nameEdit->getText();
	if ( Client) Client->ActivePlayer->name = SettingsData.sPlayerName;

	SettingsData.bAutoSave = autosaveChBox->isChecked();
	SettingsData.bAnimations = animationChBox->isChecked();
	SettingsData.bAlphaEffects = alphaChBox->isChecked();
	SettingsData.bDamageEffects = demageBuilChBox->isChecked();
	SettingsData.bDamageEffectsVehicles = demageVehChBox->isChecked();
	SettingsData.bIntro = introChBox->isChecked();
	SettingsData.bMakeTracks = tracksChBox->isChecked();
	SettingsData.bWindowMode = windowChBox->isChecked();
	SettingsData.bShadows = shadowsChBox->isChecked();

	SettingsData.iScrollSpeed = (int)scrollSpeedSlider->getValue();

	// Save new settings to max.xml
	SaveOption ( SAVETYPE_MUSICMUTE );
	SaveOption ( SAVETYPE_SOUNDMUTE );
	SaveOption ( SAVETYPE_VOICEMUTE );
	SaveOption ( SAVETYPE_AUTOSAVE );
	SaveOption ( SAVETYPE_ANIMATIONS );
	SaveOption ( SAVETYPE_SHADOWS );
	SaveOption ( SAVETYPE_ALPHA );
	SaveOption ( SAVETYPE_SCROLLSPEED );
	SaveOption ( SAVETYPE_MUSICVOL );
	SaveOption ( SAVETYPE_SOUNDVOL );
	SaveOption ( SAVETYPE_VOICEVOL );
	SaveOption ( SAVETYPE_DAMAGEEFFECTS_BUILDINGS );
	SaveOption ( SAVETYPE_DAMAGEEFFECTS_VEHICLES );
	SaveOption ( SAVETYPE_TRACKS );
	// TODO: remove game
	SaveOption ( SAVETYPE_NAME );
	SaveOption ( SAVETYPE_INTRO );
	SaveOption ( SAVETYPE_WINDOW );

	// save resolution
	int oldScreenW = SettingsData.iScreenW;
	int oldScreenH = SettingsData.iScreenH;

	for ( int i = 0; i < 9; i++ )
	{
		if ( resoulutionGroup->buttonIsChecked ( i ) )
		{
			string sTmp = Video.getVideoMode(i);
			SettingsData.iScreenW = atoi ( sTmp.substr(0, sTmp.find_first_of('x')).c_str());
			SettingsData.iScreenH = atoi ( sTmp.substr(sTmp.find_first_of('x')+1, sTmp.size()).c_str());
			
			SaveOption ( SAVETYPE_RESOLUTION );
			if ( SettingsData.iScreenW != oldScreenW || SettingsData.iScreenH != oldScreenH )
			{
				SettingsData.iScreenW = oldScreenW;
				SettingsData.iScreenH = oldScreenH;
				cDialogOK okDialog( lngPack.i18n( "Text~Comp~ResolutionChange" ) );
				okDialog.show();
				break;
			}
		}
	}
}

void cDialogPreferences::okReleased( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	menu->saveValues();
	menu->end = true;
}

void cDialogPreferences::cancelReleased( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);

	// restore old volumes
	SettingsData.MusicVol = menu->oldMusicVolume;
	SettingsData.SoundVol = menu->oldEffectsVolume;
	SettingsData.VoiceVol = menu->oldVoicesVolume;
	if(SettingsData.bSoundEnabled)Mix_VolumeMusic ( SettingsData.MusicVol );
	if(SettingsData.bSoundEnabled)Mix_Volume ( SoundLoopChannel, SettingsData.SoundVol );

	bool wasMusicMute = SettingsData.MusicMute;
	SettingsData.MusicMute = menu->oldMusicMute;
	if ( wasMusicMute && !menu->oldMusicMute ) StartMusic();
	SettingsData.SoundMute = menu->oldEffectsMute;
	SettingsData.VoiceMute = menu->oldVoicesMute;

	menu->terminate = true;
}

void cDialogPreferences::musicVolumeChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	SettingsData.MusicVol = (int)menu->musicSlider->getValue();
	if(SettingsData.bSoundEnabled)Mix_VolumeMusic ( SettingsData.MusicVol );
}

void cDialogPreferences::effectsVolumeChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	SettingsData.SoundVol = (int)menu->effectsSlider->getValue();
	if(SettingsData.bSoundEnabled)Mix_Volume ( SoundLoopChannel, SettingsData.SoundVol );
}

void cDialogPreferences::voicesVolumeChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	SettingsData.VoiceVol = (int)menu->voicesSlider->getValue();
}

void cDialogPreferences::musicMuteChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	bool wasMute = SettingsData.MusicMute;
	SettingsData.MusicMute = menu->disableMusicChBox->isChecked();
	if ( SettingsData.MusicMute ) StopMusic ();
	if ( !SettingsData.MusicMute && wasMute ) StartMusic();
}

void cDialogPreferences::effectsMuteChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	SettingsData.SoundMute = menu->disableEffectsChBox->isChecked();
}

void cDialogPreferences::voicesMuteChanged( void *parent )
{
	cDialogPreferences* menu = static_cast<cDialogPreferences*>((cMenu*)parent);
	SettingsData.VoiceMute = menu->disableVoicesChBox->isChecked();
}

cDialogTransfer::cDialogTransfer( cBuilding *srcBuilding_, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_  ) :
	cMenu(LoadPCX(GFXOD_DIALOG_TRANSFER), MNU_BG_ALPHA),
	srcBuilding(srcBuilding_),
	destBuilding(destBuilding_),
	srcVehicle(srcVehicle_),
	destVehicle(destVehicle_)
{
	// TODO: add changing arrow direction!

	getTransferType();

	incButton = new cMenuButton ( position.x+279, position.y+159, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL );
	incButton->setReleasedFunction ( &incReleased );
	menuItems.Add ( incButton );

	decButton = new cMenuButton ( position.x+17, position.y+159, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL );
	decButton->setReleasedFunction ( &decReleased );
	menuItems.Add ( decButton );

	resBar = new cMenuMaterialBar ( position.x+43, position.y+159, 0, 0, 223, transferType, false, false );
	resBar->setClickedFunction ( &barClicked );
	menuItems.Add ( resBar );

	doneButton = new cMenuButton ( position.x+159, position.y+200, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	doneButton->setReleasedFunction ( &doneReleased );
	menuItems.Add ( doneButton );

	cancelButton = new cMenuButton ( position.x+71, position.y+200, lngPack.i18n ("Text~Button~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	cancelButton->setReleasedFunction ( &cancelReleased );
	menuItems.Add ( cancelButton );

	unitNameLabels[0] = new cMenuLabel ( position.x+70, position.y+105, "", FONT_LATIN_SMALL_WHITE );
	unitNameLabels[0]->setCentered ( true );
	menuItems.Add ( unitNameLabels[0] );

	unitNameLabels[1] = new cMenuLabel ( position.x+240, position.y+105, "", FONT_LATIN_SMALL_WHITE );
	unitNameLabels[1]->setCentered ( true );
	menuItems.Add ( unitNameLabels[1] );

	unitCargoLabels[0] = new cMenuLabel ( position.x+30, position.y+60, "", FONT_LATIN_SMALL_WHITE );
	unitCargoLabels[0]->setCentered ( true );
	menuItems.Add ( unitCargoLabels[0] );

	unitCargoLabels[1] = new cMenuLabel ( position.x+280, position.y+60, "", FONT_LATIN_SMALL_WHITE );
	unitCargoLabels[1]->setCentered ( true );
	menuItems.Add ( unitCargoLabels[1] );

	transferLabel = new cMenuLabel ( position.x+157, position.y+49, "", FONT_LATIN_BIG );
	transferLabel->setCentered ( true );
	menuItems.Add ( transferLabel );

	unitImages[0] = new cMenuImage ( position.x+39, position.y+26 );
	menuItems.Add ( unitImages[0] );

	unitImages[1] = new cMenuImage ( position.x+208, position.y+26 );
	menuItems.Add ( unitImages[1] );

	getNamesNCargoNImages();
	setCargos();
}

cDialogTransfer::~cDialogTransfer()
{
	delete doneButton;
	delete cancelButton;

	delete incButton;
	delete decButton;

	delete resBar;

	delete transferLabel;

	for ( int i = 0; i < 2; i++ )
	{
		delete unitImages[i];

		delete unitNameLabels[i];
		delete unitCargoLabels[i];
	}

	float fNewZoom = Client->gameGUI.getZoom();

	if ( srcBuilding != NULL )
	{
		scaleSurface ( srcBuilding->typ->img_org, srcBuilding->typ->img, ( int ) ( srcBuilding->typ->img_org->w* fNewZoom ) , ( int ) ( srcBuilding->typ->img_org->h* fNewZoom ) );
	}
	else
	{
		scaleSurface ( srcVehicle->typ->img_org[0], srcVehicle->typ->img[0], ( int ) ( srcVehicle->typ->img_org[0]->w* fNewZoom ) , ( int ) (srcVehicle->typ->img_org[0]->h* fNewZoom ) );
	}

	Client->gameGUI.mouseInputMode = normalInput;

	if ( destBuilding ) scaleSurface ( destBuilding->typ->img_org, destBuilding->typ->img, ( int ) ( destBuilding->typ->img_org->w* fNewZoom ), ( int ) ( destBuilding->typ->img_org->h* fNewZoom ) );
	else scaleSurface ( destVehicle->typ->img_org[0], destVehicle->typ->img[0], ( int ) ( destVehicle->typ->img_org[0]->w* fNewZoom ), ( int ) ( destVehicle->typ->img_org[0]->h* fNewZoom ) );
}

void cDialogTransfer::getTransferType()
{
	sUnitData::eStorageResType tmpTransferType;

	if ( srcVehicle ) tmpTransferType = srcVehicle->data.storeResType;
	else if ( destVehicle ) tmpTransferType = destVehicle->data.storeResType;
	else
	{
		if ( srcBuilding->data.storeResType != destBuilding->data.storeResType )
		{
			end = true;
			return;
		}
		else tmpTransferType = destBuilding->data.storeResType;
	}

	switch ( tmpTransferType )
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
	}
}

void cDialogTransfer::getNamesNCargoNImages ()
{
	SDL_Surface *unitImage1, *unitImage2;

	if ( srcBuilding )
	{
		scaleSurface ( srcBuilding->typ->img_org, srcBuilding->typ->img, Round ( (float)srcBuilding->typ->img_org->w / srcBuilding->typ->img_org->h ) * 64, 64 );
		SDL_Rect src = { 0, 0, srcBuilding->typ->img->w, srcBuilding->typ->img->h };
		if ( srcBuilding->data.hasFrames ) src.w /= srcBuilding->data.hasFrames;
		if ( srcBuilding->data.isConnectorGraphic || srcBuilding->data.hasClanLogos ) src.w = src.h;
		unitImage1 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_FillRect ( unitImage1, NULL, 0xFF00FF );
		SDL_SetColorKey ( unitImage1, SDL_SRCCOLORKEY, 0xFF00FF );
		if ( srcBuilding->data.hasPlayerColor ) SDL_BlitSurface ( srcBuilding->owner->color, NULL, unitImage1, NULL );
		SDL_BlitSurface ( srcBuilding->typ->img, &src, unitImage1, NULL );

		unitNameLabels[0]->setText ( srcBuilding->data.name );
		if ( destVehicle )
		{
			switch ( destVehicle->data.storeResType )
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
			}
		}
		else
		{
			maxSrcCargo = srcBuilding->data.storageResMax;
			srcCargo = srcBuilding->data.storageResCur;
		}
	}
	else if ( srcVehicle )
	{
		scaleSurface ( srcVehicle->typ->img_org[0], srcVehicle->typ->img[0], Round ( (float)srcVehicle->typ->img_org[0]->w / srcVehicle->typ->img_org[0]->h ) * 64, 64 );
		unitImage1 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, srcVehicle->typ->img[0]->w, srcVehicle->typ->img[0]->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_SetColorKey ( unitImage1, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( srcVehicle->owner->color, NULL, unitImage1, NULL );
		SDL_BlitSurface ( srcVehicle->typ->img[0], NULL, unitImage1, NULL );

		unitNameLabels[0]->setText ( srcVehicle->data.name );
		maxSrcCargo = srcVehicle->data.storageResMax;
		srcCargo = srcVehicle->data.storageResCur;
	}

	if ( destBuilding )
	{
		scaleSurface ( destBuilding->typ->img_org, destBuilding->typ->img, Round ( (float)destBuilding->typ->img_org->w / destBuilding->typ->img_org->h ) * 64, 64 );
		SDL_Rect src = { 0, 0, destBuilding->typ->img->w, destBuilding->typ->img->h };
		if ( destBuilding->data.hasFrames ) src.w /= destBuilding->data.hasFrames;
		if ( destBuilding->data.isConnectorGraphic || destBuilding->data.hasClanLogos ) src.w = src.h;
		unitImage2 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_FillRect ( unitImage2, NULL, 0xFF00FF );
		SDL_SetColorKey ( unitImage2, SDL_SRCCOLORKEY, 0xFF00FF );
		if ( destBuilding->data.hasPlayerColor ) SDL_BlitSurface ( destBuilding->owner->color, NULL, unitImage2, NULL );
		SDL_BlitSurface ( destBuilding->typ->img, &src, unitImage2, NULL );

		unitNameLabels[1]->setText ( destBuilding->data.name );
		if ( srcVehicle )
		{
			switch ( srcVehicle->data.storeResType )
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
		scaleSurface ( destVehicle->typ->img_org[0], destVehicle->typ->img[0], Round ( (float)destVehicle->typ->img_org[0]->w / destVehicle->typ->img_org[0]->h ) * 64, 64 );
		unitImage2 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, destVehicle->typ->img[0]->w, destVehicle->typ->img[0]->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_SetColorKey ( unitImage2, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( destVehicle->owner->color, NULL, unitImage2, NULL );
		SDL_BlitSurface ( destVehicle->typ->img[0], NULL, unitImage2, NULL );

		unitNameLabels[1]->setText ( destVehicle->data.name );
		maxDestCargo = destVehicle->data.storageResMax;
		destCargo = destVehicle->data.storageResCur;
	}

	unitImages[0]->setImage ( unitImage1 );
	unitImages[1]->setImage ( unitImage2 );
	transferValue = maxDestCargo;
}

void cDialogTransfer::setCargos()
{
	if ( srcCargo - transferValue < 0 ) transferValue += srcCargo - transferValue;
	if ( destCargo + transferValue < 0 ) transferValue -= destCargo + transferValue;
	if ( destCargo + transferValue > maxDestCargo ) transferValue -= ( destCargo + transferValue ) - maxDestCargo;
	if ( srcCargo - transferValue > maxSrcCargo ) transferValue += ( srcCargo - transferValue ) - maxSrcCargo;

	unitCargoLabels[0]->setText ( iToStr (srcCargo - transferValue) );
	unitCargoLabels[1]->setText ( iToStr (destCargo + transferValue) );

	transferLabel->setText ( iToStr ( abs(transferValue) ) );

	resBar->setCurrentValue ( (int)( 223 * (float)(destCargo+transferValue) / maxDestCargo ) );
}

void cDialogTransfer::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	switch ( key.keysym.sym )
	{
	case SDLK_RETURN:
		if ( key.state == SDL_PRESSED && !doneButton->getIsClicked() ) doneButton->clicked ( this );
		else if ( key.state == SDL_RELEASED && doneButton->getIsClicked() ) doneButton->released ( this );
		break;
	case SDLK_ESCAPE:
		if ( key.state == SDL_PRESSED && !cancelButton->getIsClicked() ) cancelButton->clicked ( this );
		else if ( key.state == SDL_RELEASED && cancelButton->getIsClicked() ) cancelButton->released ( this );
		break;
	}
}

void cDialogTransfer::doneReleased( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);

	if ( menu->transferValue != 0 )
	{
		if ( menu->srcBuilding )
		{
			if ( menu->destBuilding ) sendWantTransfer ( false, menu->srcBuilding->iID, false, menu->destBuilding->iID, menu->transferValue, menu->srcBuilding->data.storeResType );
			else sendWantTransfer ( false, menu->srcBuilding->iID, true, menu->destVehicle->iID, menu->transferValue, menu->srcBuilding->data.storeResType );
		}
		else
		{
			if ( menu->destBuilding ) sendWantTransfer ( true, menu->srcVehicle->iID, false, menu->destBuilding->iID, menu->transferValue, menu->srcVehicle->data.storeResType );
			else sendWantTransfer ( true, menu->srcVehicle->iID, true, menu->destVehicle->iID, menu->transferValue, menu->srcVehicle->data.storeResType );
		}
	}

	menu->end = true;
}

void cDialogTransfer::cancelReleased( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);
	menu->terminate = true;
}

void cDialogTransfer::incReleased( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);
	menu->transferValue++;
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::decReleased( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);
	menu->transferValue--;
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::barClicked( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);
	menu->transferValue = Round ( (mouse->x-menu->resBar->getPosition().x) * ( menu->maxDestCargo / 223.0 ) - menu->destCargo );
	menu->setCargos();
	menu->draw();
}

void cDialogTransfer::handleDestroyUnit( cBuilding *destroyedBuilding, cVehicle *destroyedVehicle )
{
	if ( destroyedBuilding == srcBuilding || destroyedVehicle == srcVehicle ||
		 destroyedBuilding == destBuilding || destroyedVehicle == destVehicle ) terminate = true;
}

void drawContextItem(string sText, bool bPressed, int x, int y, SDL_Surface *surface)
{
	SDL_Rect dest={x,y,42,21};
	SDL_Rect src={0,0,42,21}; //default button deselected
	if(bPressed) src.y+=21;

	SDL_BlitSurface ( GraphicsData.gfx_context_menu, &src, surface, &dest );
	font->showTextCentered ( dest.x + dest.w / 2, dest.y + (dest.h / 2 - font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2) +1, sText, FONT_LATIN_SMALL_WHITE );
}

cDialogResearch::cDialogResearch( cPlayer *owner_ ) : cMenu ( LoadPCX(GFXOD_DIALOG_RESEARCH), MNU_BG_ALPHA ), owner(owner_)
{
	titleLabel = new cMenuLabel ( position.x+position.w/2, position.y+19, lngPack.i18n( "Text~Title~Labs" ) );
	titleLabel->setCentered ( true );
	menuItems.Add ( titleLabel );

	centersLabel = new cMenuLabel ( position.x+58, position.y+52, lngPack.i18n( "Text~Comp~Labs" ) );
	centersLabel->setCentered ( true );
	menuItems.Add ( centersLabel );

	themeLabel = new cMenuLabel ( position.x+200, position.y+52, lngPack.i18n( "Text~Comp~Themes" ) );
	themeLabel->setCentered ( true );
	menuItems.Add ( themeLabel );

	turnsLabel = new cMenuLabel ( position.x+313, position.y+52, lngPack.i18n( "Text~Comp~Turns" ) );
	turnsLabel->setCentered ( true );
	menuItems.Add ( turnsLabel );

	doneButton = new cMenuButton ( position.x+193, position.y+294, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	doneButton->setReleasedFunction ( &doneReleased );
	menuItems.Add ( doneButton );

	cancelButton = new cMenuButton ( position.x+91, position.y+294, lngPack.i18n ("Text~Button~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	cancelButton->setReleasedFunction ( &cancelReleased );
	menuItems.Add ( cancelButton );

	string themeNames[8] = {
		lngPack.i18n ( "Text~Vehicles~Damage" ),
		lngPack.i18n ( "Text~Hud~Shots" ),
		lngPack.i18n ( "Text~Hud~Range" ),
		lngPack.i18n ( "Text~Hud~Armor" ),
		lngPack.i18n ( "Text~Hud~Hitpoints" ),
		lngPack.i18n ( "Text~Hud~Speed" ),
		lngPack.i18n ( "Text~Hud~Scan" ),
		lngPack.i18n ( "Text~Vehicles~Costs" ) };

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
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &attackSymbol, background, &dest );
	dest.x = 165;
	dest.y = 102;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &shotsSymbol, background, &dest );
	dest.x = 166;
	dest.y = 127;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &rangeSymbol, background, &dest );
	dest.x = 167;
	dest.y = 154;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &armorSymbol, background, &dest );
	dest.x = 169;
	dest.y = 184;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &hitpointsSymbol, background, &dest );
	dest.x = 167;
	dest.y = 212;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &speedSymbol, background, &dest );
	dest.x = 166;
	dest.y = 239;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &scanSymbol, background, &dest );
	dest.x = 166;
	dest.y = 268;
	SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &costsSymbol, background, &dest );

	for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
	{
		centerCountLabels[i] = new cMenuLabel ( position.x+43, position.y+71+28*i, "0" );
		centerCountLabels[i]->setCentered ( true );
		menuItems.Add ( centerCountLabels[i] );

		themeNameLabels[i] = new cMenuLabel ( position.x+183, position.y+71+28*i, themeNames[i] );
		menuItems.Add ( themeNameLabels[i] );

		percentageLabels[i] = new cMenuLabel ( position.x+258, position.y+71+28*i, "+"+iToStr (owner->researchLevel.getCurResearchLevel(i))+"%" );
		percentageLabels[i]->setCentered ( true );
		menuItems.Add ( percentageLabels[i] );

		turnsLabels[i] = new cMenuLabel ( position.x+313, position.y+71+28*i, "" );
		turnsLabels[i]->setCentered ( true );
		menuItems.Add ( turnsLabels[i] );

		incButtons[i] = new cMenuButton ( position.x+143, position.y+70+28*i, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL );
		incButtons[i]->setReleasedFunction ( &incReleased );
		menuItems.Add ( incButtons[i] );

		decButtons[i] = new cMenuButton ( position.x+71, position.y+70+28*i, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL );
		decButtons[i]->setReleasedFunction ( &decReleased );
		menuItems.Add ( decButtons[i] );

		scroller[i] = new cMenuScrollerHandler ( position.x+90, position.y+70+28*i, 51, owner->ResearchCount );
		scroller[i]->setClickedFunction ( &sliderClicked );
		menuItems.Add ( scroller[i] );
	}

	unusedResearch = owner->ResearchCount;
	for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
	{
		newResearchSettings[i] = owner->researchCentersWorkingOnArea[i];
		unusedResearch -= newResearchSettings[i];
	}

	setData();
}

cDialogResearch::~cDialogResearch()
{
	delete centersLabel;
	delete themeLabel;
	delete turnsLabel;

	delete doneButton;
	delete cancelButton;

	for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
	{
		delete incButtons[i];
		delete decButtons[i];

		delete scroller[i];

		delete centerCountLabels[i];
		delete themeNameLabels[i];
		delete percentageLabels[i];
		delete turnsLabels[i];
	}
}

void cDialogResearch::setData()
{
	for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
	{
		centerCountLabels[i]->setText ( iToStr ( newResearchSettings[i] ) );
		scroller[i]->setValue ( newResearchSettings[i] );

		turnsLabels[i]->setText ( iToStr ( owner->researchLevel.getRemainingTurns (i, newResearchSettings[i]) ) );

		incButtons[i]->setLocked ( unusedResearch <= 0 );
		decButtons[i]->setLocked ( newResearchSettings[i] <= 0 );
	}
}

void cDialogResearch::handleKeyInput( SDL_KeyboardEvent &key, string ch )
{
	switch ( key.keysym.sym )
	{
	case SDLK_RETURN:
		if ( key.state == SDL_PRESSED && !doneButton->getIsClicked() ) doneButton->clicked ( this );
		else if ( key.state == SDL_RELEASED && doneButton->getIsClicked() ) doneButton->released ( this );
		break;
	case SDLK_ESCAPE:
		if ( key.state == SDL_PRESSED && !cancelButton->getIsClicked() ) cancelButton->clicked ( this );
		else if ( key.state == SDL_RELEASED && cancelButton->getIsClicked() ) cancelButton->released ( this );
		break;
	}
}

void cDialogResearch::doneReleased( void *parent )
{
	cDialogResearch* menu = static_cast<cDialogResearch*>((cMenu*)parent);
	sendWantResearchChange ( menu->newResearchSettings, menu->owner->Nr );
	menu->end = true;
}

void cDialogResearch::cancelReleased( void *parent )
{
	cDialogResearch* menu = static_cast<cDialogResearch*>((cMenu*)parent);
	menu->terminate = true;
}

void cDialogResearch::incReleased( void *parent )
{
	cDialogResearch* menu = static_cast<cDialogResearch*>((cMenu*)parent);
	if ( menu->unusedResearch > 0 )
	{
		menu->unusedResearch--;
		for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
		{
			if ( menu->incButtons[i]->overItem( mouse->x, mouse->y ) )
			{
				menu->newResearchSettings[i]++;
				break;
			}
		}
		menu->setData();
		menu->draw();
	}
}

void cDialogResearch::decReleased( void *parent )
{
	cDialogResearch* menu = static_cast<cDialogResearch*>((cMenu*)parent);
	if ( menu->unusedResearch < menu->owner->ResearchCount )
	{
		menu->unusedResearch++;
		for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
		{
			if ( menu->decButtons[i]->overItem( mouse->x, mouse->y ) )
			{
				menu->newResearchSettings[i]--;
				break;
			}
		}
		menu->setData();
		menu->draw();
	}
}

void cDialogResearch::sliderClicked( void *parent )
{
	cDialogResearch* menu = static_cast<cDialogResearch*>((cMenu*)parent);
	for ( int i = 0; i < cResearch::kNrResearchAreas; i++ )
	{
		if ( menu->scroller[i]->overItem( mouse->x, mouse->y ) )
		{
			int posX = mouse->x - menu->scroller[i]->getPosition().x;
			int wantResearch = Round ( (float)menu->owner->ResearchCount / menu->scroller[i]->getPosition().w * posX );
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

void cDialogResearch::handleDestroyUnit( cBuilding *destroyedBuilding, cVehicle *destroyedVehicle )
{
	if ( destroyedBuilding && destroyedBuilding->data.canResearch && destroyedBuilding->owner == owner && destroyedBuilding->IsWorking ) terminate = true;
}
