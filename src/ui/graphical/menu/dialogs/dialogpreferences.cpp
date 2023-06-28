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

#include "dialogpreferences.h"

#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/pcx.h"
#include "settings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/colorselector.h"
#include "ui/graphical/menu/widgets/combobox.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"
#include "ui/graphical/menu/widgets/special/textlistviewitem.h"
#include "ui/uidefines.h"
#include "ui/widgets/label.h"
#include "ui/widgets/lineedit.h"
#include "ui/widgets/validators/validatorint.h"
#include "utility/language.h"
#include "utility/string/iequals.h"

#include <SDL_mixer.h>
#include <functional>

//------------------------------------------------------------------------------
cDialogPreferences::cDialogPreferences() :
	cWindow (LoadPCX (GFXOD_DIALOG5), eWindowBackgrounds::Alpha)
{
	// blit black titlebar behind textfield for playername
	SDL_Rect src = {108, 12, 186, 18};
	SDL_Rect dest = {140, 154, 0, 0};
	SDL_BlitSurface (getSurface(), &src, getSurface(), &dest);

	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 15), getPosition() + cPosition (getArea().getMaxCorner().x(), 25)), lngPack.i18n ("Settings~Preferences"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	volumeLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56), getPosition() + cPosition (135, 66)), lngPack.i18n ("Settings~Volume"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);

	musicLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20), getPosition() + cPosition (135, 66 + 20)), lngPack.i18n ("Settings~Music"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	musicVolumeSlider = emplaceChild<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20), getPosition() + cPosition (240, 70 + 20)), 0, 128, eOrientationType::Horizontal);
	signalConnectionManager.connect (musicVolumeSlider->valueChanged, [this]() { musicVolumeChanged(); });
	disableMusicCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (245, 73), lngPack.i18n ("Settings~Disable"));
	signalConnectionManager.connect (disableMusicCheckBox->toggled, [this]() { musicMuteChanged(); });

	effectsLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20 * 2), getPosition() + cPosition (135, 66 + 20 * 2)), lngPack.i18n ("Settings~Effects"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	effectsVolumeSlider = emplaceChild<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20 * 2), getPosition() + cPosition (240, 70 + 20 * 2)), 0, 128, eOrientationType::Horizontal);
	signalConnectionManager.connect (effectsVolumeSlider->valueChanged, [this]() { effectsVolumeChanged(); });
	disableEffectsCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (245, 73 + 20), lngPack.i18n ("Settings~Disable"));
	signalConnectionManager.connect (disableEffectsCheckBox->toggled, [this]() { effectsMuteChanged(); });
	effects3DCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (340, 73 + 20), "3D");

	voicesLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 56 + 20 * 3), getPosition() + cPosition (135, 66 + 20 * 3)), lngPack.i18n ("Settings~Voices"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	voicesVolumeSlider = emplaceChild<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 53 + 20 * 3), getPosition() + cPosition (240, 70 + 20 * 3)), 0, 128, eOrientationType::Horizontal);
	signalConnectionManager.connect (voicesVolumeSlider->valueChanged, [this]() { voicesVolumeChanged(); });
	disableVoicesCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (245, 73 + 20 * 2), lngPack.i18n ("Settings~Disable"));
	signalConnectionManager.connect (disableVoicesCheckBox->toggled, [this]() { voicesMuteChanged(); });

	playerNameLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 158), getPosition() + cPosition (138, 168)), lngPack.i18n ("Title~Player_Name"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	nameEdit = emplaceChild<cLineEdit> (cBox<cPosition> (getPosition() + cPosition (140, 154), getPosition() + cPosition (140 + 185, 154 + 18)), eLineEditFrameType::Box);

	colorLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 175), getPosition() + cPosition (135, 185)), lngPack.i18n ("Title~Color"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	colorSelector = emplaceChild<cColorSelector> (getPosition() + cPosition (160, 172), cRgbColor::black());

	scrollSpeedLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (25, 257), getPosition() + cPosition (135, 267)), lngPack.i18n ("Settings~Scrollspeed"), eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	scrollSpeedSlider = emplaceChild<cSlider> (cBox<cPosition> (getPosition() + cPosition (140, 254), getPosition() + cPosition (240, 271)), 0, 128, eOrientationType::Horizontal);

	animationCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 193), lngPack.i18n ("Settings~Animation"));
	shadowsCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 193 + 20), lngPack.i18n ("Settings~Shadows"));
	aplhaCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 193 + 20 * 2), lngPack.i18n ("Settings~Alphaeffects"));
	demageBuildingsCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (210, 193), lngPack.i18n ("Settings~ShowDamage"));
	demageVehiclesCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (210, 193 + 20), lngPack.i18n ("Settings~ShowDamageVehicle"));
	tracksCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (210, 193 + 20 * 2), lngPack.i18n ("Settings~Tracks"));

	autosaveCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 294), lngPack.i18n ("Settings~Autosave"));
	introCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 294 + 20), lngPack.i18n ("Settings~Intro"));
	windowCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (25, 294 + 20 * 2), lngPack.i18n ("Settings~Window"));

	doneButton = emplaceChild<cPushButton> (getPosition() + cPosition (208, 383), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [this]() { doneClicked(); });

	cancelButton = emplaceChild<cPushButton> (getPosition() + cPosition (118, 383), ePushButtonType::Angular, lngPack.i18n ("Others~Cancel"), eUnicodeFontType::LatinNormal);
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [this]() { cancelClicked(); });

	languageLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (140, 298 + 20), getPosition() + cPosition (140 + 95, 298 + 20 + 10)), lngPack.i18n ("Settings~Language") + lngPack.i18n ("Punctuation~Colon"), eUnicodeFontType::LatinNormal, eAlignmentType::Right);
	languagesComboBox = emplaceChild<cComboBox> (cBox<cPosition> (getPosition() + cPosition (240, 294 + 20), getPosition() + cPosition (240 + 100, 294 + 20 + 17)));
	signalConnectionManager.connect (languagesComboBox->onItemChanged, [this] (const std::string& selectedLanguage) {
		auto& settings = cSettings::getInstance();
		if (iequals (selectedLanguage, settings.getLanguage()))
		{
			return;
		}
		settings.setLanguage (selectedLanguage.c_str());
		lngPack.setCurrentLanguage (settings.getLanguage());

		auto application = getActiveApplication();
		if (application)
		{
			application->retranslate();
		}
	});

	resolutionLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (140, 298), getPosition() + cPosition (140 + 95, 298 + 10)), lngPack.i18n ("Settings~Resolution") + lngPack.i18n ("Punctuation~Colon"), eUnicodeFontType::LatinNormal, eAlignmentType::Right);
	resolutionsComboBox = emplaceChild<cComboBox> (cBox<cPosition> (getPosition() + cPosition (240, 294), getPosition() + cPosition (240 + 100, 294 + 17)));

	loadValues();
}

//------------------------------------------------------------------------------
cDialogPreferences::~cDialogPreferences()
{}

//------------------------------------------------------------------------------
void cDialogPreferences::retranslate()
{
	titleLabel->setText (lngPack.i18n ("Settings~Preferences"));
	volumeLabel->setText (lngPack.i18n ("Settings~Volume"));
	musicLabel->setText (lngPack.i18n ("Settings~Music"));
	effectsLabel->setText (lngPack.i18n ("Settings~Effects"));
	voicesLabel->setText (lngPack.i18n ("Settings~Voices"));
	playerNameLabel->setText (lngPack.i18n ("Title~Player_Name"));
	colorLabel->setText (lngPack.i18n ("Title~Color"));
	scrollSpeedLabel->setText (lngPack.i18n ("Settings~Scrollspeed"));
	languageLabel->setText (lngPack.i18n ("Settings~Language") + lngPack.i18n ("Punctuation~Colon"));
	resolutionLabel->setText (lngPack.i18n ("Settings~Resolution") + lngPack.i18n ("Punctuation~Colon"));

	doneButton->setText (lngPack.i18n ("Others~Done"));
	cancelButton->setText (lngPack.i18n ("Others~Cancel"));

	disableMusicCheckBox->setText (lngPack.i18n ("Settings~Disable"));
	disableEffectsCheckBox->setText (lngPack.i18n ("Settings~Disable"));
	effects3DCheckBox->setText ("3D"); // TODO: Missing translation
	disableVoicesCheckBox->setText (lngPack.i18n ("Settings~Disable"));
	animationCheckBox->setText (lngPack.i18n ("Settings~Animation"));
	shadowsCheckBox->setText (lngPack.i18n ("Settings~Shadows"));
	aplhaCheckBox->setText (lngPack.i18n ("Settings~Alphaeffects"));
	demageBuildingsCheckBox->setText (lngPack.i18n ("Settings~ShowDamage"));
	demageVehiclesCheckBox->setText (lngPack.i18n ("Settings~ShowDamageVehicle"));
	tracksCheckBox->setText (lngPack.i18n ("Settings~Tracks"));
	autosaveCheckBox->setText (lngPack.i18n ("Settings~Autosave"));
	introCheckBox->setText (lngPack.i18n ("Settings~Intro"));
	windowCheckBox->setText (lngPack.i18n ("Settings~Window"));
}

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
	for (const auto& [w, h] : resolutions)
	{
		resolutionsComboBox->addItem (std::to_string (w) + "x" + std::to_string (h));
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

	bool needRestart = false;
	if (settings.getVideoSettings().windowMode != windowCheckBox->isChecked())
	{
		settings.getVideoSettings().windowMode = windowCheckBox->isChecked();
		needRestart = true;
	}

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
			settings.getVideoSettings().resolution = cPosition (newResolutionX, newResolutionY);

			if (Video.getResolutionX() != oldScreenX || Video.getResolutionY() != oldScreenY)
			{
				needRestart = true;
			}
		}
	}
	else
	{
		// TODO: handle invalid resolution?!

		// lngPack.i18n ("Comp~ResolutionWarning")
		// added info to old langpack if needed, else it can be removed from lang-files - nonsinn
	}
	if (needRestart)
	{
		auto application = getActiveApplication();
		if (application)
		{
			application->show (std::make_shared<cDialogOk> (lngPack.i18n ("Comp~RestartRequired")));
		}
	}
	applySettings (Video, settings.getVideoSettings());
	settings.saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::storePreviewValues()
{
	storedLanguage = cSettings::getInstance().getLanguage();

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
	if (storedLanguage != cSettings::getInstance().getLanguage())
	{
		cSettings::getInstance().setLanguage (storedLanguage.c_str());
		lngPack.setCurrentLanguage (storedLanguage);
		auto application = getActiveApplication();
		if (application)
		{
			application->retranslate();
		}
	}

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
	cSettings::getInstance().saveInFile();
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
	cSettings::getInstance().saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::effectsVolumeChanged()
{
	cSettings::getInstance().setSoundVol (effectsVolumeSlider->getValue());
	if (cSettings::getInstance().isSoundEnabled()) cSoundDevice::getInstance().setSoundEffectVolume (cSettings::getInstance().getSoundVol());
	cSettings::getInstance().saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::voicesVolumeChanged()
{
	cSettings::getInstance().setVoiceVol (voicesVolumeSlider->getValue());
	if (cSettings::getInstance().isSoundEnabled()) cSoundDevice::getInstance().setVoiceVolume (cSettings::getInstance().getVoiceVol());
	cSettings::getInstance().saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::musicMuteChanged()
{
	bool wasMute = cSettings::getInstance().isMusicMute();
	cSettings::getInstance().setMusicMute (disableMusicCheckBox->isChecked());
	if (cSettings::getInstance().isMusicMute()) cSoundDevice::getInstance().stopMusic();
	if (!cSettings::getInstance().isMusicMute() && wasMute) cSoundDevice::getInstance().startRandomMusic();
	cSettings::getInstance().saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::effectsMuteChanged()
{
	cSettings::getInstance().setSoundMute (disableEffectsCheckBox->isChecked());
	cSettings::getInstance().saveInFile();
}

//------------------------------------------------------------------------------
void cDialogPreferences::voicesMuteChanged()
{
	cSettings::getInstance().setVoiceMute (disableVoicesCheckBox->isChecked());
	cSettings::getInstance().saveInFile();
}
