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

#ifndef ui_graphical_menu_dialogs_dialogpreferencesH
#define ui_graphical_menu_dialogs_dialogpreferencesH

#include "ui/widgets/window.h"
#include "utility/signal/signalconnectionmanager.h"

class cCheckBox;
class cColorSelector;
class cComboBox;
class cLabel;
class cLineEdit;
class cPushButton;
class cSlider;

class cDialogPreferences : public cWindow
{
public:
	cDialogPreferences();
	~cDialogPreferences();

	void retranslate() override;

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* titleLabel = nullptr;
	cLabel* volumeLabel = nullptr;
	cLabel* musicLabel = nullptr;
	cLabel* effectsLabel = nullptr;
	cLabel* voicesLabel = nullptr;
	cLabel* playerNameLabel = nullptr;
	cLabel* colorLabel = nullptr;
	cLabel* scrollSpeedLabel = nullptr;
	cLabel* languageLabel = nullptr;
	cLabel* resolutionLabel = nullptr;

	cPushButton* doneButton = nullptr;
	cPushButton* cancelButton = nullptr;

	cSlider* musicVolumeSlider = nullptr;
	cSlider* effectsVolumeSlider = nullptr;
	cSlider* voicesVolumeSlider = nullptr;

	cCheckBox* disableMusicCheckBox = nullptr;
	cCheckBox* disableEffectsCheckBox = nullptr;
	cCheckBox* disableVoicesCheckBox = nullptr;

	cCheckBox* effects3DCheckBox = nullptr;

	cSlider* scrollSpeedSlider = nullptr;

	cLineEdit* nameEdit = nullptr;
	cColorSelector* colorSelector = nullptr;

	cCheckBox* animationCheckBox = nullptr;
	cCheckBox* shadowsCheckBox = nullptr;
	cCheckBox* aplhaCheckBox = nullptr;
	cCheckBox* demageBuildingsCheckBox = nullptr;
	cCheckBox* demageVehiclesCheckBox = nullptr;
	cCheckBox* tracksCheckBox = nullptr;

	cCheckBox* autosaveCheckBox = nullptr;
	cCheckBox* introCheckBox = nullptr;
	cCheckBox* windowCheckBox = nullptr;

	cComboBox* languagesComboBox = nullptr;
	cComboBox* resolutionsComboBox = nullptr;

	std::string storedLanguage;

	int storedMusicVolume;
	int storedEffectsVolume;
	int storedVoicesVolume;

	bool storedMusicMute;
	bool storedEffectsMute;
	bool storedVoicesMute;

	void loadValues();
	void saveValues();

	void storePreviewValues();
	void restorePreviewValues();

	void doneClicked();
	void cancelClicked();

	void musicVolumeChanged();
	void effectsVolumeChanged();
	void voicesVolumeChanged();

	void musicMuteChanged();
	void effectsMuteChanged();
	void voicesMuteChanged();
};

#endif // ui_graphical_menu_dialogs_dialogpreferencesH
