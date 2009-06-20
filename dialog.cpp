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

cDialogYesNow::cDialogYesNow( string text ) : cMenu ( LoadPCX ( GFXOD_DIALOG2 ), MNU_BG_ALPHA )
{
	textLabel = new cMenuLabel ( position.x+20, position.y+20, text );
	textLabel->setBox ( position.w-40, position.h-150 );
	menuItems.Add ( textLabel );

	yesButton = new cMenuButton ( position.x+155, position.y+185, lngPack.i18n ("Text~Button~Yes"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	yesButton->setReleasedFunction ( &yesReleased );
	menuItems.Add ( yesButton );

	noButton = new cMenuButton ( position.x+67, position.y+185, lngPack.i18n ("Text~Button~No"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	noButton->setReleasedFunction ( &noReleased );
	menuItems.Add ( noButton );
}

cDialogYesNow::~cDialogYesNow()
{
	delete textLabel;
	delete yesButton;
	delete noButton;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cDialogYesNow::yesReleased( void *parent )
{
	cDialogYesNow* menu = static_cast<cDialogYesNow*>((cMenu*)parent);
	menu->end = true;
}

void cDialogYesNow::noReleased( void *parent )
{
	cDialogYesNow* menu = static_cast<cDialogYesNow*>((cMenu*)parent);
	menu->terminate = true;
}

cDialogOK::cDialogOK( string text ) : cMenu ( LoadPCX( GFXOD_DIALOG2 ), MNU_BG_ALPHA )
{
	textLabel = new cMenuLabel ( position.x+20, position.y+20, text );
	textLabel->setBox ( position.w-40, position.h-150 );
	menuItems.Add ( textLabel );

	okButton = new cMenuButton ( position.x+111, position.y+185, lngPack.i18n ("Text~Button~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );
}

cDialogOK::~cDialogOK()
{
	delete textLabel;
	delete okButton;

	if ( Client ) Client->bFlagDrawHud = true;
}

void cDialogOK::okReleased( void *parent )
{
	cDialogOK* menu = static_cast<cDialogOK*>((cMenu*)parent);
	menu->end = true;
}

cDialogLicence::cDialogLicence() : cMenu ( LoadPCX( GFXOD_DIALOG4 ), MNU_BG_ALPHA )
{
	generateLicenceTexts();

	maxrLabel = new cMenuLabel ( position.x+position.w/2, position.y+30, "\"M.A.X. Reloaded\"" );
	maxrLabel->setCentered ( true );
	menuItems.Add ( maxrLabel );

	headerLabel = new cMenuLabel ( position.x+position.w/2, position.y+30+font->getFontHeight(), "(C) 2007 by its authors" );
	headerLabel->setCentered ( true );
	menuItems.Add ( headerLabel );

	textLabel = new cMenuLabel ( position.x+35, position.y+30+3*font->getFontHeight() );
	textLabel->setBox ( 232, 142 );
	menuItems.Add ( textLabel );

	okButton = new cMenuButton ( position.x+111, position.y+185, lngPack.i18n ("Text~Button~OK"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	upButton = new cMenuButton ( position.x+241, position.y+187, "", cMenuButton::BUTTON_TYPE_ARROW_UP_SMALL );
	upButton->setReleasedFunction ( &upReleased );
	menuItems.Add ( upButton );

	downButton = new cMenuButton ( position.x+261, position.y+187, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_SMALL );
	downButton->setReleasedFunction ( &downReleased );
	menuItems.Add ( downButton );

	offset = 0;
	resetText();
}

cDialogLicence::~cDialogLicence()
{
	delete maxrLabel;
	delete headerLabel;
	delete textLabel;

	delete okButton;
	delete upButton;
	delete downButton;

	if ( Client ) Client->bFlagDrawHud = true;
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
		textLabel->setText ( sLicence1 );
		upButton->setLocked ( true );
		break;
	case 1:
		textLabel->setText ( sLicence2 );
		upButton->setLocked ( false );
		break;
	case 2:
		textLabel->setText ( sLicence3 );
		textLabel->setFontType ( FONT_LATIN_NORMAL );
		downButton->setLocked ( false );
		headerLabel->setText ( "(C) 2007 by its authors" );
		break;
	case 3:
		textLabel->setText ( sLicence4 );
		textLabel->setFontType ( FONT_LATIN_SMALL_WHITE );
		downButton->setLocked ( true );
		headerLabel->setText ( "AUTHORS:" );
		break;
	}
	draw();
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
	menuItems.Add ( disableMusicChBox );
	musicSlider = new cMenuSlider ( position.x+140, position.y+81, 128, this );
	musicSlider->setValue ( SettingsData.MusicVol );
	menuItems.Add ( musicSlider );
	menuItems.Add ( musicSlider->scroller );

	effectsLabel = new cMenuLabel ( position.x+25, position.y+56+20*2, lngPack.i18n( "Text~Settings~Effects" ) );
	menuItems.Add ( effectsLabel );
	disableEffectsChBox = new cMenuCheckButton ( position.x+210, position.y+73+20, lngPack.i18n( "Text~Settings~Disable" ), SettingsData.SoundMute, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( disableEffectsChBox );
	effectsSlider = new cMenuSlider ( position.x+140, position.y+81+20, 128, this );
	effectsSlider->setValue ( SettingsData.SoundVol );
	menuItems.Add ( effectsSlider );
	menuItems.Add ( effectsSlider->scroller );

	voicesLabel = new cMenuLabel ( position.x+25, position.y+56+20*3, lngPack.i18n( "Text~Settings~Voices" ) );
	menuItems.Add ( voicesLabel );
	disableVoicesChBox = new cMenuCheckButton ( position.x+210, position.y+73+20*2, lngPack.i18n( "Text~Settings~Disable" ), SettingsData.VoiceMute, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( disableVoicesChBox );
	voicesSlider = new cMenuSlider ( position.x+140, position.y+81+20*2, 128, this );
	voicesSlider->setValue ( SettingsData.VoiceVol );
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
	scrollSpeedSlider = new cMenuSlider ( position.x+140, position.y+261, 50, this );
	scrollSpeedSlider->setValue ( SettingsData.iScrollSpeed );
	menuItems.Add ( scrollSpeedSlider );
	menuItems.Add ( scrollSpeedSlider->scroller );

	autosaveChBox = new cMenuCheckButton ( position.x+25, position.y+294, lngPack.i18n( "Text~Settings~Autosave" ), SettingsData.bAutoSave, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( autosaveChBox );

	introChBox = new cMenuCheckButton ( position.x+25, position.y+294+20, lngPack.i18n( "Text~Settings~Intro" ), SettingsData.bIntro, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( introChBox );

	windowChBox = new cMenuCheckButton ( position.x+25, position.y+294+20*2, lngPack.i18n( "Text~Settings~Window" ), SettingsData.bWindowMode, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
	menuItems.Add ( windowChBox );

	resolutions[0] = "640x480";
	resolutions[1] = "800x600";
	resolutions[2] = "1024x768";
	resolutions[3] = "1024x960";
	resolutions[4] = "1280x960";
	resolutions[5] = "1280x1024";

	int resolutionMode;
	if(SettingsData.iScreenW == 640 && SettingsData.iScreenH == 480) resolutionMode = 0;
	else if(SettingsData.iScreenW == 800 && SettingsData.iScreenH == 600) resolutionMode = 1;
	else if(SettingsData.iScreenW == 1024 && SettingsData.iScreenH == 768) resolutionMode = 2;
	else if(SettingsData.iScreenW == 1024 && SettingsData.iScreenH == 960) resolutionMode = 3;
	else if(SettingsData.iScreenW == 1280 && SettingsData.iScreenH == 960) resolutionMode = 4;
	else if(SettingsData.iScreenW == 1280 && SettingsData.iScreenH == 1024) resolutionMode = 5;
	else resolutionMode = -1;

	resoulutionGroup = new cMenuRadioGroup;
	menuItems.Add ( resoulutionGroup );

	for ( int i = 0, x = 0; x < 2; x++ )
	{
		for ( int y = 0; y < 3; y++, i++ )
		{
			cMenuCheckButton *button = new cMenuCheckButton ( position.x+160+100*x, position.y+290+20*y, resolutions[i], resolutionMode == i, false, cMenuCheckButton::CHECKBOX_TYPE_STANDARD );
			resoulutionGroup->addButton ( button );
		}
	}

	okButton = new cMenuButton ( position.x+208, position.y+383, lngPack.i18n ("Text~Button~Done"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	okButton->setReleasedFunction ( &okReleased );
	menuItems.Add ( okButton );

	cancelButton = new cMenuButton ( position.x+118, position.y+383, lngPack.i18n ("Text~Button~Cancel"), cMenuButton::BUTTON_TYPE_ANGULAR, FONT_LATIN_NORMAL );
	cancelButton->setReleasedFunction ( &cancelReleased );
	menuItems.Add ( cancelButton );
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

	if ( Client ) Client->bFlagDrawHud = true;
}

void cDialogPreferences::saveValues()
{

	SettingsData.sPlayerName = nameEdit->getText();
	if ( Client) Client->ActivePlayer->name = SettingsData.sPlayerName;
	
	SettingsData.MusicMute = disableMusicChBox->isChecked();
	SettingsData.SoundMute = disableEffectsChBox->isChecked();
	SettingsData.VoiceMute = disableVoicesChBox->isChecked();

	SettingsData.bAutoSave = autosaveChBox->isChecked();
	SettingsData.bAnimations = animationChBox->isChecked();
	SettingsData.bAlphaEffects = alphaChBox->isChecked();
	SettingsData.bDamageEffects = demageBuilChBox->isChecked();
	SettingsData.bDamageEffectsVehicles = demageVehChBox->isChecked();
	SettingsData.bIntro = introChBox->isChecked();
	SettingsData.bMakeTracks = tracksChBox->isChecked();
	SettingsData.bWindowMode = windowChBox->isChecked();
	SettingsData.bShadows = shadowsChBox->isChecked();

	SettingsData.MusicVol = musicSlider->getValue();
	SettingsData.SoundVol = effectsSlider->getValue();
	SettingsData.VoiceVol = voicesSlider->getValue();
	SettingsData.iScrollSpeed = scrollSpeedSlider->getValue();

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

	for ( int i = 0; i < 6; i++ )
	{
		if ( resoulutionGroup->buttonIsChecked ( i ) )
		{
			SettingsData.iScreenW = atoi ( resolutions[i].substr ( 0, resolutions[i].find_first_of ( "x", 0 ) ).c_str() );
			SettingsData.iScreenH = atoi ( resolutions[i].substr ( resolutions[i].find_first_of ( "x", 0 )+1, resolutions[i].length() ).c_str() );
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
	menu->terminate = true;
}

cDialogTransfer::cDialogTransfer( cBuilding *srcBuilding_, cVehicle *srcVehicle_, cBuilding *destBuilding_, cVehicle *destVehicle_  ) :
	cMenu ( LoadPCX(GFXOD_DIALOG_TRANSFER), MNU_BG_ALPHA ),
	srcBuilding ( srcBuilding_ ),
	srcVehicle ( srcVehicle_ ),
	destBuilding ( destBuilding_ ),
	destVehicle ( destVehicle_ )
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

	unitNameLabels[0] = new cMenuLabel ( position.x+70, position.y+105 );
	unitNameLabels[0]->setCentered ( true );
	menuItems.Add ( unitNameLabels[0] );

	unitNameLabels[1] = new cMenuLabel ( position.x+240, position.y+105 );
	unitNameLabels[1]->setCentered ( true );
	menuItems.Add ( unitNameLabels[1] );

	unitCargoLabels[0] = new cMenuLabel ( position.x+30, position.y+60 );
	unitCargoLabels[0]->setCentered ( true );
	menuItems.Add ( unitCargoLabels[0] );

	unitCargoLabels[1] = new cMenuLabel ( position.x+280, position.y+60 );
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

	float fNewZoom = (float)(Client->Hud.Zoom / 64.0);

	if ( srcBuilding != NULL )
	{
		scaleSurface ( srcBuilding->typ->img_org, srcBuilding->typ->img, ( int ) ( srcBuilding->typ->img_org->w* fNewZoom ) , ( int ) ( srcBuilding->typ->img_org->h* fNewZoom ) );
		srcBuilding->Transfer = false;
	}
	else
	{
		scaleSurface ( srcVehicle->typ->img_org[0], srcVehicle->typ->img[0], ( int ) ( srcVehicle->typ->img_org[0]->w* fNewZoom ) , ( int ) (srcVehicle->typ->img_org[0]->h* fNewZoom ) );
		srcVehicle->Transfer = false;
	}

	if ( destBuilding ) scaleSurface ( destBuilding->typ->img_org, destBuilding->typ->img, ( int ) ( destBuilding->typ->img_org->w* fNewZoom ), ( int ) ( destBuilding->typ->img_org->h* fNewZoom ) );
	else scaleSurface ( destVehicle->typ->img_org[0], destVehicle->typ->img[0], ( int ) ( destVehicle->typ->img_org[0]->w* fNewZoom ), ( int ) ( destVehicle->typ->img_org[0]->h* fNewZoom ) );

	if ( Client ) Client->bFlagDrawHud = true;
}

void cDialogTransfer::getTransferType()
{
	int tmpTransferType;

	if ( srcVehicle ) tmpTransferType = srcVehicle->data.can_transport;
	else if ( destVehicle ) tmpTransferType = destVehicle->data.can_transport;
	else
	{
		if ( srcBuilding->data.can_load != destBuilding->data.can_load )
		{
			end = true;
			return;
		}
		else tmpTransferType = destBuilding->data.can_load;
	}

	switch ( tmpTransferType )
	{
	case TRANS_METAL:
		transferType = cMenuMaterialBar::MAT_BAR_TYPE_METAL_HORI_SMALL;
		break;
	case TRANS_OIL:
		transferType = cMenuMaterialBar::MAT_BAR_TYPE_OIL_HORI_SMALL;
		break;
	case TRANS_GOLD:
		transferType = cMenuMaterialBar::MAT_BAR_TYPE_GOLD_HORI_SMALL;
		break;
	}
}

void cDialogTransfer::getNamesNCargoNImages ()
{
	SDL_Surface *unitImage1, *unitImage2;

	if ( srcBuilding )
	{
		if ( srcBuilding->data.is_mine )
		{
			SDL_Rect src = { 0, 0, 64, 64 };
			scaleSurface ( srcBuilding->typ->img_org, srcBuilding->typ->img, srcBuilding->typ->img_org->w / srcBuilding->typ->img->h * 64, 64 );
			unitImage1 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, 64, srcBuilding->typ->img->h, 32, 0, 0, 0, 0 );
			SDL_SetColorKey ( unitImage1, SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( srcBuilding->owner->color, NULL, unitImage1, NULL );
			if ( srcBuilding->owner->getClan() != -1 ) src.x = (srcBuilding->owner->getClan()+1) * 64;
			SDL_BlitSurface ( srcBuilding->typ->img, &src, unitImage1, NULL );
		}
		else
		{
			scaleSurface ( srcBuilding->typ->img_org, srcBuilding->typ->img, 64, 64 );
			unitImage1 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, srcBuilding->typ->img->w, srcBuilding->typ->img->h, 32, 0, 0, 0, 0 );
			SDL_SetColorKey ( unitImage1, SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( srcBuilding->owner->color, NULL, unitImage1, NULL );
			SDL_BlitSurface ( srcBuilding->typ->img, NULL, unitImage1, NULL );
		}

		unitNameLabels[0]->setText ( srcBuilding->data.szName );
		if ( destVehicle )
		{
			switch ( destVehicle->data.can_transport )
			{
			case TRANS_METAL:
				maxSrcCargo = srcBuilding->SubBase->MaxMetal;
				srcCargo = srcBuilding->SubBase->Metal;
				break;
			case TRANS_OIL:
				maxSrcCargo = srcBuilding->SubBase->MaxOil;
				srcCargo = srcBuilding->SubBase->Oil;
				break;
			case TRANS_GOLD:
				maxSrcCargo = srcBuilding->SubBase->MaxGold;
				srcCargo = srcBuilding->SubBase->Gold;
				break;
			}
		}
		else
		{
			maxSrcCargo = srcBuilding->data.max_cargo;
			srcCargo = srcBuilding->data.cargo;
		}
	}
	else if ( srcVehicle )
	{
		scaleSurface ( srcVehicle->typ->img_org[0], srcVehicle->typ->img[0], 64, 64 );
		unitImage1 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, srcVehicle->typ->img[0]->w, srcVehicle->typ->img[0]->h, 32, 0, 0, 0, 0 );
		SDL_SetColorKey ( unitImage1, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( srcVehicle->owner->color, NULL, unitImage1, NULL );
		SDL_BlitSurface ( srcVehicle->typ->img[0], NULL, unitImage1, NULL );

		unitNameLabels[0]->setText ( srcVehicle->data.szName );
		maxSrcCargo = srcVehicle->data.max_cargo;
		srcCargo = srcVehicle->data.cargo;
	}

	if ( destBuilding )
	{
		if ( destBuilding->data.is_mine )
		{
			SDL_Rect src = { 0, 0, 64, 64 };
			scaleSurface ( destBuilding->typ->img_org, destBuilding->typ->img, destBuilding->typ->img_org->w / destBuilding->typ->img->h * 64, 64 );
			unitImage2 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, 64, destBuilding->typ->img->h, 32, 0, 0, 0, 0 );
			SDL_SetColorKey ( unitImage2, SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( destBuilding->owner->color, NULL, unitImage2, NULL );
			if ( destBuilding->owner->getClan() != -1 ) src.x = (destBuilding->owner->getClan()+1) * 64;
			SDL_BlitSurface ( destBuilding->typ->img, &src, unitImage2, NULL );
		}
		else
		{
			scaleSurface ( destBuilding->typ->img_org, destBuilding->typ->img, 64, 64 );
			unitImage2 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, destBuilding->typ->img->w, destBuilding->typ->img->h, 32, 0, 0, 0, 0 );
			SDL_SetColorKey ( unitImage2, SDL_SRCCOLORKEY, 0xFF00FF );
			SDL_BlitSurface ( destBuilding->owner->color, NULL, unitImage2, NULL );
			SDL_BlitSurface ( destBuilding->typ->img, NULL, unitImage2, NULL );
		}

		unitNameLabels[1]->setText ( destBuilding->data.szName );
		if ( srcVehicle )
		{
			switch ( srcVehicle->data.can_transport )
			{
			case TRANS_METAL:
				maxDestCargo = destBuilding->SubBase->MaxMetal;
				destCargo = destBuilding->SubBase->Metal;
				break;
			case TRANS_OIL:
				maxDestCargo = destBuilding->SubBase->MaxOil;
				destCargo = destBuilding->SubBase->Oil;
				break;
			case TRANS_GOLD:
				maxDestCargo = destBuilding->SubBase->MaxGold;
				destCargo = destBuilding->SubBase->Gold;
				break;
			}
		}
		else
		{
			maxDestCargo = destBuilding->data.max_cargo;
			destCargo = destBuilding->data.cargo;
		}
	}
	else
	{
		scaleSurface ( destVehicle->typ->img_org[0], destVehicle->typ->img[0], 64, 64 );
		unitImage2 = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, destVehicle->typ->img[0]->w, destVehicle->typ->img[0]->h, 32, 0, 0, 0, 0 );
		SDL_SetColorKey ( unitImage2, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( destVehicle->owner->color, NULL, unitImage2, NULL );
		SDL_BlitSurface ( destVehicle->typ->img[0], NULL, unitImage2, NULL );

		unitNameLabels[1]->setText ( destVehicle->data.szName );
		maxDestCargo = destVehicle->data.max_cargo;
		destCargo = destVehicle->data.cargo;
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

void cDialogTransfer::doneReleased( void *parent )
{
	cDialogTransfer* menu = static_cast<cDialogTransfer*>((cMenu*)parent);

	if ( menu->transferValue != 0 )
	{
		if ( menu->srcBuilding )
		{
			if ( menu->destBuilding ) sendWantTransfer ( false, menu->srcBuilding->iID, false, menu->destBuilding->iID, menu->transferValue, menu->srcBuilding->data.can_load );
			else sendWantTransfer ( false, menu->srcBuilding->iID, true, menu->destVehicle->iID, menu->transferValue, menu->srcBuilding->data.can_load );
		}
		else
		{
			if ( menu->destBuilding ) sendWantTransfer ( true, menu->srcVehicle->iID, false, menu->destBuilding->iID, menu->transferValue, menu->srcVehicle->data.can_transport );
			else sendWantTransfer ( true, menu->srcVehicle->iID, true, menu->destVehicle->iID, menu->transferValue, menu->srcVehicle->data.can_transport );
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

void drawContextItem(string sText, bool bPressed, int x, int y, SDL_Surface *surface)
{
	SDL_Rect dest={x,y,42,21};
	SDL_Rect src={0,0,42,21}; //default button deselected
	if(bPressed) src.y+=21;

	SDL_BlitSurface ( GraphicsData.gfx_context_menu, &src, surface, &dest );
	font->showTextCentered ( dest.x + dest.w / 2, dest.y + (dest.h / 2 - font->getFontHeight(FONT_LATIN_SMALL_WHITE) / 2) +1, sText, FONT_LATIN_SMALL_WHITE );

	return;
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
	if ( Client ) Client->bFlagDrawHud = true;
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
