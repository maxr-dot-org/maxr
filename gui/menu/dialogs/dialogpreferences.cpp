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

#include <functional>

#include <SDL_mixer.h>

#include "dialogpreferences.h"
#include "../widgets/label.h"
#include "../widgets/pushbutton.h"
#include "../widgets/slider.h"
#include "../widgets/checkbox.h"
#include "../widgets/lineedit.h"
#include "../../../pcx.h"
#include "../../../main.h"
#include "../../../settings.h"
#include "../../../video.h"

#include "../widgets/tools/validatorint.h"

//------------------------------------------------------------------------------
cDialogNewPreferences::cDialogNewPreferences () :
	cWindow (LoadPCX (GFXOD_DIALOG5), eWindowBackgrounds::Alpha)
{
	// blit black titlebar behind textfield for playername
	SDL_Rect src = {108, 12, 186, 18};
	SDL_Rect dest = {140, 154, 0, 0};
	SDL_BlitSurface (surface, &src, surface, &dest);

	const auto& menuPosition = getArea ().getMinCorner ();

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (0, 15), menuPosition + cPosition (getArea ().getMaxCorner ().x(), 25)), lngPack.i18n ("Text~Settings~Preferences"), FONT_LATIN_NORMAL, eAlignmentType::Center));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 56), menuPosition + cPosition (135, 66)), lngPack.i18n ("Text~Settings~Volume"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	
	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 56 + 20), menuPosition + cPosition (135, 66 + 20)), lngPack.i18n ("Text~Settings~Music"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	musicVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (menuPosition + cPosition (140, 53 + 20), menuPosition + cPosition (240, 70 + 20)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (musicVolumeSlider->valueChanged, std::bind (&cDialogNewPreferences::musicVolumeChanged, this));
	disableMusicCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (245, 73), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableMusicCheckBox->toggled, std::bind (&cDialogNewPreferences::musicMuteChanged, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 56 + 20 * 2), menuPosition + cPosition (135, 66 + 20 * 2)), lngPack.i18n ("Text~Settings~Effects"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	effectsVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (menuPosition + cPosition (140, 53 + 20 * 2), menuPosition + cPosition (240, 70 + 20 * 2)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (effectsVolumeSlider->valueChanged, std::bind (&cDialogNewPreferences::effectsVolumeChanged, this));
	disableEffectsCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (245, 73 + 20), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableEffectsCheckBox->toggled, std::bind (&cDialogNewPreferences::effectsMuteChanged, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 56 + 20 * 3), menuPosition + cPosition (135, 66 + 20 * 3)), lngPack.i18n ("Text~Settings~Voices"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	voicesVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (menuPosition + cPosition (140, 53 + 20 * 3), menuPosition + cPosition (240, 70 + 20 * 3)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (voicesVolumeSlider->valueChanged, std::bind (&cDialogNewPreferences::voicesVolumeChanged, this));
	disableVoicesCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (245, 73 + 20 * 2), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableVoicesCheckBox->toggled, std::bind (&cDialogNewPreferences::voicesMuteChanged, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 158), menuPosition + cPosition (135, 168)), lngPack.i18n ("Text~Title~Player_Name"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	nameEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (menuPosition + cPosition (140, 154), menuPosition + cPosition (140 + 185, 154 + 18)), eLineEditFrameType::Box));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (menuPosition + cPosition (25, 257), menuPosition + cPosition (135, 267)), lngPack.i18n ("Text~Settings~Scrollspeed"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	scrollSpeedSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (menuPosition + cPosition (140, 254), menuPosition + cPosition (240, 271)), 0, 128, eOrientationType::Horizontal));

	animationCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 193), lngPack.i18n ("Text~Settings~Animation")));
	shadowsCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 193 + 20), lngPack.i18n ("Text~Settings~Shadows")));
	aplhaCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 193 + 20 * 2), lngPack.i18n ("Text~Settings~Alphaeffects")));
	demageBuildingsCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (210, 193), lngPack.i18n ("Text~Settings~ShowDamage")));
	demageVehiclesCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (210, 193 + 20), lngPack.i18n ("Text~Settings~ShowDamageVehicle")));
	tracksCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (210, 193 + 20 * 2), lngPack.i18n ("Text~Settings~Tracks")));

	autosaveCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 294), lngPack.i18n ("Text~Settings~Autosave")));
	introCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 294 + 20), lngPack.i18n ("Text~Settings~Intro")));
	windowCheckBox = addChild (std::make_unique<cCheckBox> (menuPosition + cPosition (25, 294 + 20 * 2), lngPack.i18n ("Text~Settings~Window")));

	auto doneButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (208, 383), ePushButtonType::Angular, lngPack.i18n ("Text~Button~Done"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (doneButton->clicked, std::bind (&cDialogNewPreferences::doneClicked, this));

	auto cancelButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (118, 383), ePushButtonType::Angular, lngPack.i18n ("Text~Button~Cancel"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (cancelButton->clicked, std::bind (&cDialogNewPreferences::cancelClicked, this));

	loadValues ();
}

//------------------------------------------------------------------------------
cDialogNewPreferences::~cDialogNewPreferences ()
{}

//------------------------------------------------------------------------------
void cDialogNewPreferences::loadValues ()
{
	musicVolumeSlider->setValue (cSettings::getInstance ().getMusicVol ());
	effectsVolumeSlider->setValue (cSettings::getInstance ().getSoundVol());
	voicesVolumeSlider->setValue (cSettings::getInstance ().getVoiceVol ());

	disableMusicCheckBox->setChecked (cSettings::getInstance ().isMusicMute ());
	disableEffectsCheckBox->setChecked (cSettings::getInstance ().isSoundMute ());
	disableVoicesCheckBox->setChecked (cSettings::getInstance ().isVoiceMute ());

	scrollSpeedSlider->setValue (cSettings::getInstance ().getScrollSpeed());

	nameEdit->setText (cSettings::getInstance ().getPlayerName ());

	animationCheckBox->setChecked (cSettings::getInstance ().isAnimations ());
	shadowsCheckBox->setChecked (cSettings::getInstance ().isShadows ());
	aplhaCheckBox->setChecked (cSettings::getInstance ().isAlphaEffects ());
	demageBuildingsCheckBox->setChecked (cSettings::getInstance ().isDamageEffects ());
	demageVehiclesCheckBox->setChecked (cSettings::getInstance ().isDamageEffectsVehicles ());
	tracksCheckBox->setChecked (cSettings::getInstance ().isMakeTracks ());

	autosaveCheckBox->setChecked (cSettings::getInstance ().shouldAutosave ());
	introCheckBox->setChecked (cSettings::getInstance ().shouldShowIntro ());
	windowCheckBox->setChecked (Video.getWindowMode ());

	storePreviewValues ();
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::saveValues ()
{
	cSettings::getInstance ().setPlayerName (nameEdit->getText ().c_str ());

	cSettings::getInstance ().setAnimations (animationCheckBox->isChecked ());
	cSettings::getInstance ().setShadows (shadowsCheckBox->isChecked ());
	cSettings::getInstance ().setAlphaEffects (aplhaCheckBox->isChecked ());
	cSettings::getInstance ().setDamageEffects (demageBuildingsCheckBox->isChecked ());
	cSettings::getInstance ().setDamageEffectsVehicles (demageVehiclesCheckBox->isChecked ());
	cSettings::getInstance ().setMakeTracks (tracksCheckBox->isChecked ());

	cSettings::getInstance ().setAutosave (autosaveCheckBox->isChecked ());
	cSettings::getInstance ().setShowIntro (introCheckBox->isChecked ());
	Video.setWindowMode (windowCheckBox->isChecked ());
	cSettings::getInstance ().saveWindowMode ();

	cSettings::getInstance ().setScrollSpeed (scrollSpeedSlider->getValue ());

	// save resolution
	const auto oldScreenX = Video.getResolutionX ();
	const auto oldScreenY = Video.getResolutionY ();

	// FIXME: get new resolution
	const auto newResolutionX = oldScreenX;
	const auto newResolutionY = oldScreenY;

	if (newResolutionX != oldScreenX || newResolutionY != oldScreenY)
	{
		Video.setResolution (newResolutionX, newResolutionY, true);
		cSettings::getInstance ().saveResolution ();

		if (Video.getResolutionX () != oldScreenX || Video.getResolutionY () != oldScreenY)
		{
			// TODO: dialog
			//cDialogOK okDialog (lngPack.i18n ("Text~Comp~ResolutionChange"));
		}
	}
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::storePreviewValues ()
{
	storedMusicVolume = cSettings::getInstance ().getMusicVol ();
	storedEffectsVolume = cSettings::getInstance ().getSoundVol ();
	storedVoicesVolume = cSettings::getInstance ().getVoiceVol ();

	storedMusicMute = cSettings::getInstance ().isMusicMute ();
	storedEffectsMute = cSettings::getInstance ().isSoundMute ();
	storedVoicesMute = cSettings::getInstance ().isVoiceMute ();
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::restorePreviewValues ()
{
	cSettings::getInstance ().setMusicVol (storedMusicVolume);
	cSettings::getInstance ().setSoundVol (storedEffectsVolume);
	cSettings::getInstance ().setVoiceVol (storedVoicesVolume);
	if (cSettings::getInstance ().isSoundEnabled ()) Mix_VolumeMusic (cSettings::getInstance ().getMusicVol ());
	if (cSettings::getInstance ().isSoundEnabled ()) Mix_Volume (SoundLoopChannel, cSettings::getInstance ().getSoundVol ());

	bool wasMusicMute = cSettings::getInstance ().isMusicMute ();
	cSettings::getInstance ().setMusicMute (storedMusicMute);
	if (wasMusicMute && !storedMusicMute) StartMusic ();
	cSettings::getInstance ().setSoundMute (storedEffectsMute);
	cSettings::getInstance ().setVoiceMute (storedVoicesMute);
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::doneClicked ()
{
	saveValues ();
	close ();
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::cancelClicked ()
{
	restorePreviewValues ();
	close ();
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::musicVolumeChanged ()
{
	cSettings::getInstance ().setMusicVol (musicVolumeSlider->getValue ());
	if (cSettings::getInstance ().isSoundEnabled ()) Mix_VolumeMusic (cSettings::getInstance ().getMusicVol ());
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::effectsVolumeChanged ()
{
	cSettings::getInstance ().setSoundVol (effectsVolumeSlider->getValue ());
	if (cSettings::getInstance ().isSoundEnabled ()) Mix_Volume (SoundLoopChannel, cSettings::getInstance ().getSoundVol ());
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::voicesVolumeChanged ()
{
	cSettings::getInstance ().setVoiceVol (voicesVolumeSlider->getValue ());
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::musicMuteChanged ()
{
	bool wasMute = cSettings::getInstance ().isMusicMute ();
	cSettings::getInstance ().setMusicMute (disableMusicCheckBox->isChecked ());
	if (cSettings::getInstance ().isMusicMute ()) StopMusic ();
	if (!cSettings::getInstance ().isMusicMute () && wasMute) StartMusic ();
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::effectsMuteChanged ()
{
	cSettings::getInstance ().setSoundMute (disableEffectsCheckBox->isChecked ());
}

//------------------------------------------------------------------------------
void cDialogNewPreferences::voicesMuteChanged ()
{
	cSettings::getInstance ().setVoiceMute (disableVoicesCheckBox->isChecked ());
}
