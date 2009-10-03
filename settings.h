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
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

// SettingsData - Class containing all gamesettings ///////////////////////
/**
 * cSettings class. Stores gamesettings :-)
 *
 * @author Bernd "beko" Kosmahl
 */
class cSettings
{
public:
	/**sConfig is where the config is read from - set in setPaths() **/
	std::string sConfig;
	/**sExePath is where the exe is located - set in setPaths*/
	std::string sExePath;
	/**sDataDir is where the data files are stored*/
	std::string sDataDir;
	/**sLog is where the log goes - set in setPaths() **/
	std::string sLog;
	/**sNetLog is where the netlog goes - set in setPaths() **/
	std::string sNetLog;
	/**sHome is where the user has his $HOME dir - set in setPaths() **/
	std::string sHome;

	//START-Node
	/**screen width in pixels */
	int iScreenW;
	/**screen height in pixels */
	int iScreenH;
	/**colour depth - e.g. 32*/
	int iColourDepth;
	/**enable intro on start*/
	bool bIntro;
	/**start in windowmode*/
	bool bWindowMode;
	/**start in fastmode */
	bool bFastMode;
	/**prescale gfx */
	bool bPreScale;
	/**translation file*/
	std::string sLanguage;
	/**cache size*/
	unsigned int iCacheSize;

	//GAME-Node
	/** enable debug*/
	bool bDebug;
	/**enable autosafe */
	bool bAutoSave;
	/**enable animations */
	bool bAnimations;
	/**enable shadows */
	bool bShadows;
	/**enable alpha effects */
	bool bAlphaEffects;
	/**enable describtions (e.g. in buildmenues) */
	bool bShowDescription;
	/**enable damage effects (smoke'n stuff)*/
	bool bDamageEffects;
	/**enable damage effects for vehicles (smoke'n stuff)*/
	bool bDamageEffectsVehicles;
	/**enable tracks (units leave tracks on the floor) */
	bool bMakeTracks;
	/**scrollspeed on map */
	int iScrollSpeed;

	//NET
	/**Last/default ip used for network game */
	std::string sIP; //string? why not int array? --beko
	/**Last/default port  used for network game */
	int iPort;
	/**Last/default player's name used for network game */
	std::string sPlayerName;
	/**Last color choosen by player*/
	int iColor;

	//SOUND
	/**sound enabled*/
	bool bSoundEnabled;
	/**volume music */
	int MusicVol;
	/**volume sound effects */
	int SoundVol;
	/**volume voices */
	int VoiceVol;
	/**chunk size */
	int iChunkSize;
	/**frequenzy */
	int iFrequency;
	/**mute music */
	bool MusicMute;
	/**mute sound effects */
	bool SoundMute;
	/**mute voices */
	bool VoiceMute;	// Muteeigenschaften der Sounds

	//PATHS
	std::string sFontPath;			// Path to the fonts
	std::string sFxPath;				// Path to the effects
	std::string sGfxPath;			// Path to the graphics
	std::string sLangPath;			//Path to language files
	std::string sMapsPath;			// Path to the maps
	std::string sSavesPath;			// Path to the saves
	std::string sSoundsPath;			// Path to the sound-files
	std::string sVoicesPath;			// Path to the voice-files
	std::string sMusicPath;			// Path to the music-files
	std::string sVehiclesPath;			// Path to the vehicles
	std::string sBuildingsPath;			// Path to the buildings
	std::string sMVEPath;			// Path to the in-game movies (*.mve)

	unsigned int Checksum;		// Die Checksumme ¬∏ber alle Eigenschaften - NOT IN XML-file (yet?)!
};

extern cSettings SettingsData;

#endif // SETTINGS_H
