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

#ifndef gui_menu_dialogs_dialogpreferencesH
#define gui_menu_dialogs_dialogpreferencesH

#include "../../window.h"
#include "../../../utility/signal/signalconnectionmanager.h"

class cSlider;
class cLineEdit;
class cCheckBox;

class cComboBox;

class cDialogPreferences : public cWindow
{
public:
	cDialogPreferences ();
	~cDialogPreferences ();

private:
	cSignalConnectionManager signalConnectionManager;

	cSlider* musicVolumeSlider;
	cSlider* effectsVolumeSlider;
	cSlider* voicesVolumeSlider;

	cCheckBox* disableMusicCheckBox;
	cCheckBox* disableEffectsCheckBox;
	cCheckBox* disableVoicesCheckBox;

	cSlider* scrollSpeedSlider;

	cLineEdit* nameEdit;

	cCheckBox* animationCheckBox;
	cCheckBox* shadowsCheckBox;
	cCheckBox* aplhaCheckBox;
	cCheckBox* demageBuildingsCheckBox;
	cCheckBox* demageVehiclesCheckBox;
	cCheckBox* tracksCheckBox;

	cCheckBox* autosaveCheckBox;
	cCheckBox* introCheckBox;
	cCheckBox* windowCheckBox;

	cComboBox* languagesComboBox;
	cComboBox* resolutionsComboBox;

	int storedMusicVolume;
	int storedEffectsVolume;
	int storedVoicesVolume;

	bool storedMusicMute;
	bool storedEffectsMute;
	bool storedVoicesMute;

	void loadValues ();
	void saveValues ();

	void storePreviewValues ();
	void restorePreviewValues ();

	void doneClicked ();
	void cancelClicked ();

	void musicVolumeChanged ();
	void effectsVolumeChanged ();
	void voicesVolumeChanged ();

	void musicMuteChanged ();
	void effectsMuteChanged ();
	void voicesMuteChanged ();
};

#endif // gui_menu_dialogs_dialogpreferencesH
