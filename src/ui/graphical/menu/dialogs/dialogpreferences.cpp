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

#include "ui/graphical/menu/dialogs/dialogpreferences.h"

#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/pcx.h"
#include "settings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/colorselector.h"
#include "ui/graphical/menu/widgets/combobox.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"
#include "ui/graphical/menu/widgets/special/textlistviewitem.h"
#include "ui/graphical/menu/widgets/tools/validatorint.h"
#include "ui/uidefines.h"
#include "utility/language.h"
#include "utility/string/iequals.h"

#include <functional>

#include <SDL_mixer.h>

//------------------------------------------------------------------------------
cDialogPreferences::cDialogPreferences() :
	cWindow (LoadPCX (GFXOD_DIALOG5), eWindowBackgrounds::Alpha)
{
	// blit black titlebar behind textfield for playername
	SDL_Rect src = {108, 12, 186, 18};
	SDL_Rect dest = {140, 154, 0, 0};
	SDL_BlitSurface (getSurface(), &src, getSurface(), &dest);

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 15), getPosition() + cPosition (getArea().getMaxCorner().x(), 25)), lngPack.i18n ("Text~Settings~Preferences"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56), getPosition() + cPosition (135, 66)), lngPack.i18n ("Text~Settings~Volume"), FONT_LATIN_NORMAL, eAlignmentType::Left));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20), getPosition() + cPosition (135, 66 + 20)), lngPack.i18n ("Text~Settings~Music"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	musicVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20), getPosition() + cPosition (240, 70 + 20)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (musicVolumeSlider->valueChanged, std::bind (&cDialogPreferences::musicVolumeChanged, this));
	disableMusicCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (245, 73), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableMusicCheckBox->toggled, std::bind (&cDialogPreferences::musicMuteChanged, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20 * 2), getPosition() + cPosition (135, 66 + 20 * 2)), lngPack.i18n ("Text~Settings~Effects"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	effectsVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20 * 2), getPosition() + cPosition (240, 70 + 20 * 2)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (effectsVolumeSlider->valueChanged, std::bind (&cDialogPreferences::effectsVolumeChanged, this));
	disableEffectsCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (245, 73 + 20), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableEffectsCheckBox->toggled, std::bind (&cDialogPreferences::effectsMuteChanged, this));
	effects3DCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (340, 73 + 20), "3D"));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20 * 3), getPosition() + cPosition (135, 66 + 20 * 3)), lngPack.i18n ("Text~Settings~Voices"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	voicesVolumeSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20 * 3), getPosition() + cPosition (240, 70 + 20 * 3)), 0, 128, eOrientationType::Horizontal));
	signalConnectionManager.connect (voicesVolumeSlider->valueChanged, std::bind (&cDialogPreferences::voicesVolumeChanged, this));
	disableVoicesCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (245, 73 + 20 * 2), lngPack.i18n ("Text~Settings~Disable")));
	signalConnectionManager.connect (disableVoicesCheckBox->toggled, std::bind (&cDialogPreferences::voicesMuteChanged, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 158), getPosition() + cPosition (135, 168)), lngPack.i18n ("Text~Title~Player_Name"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	nameEdit = addChild (std::make_unique<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (140, 154), getPosition() + cPosition (140 + 185, 154 + 18)), eLineEditFrameType::Box));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 175), getPosition() + cPosition (135, 185)), lngPack.i18n ("Text~Title~Color"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	colorSelector = addChild (std::make_unique<cColorSelector> (getPosition() + cPosition (160, 172), cRgbColor::black()));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 257), getPosition() + cPosition (135, 267)), lngPack.i18n ("Text~Settings~Scrollspeed"), FONT_LATIN_NORMAL, eAlignmentType::Left));
	scrollSpeedSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 254), getPosition() + cPosition (240, 271)), 0, 128, eOrientationType::Horizontal));

	animationCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 193), lngPack.i18n ("Text~Settings~Animation")));
	shadowsCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 193 + 20), lngPack.i18n ("Text~Settings~Shadows")));
	aplhaCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 193 + 20 * 2), lngPack.i18n ("Text~Settings~Alphaeffects")));
	demageBuildingsCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (210, 193), lngPack.i18n ("Text~Settings~ShowDamage")));
	demageVehiclesCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (210, 193 + 20), lngPack.i18n ("Text~Settings~ShowDamageVehicle")));
	tracksCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (210, 193 + 20 * 2), lngPack.i18n ("Text~Settings~Tracks")));

	autosaveCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 294), lngPack.i18n ("Text~Settings~Autosave")));
	introCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 294 + 20), lngPack.i18n ("Text~Settings~Intro")));
	windowCheckBox = addChild (std::make_unique<cCheckBox> (getPosition() + cPosition (25, 294 + 20 * 2), lngPack.i18n ("Text~Settings~Window")));

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (208, 383), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, std::bind (&cDialogPreferences::doneClicked, this));

	auto cancelButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (118, 383), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Cancel"), FONT_LATIN_NORMAL));
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, std::bind (&cDialogPreferences::cancelClicked, this));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (140, 298 + 20), getPosition() + cPosition (140 + 95, 298 + 20 + 10)), lngPack.i18n ("Text~Settings~Language") + lngPack.i18n ("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Right));
	languagesComboBox = addChild (std::make_unique<cComboBox> (cBox<cPosition> (getPosition() + cPosition (240, 294 + 20), getPosition() + cPosition (240 + 100, 294 + 20 + 17))));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (140, 298), getPosition() + cPosition (140 + 95, 298 + 10)), lngPack.i18n ("Text~Settings~Resolution") + lngPack.i18n ("Text~Punctuation~Colon"), FONT_LATIN_NORMAL, eAlignmentType::Right));
	resolutionsComboBox = addChild (std::make_unique<cComboBox> (cBox<cPosition> (getPosition() + cPosition (240, 294), getPosition() + cPosition (240 + 100, 294 + 17))));

	loadValues();
}

//------------------------------------------------------------------------------
cDialogPreferences::~cDialogPreferences()
{}

//------------------------------------------------------------------------------
void cDialogPreferences::loadValues()
{
	const auto& settings = cSettings::getInstance();
	musicVolumeSlider->setValue (settings.getMusicVol());
	effectsVolumeSlider->setValue (settings.getSoundVol());
	voicesVolumeSlider->setValue (settings.getVoiceVol());

	disableMusicCheckBox->setChecked (settings.isMusicMute());
	disableEffectsCheckBox->setChecked (settings.isSoundMute());
	disableVoicesCheckBox->setChecked (settings.isVoiceMute());

	effects3DCheckBox->setChecked (settings.is3DSound());

	scrollSpeedSlider->setValue (settings.getScrollSpeed());

	nameEdit->setText (settings.getPlayerSettings().name);
	colorSelector->setColor (settings.getPlayerSettings().color);

	animationCheckBox->setChecked (settings.isAnimations());
	shadowsCheckBox->setChecked (settings.isShadows());
	aplhaCheckBox->setChecked (settings.isAlphaEffects());
	demageBuildingsCheckBox->setChecked (settings.isDamageEffects());
	demageVehiclesCheckBox->setChecked (settings.isDamageEffectsVehicles());
	tracksCheckBox->setChecked (settings.isMakeTracks());

	autosaveCheckBox->setChecked (settings.shouldAutosave());
	introCheckBox->setChecked (settings.shouldShowIntro());
	windowCheckBox->setChecked (Video.getWindowMode());

	languagesComboBox->clearItems();
	const auto availableLanguages = lngPack.getAvailableLanguages();
	size_t selectedLanguageIndex = 0;
	for (size_t i = 0; i < availableLanguages.size(); ++i)
	{
		languagesComboBox->addItem (availableLanguages[i]);
		if (iequals (availableLanguages[i], settings.getLanguage()))
		{
			selectedLanguageIndex = i;
		}
	}
	if (!availableLanguages.empty())
	{
		languagesComboBox->setSelectedIndex (selectedLanguageIndex);
	}

	resolutionsComboBox->clearItems();
	const auto& resolutions = Video.getDetectedResolutions();
	for (const auto& resolution : resolutions)
	{
		resolutionsComboBox->addItem (std::to_string (resolution.first) + "x" + std::to_string (resolution.second));
	}
	const auto currentResolutionIndex = Video.validateResolution (Video.getResolutionX(), Video.getResolutionY());
	if (currentResolutionIndex != -1) resolutionsComboBox->setSelectedIndex (currentResolutionIndex);

	storePreviewValues();
}

//------------------------------------------------------------------------------
void cDialogPreferences::saveValues()
{
	cSettings& settings = cSettings::getInstance();

	settings.setPlayerSettings ({nameEdit->getText(), colorSelector->getColor()});

	settings.set3DSound (effects3DCheckBox->isChecked());

	settings.setAnimations (animationCheckBox->isChecked());
	settings.setShadows (shadowsCheckBox->isChecked());
	settings.setAlphaEffects (aplhaCheckBox->isChecked());
	settings.setDamageEffects (demageBuildingsCheckBox->isChecked());
	settings.setDamageEffectsVehicles (demageVehiclesCheckBox->isChecked());
	settings.setMakeTracks (tracksCheckBox->isChecked());

	settings.setAutosave (autosaveCheckBox->isChecked());
	settings.setShowIntro (introCheckBox->isChecked());
	Video.setWindowMode (windowCheckBox->isChecked());
	settings.saveWindowMode();

	settings.setScrollSpeed (scrollSpeedSlider->getValue());

	// save resolution
	const auto oldScreenX = Video.getResolutionX();
	const auto oldScreenY = Video.getResolutionY();

	const auto& resolutionText = resolutionsComboBox->getSelectedText();

	const auto newResolutionX = atoi (resolutionText.substr (0, resolutionText.find ("x")).c_str());
	const auto newResolutionY = atoi (resolutionText.substr (resolutionText.find ("x") + 1).c_str());

	if (newResolutionX > 0 && newResolutionY > 0)
	{
		if (newResolutionX != oldScreenX || newResolutionY != oldScreenY)
		{
			Video.setResolution (newResolutionX, newResolutionY, true);
			settings.saveResolution();

			if (Video.getResolutionX() != oldScreenX || Video.getResolutionY() != oldScreenY)
			{
				auto application = getActiveApplication();
				if (application)
				{
					application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Text~Comp~RestartRequired")));
				}
			}
		}
	}
	else
	{
		// TODO: handle invalid resolution?!

		// lngPack.i18n ("Text~Comp~ResolutionWarning")
		// added info to old langpack if needed, else it can be removed from lang-files - nonsinn
	}

	const auto& selectedLanguage = languagesComboBox->getSelectedText();
	if (!iequals (selectedLanguage, settings.getLanguage()))
	{
		settings.setLanguage (selectedLanguage.c_str());

		auto application = getActiveApplication();
		if (application)
		{
			const auto text = (lngPack.i18n ("Text~Comp~RestartRequired"));
			application->show (std::make_shared<cDialogOk> (text));
		}
	}
}

//------------------------------------------------------------------------------
void cDialogPreferences::storePreviewValues()
{
	storedMusicVolume = cSettings::getInstance().getMusicVol();
	storedEffectsVolume = cSettings::getInstance().getSoundVol();
	storedVoicesVolume = cSettings::getInstance().getVoiceVol();

	storedMusicMute = cSettings::getInstance().isMusicMute();
	storedEffectsMute = cSettings::getInstance().isSoundMute();
	storedVoicesMute = cSettings::getInstance().isVoiceMute();
}

//------------------------------------------------------------------------------
void cDialogPreferences::restorePreviewValues()
{
	cSettings::getInstance().setMusicVol (storedMusicVolume);
	cSettings::getInstance().setSoundVol (storedEffectsVolume);
	cSettings::getInstance().setVoiceVol (storedVoicesVolume);
	if (cSettings::getInstance().isSoundEnabled())
	{
		cSoundDevice::getInstance().setMusicVolume (cSettings::getInstance().getMusicVol());
		cSoundDevice::getInstance().setSoundEffectVolume (cSettings::getInstance().getSoundVol());
		cSoundDevice::getInstance().setVoiceVolume (cSettings::getInstance().getVoiceVol());
	}

	bool wasMusicMute = cSettings::getInstance().isMusicMute();
	cSettings::getInstance().setMusicMute (storedMusicMute);
	if (wasMusicMute && !storedMusicMute) cSoundDevice::getInstance().startRandomMusic();
	cSettings::getInstance().setSoundMute (storedEffectsMute);
	cSettings::getInstance().setVoiceMute (storedVoicesMute);
}

//------------------------------------------------------------------------------
void cDialogPreferences::doneClicked()
{
	saveValues();
	close();
}

//------------------------------------------------------------------------------
void cDialogPreferences::cancelClicked()
{
	restorePreviewValues();
	close();
}

//------------------------------------------------------------------------------
void cDialogPreferences::musicVolumeChanged()
{
	cSettings::getInstance().setMusicVol (musicVolumeSlider->getValue());
	if (cSettings::getInstance().isSoundEnabled()) cSoundDevice::getInstance().setMusicVolume (cSettings::getInstance().getMusicVol());
}

//------------------------------------------------------------------------------
void cDialogPreferences::effectsVolumeChanged()
{
	cSettings::getInstance().setSoundVol (effectsVolumeSlider->getValue());
	if (cSettings::getInstance().isSoundEnabled()) cSoundDevice::getInstance().setSoundEffectVolume (cSettings::getInstance().getSoundVol());
}

//------------------------------------------------------------------------------
void cDialogPreferences::voicesVolumeChanged()
{
	cSettings::getInstance().setVoiceVol (voicesVolumeSlider->getValue());
	if (cSettings::getInstance().isSoundEnabled()) cSoundDevice::getInstance().setVoiceVolume (cSettings::getInstance().getVoiceVol());
}

//------------------------------------------------------------------------------
void cDialogPreferences::musicMuteChanged()
{
	bool wasMute = cSettings::getInstance().isMusicMute();
	cSettings::getInstance().setMusicMute (disableMusicCheckBox->isChecked());
	if (cSettings::getInstance().isMusicMute()) cSoundDevice::getInstance().stopMusic();
	if (!cSettings::getInstance().isMusicMute() && wasMute) cSoundDevice::getInstance().startRandomMusic();
}

//------------------------------------------------------------------------------
void cDialogPreferences::effectsMuteChanged()
{
	cSettings::getInstance().setSoundMute (disableEffectsCheckBox->isChecked());
}

//------------------------------------------------------------------------------
void cDialogPreferences::voicesMuteChanged()
{
	cSettings::getInstance().setVoiceMute (disableVoicesCheckBox->isChecked());
}
