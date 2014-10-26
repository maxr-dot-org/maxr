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
#include <SDL.h>
#include "tinyxml2.h"
#include "utility/thread/mutex.h"
#include "utility/signal/signal.h"
#include "utility/color.h"

/**
 * cSettings class stores all settings for the game and handles reading
 * and writing them from and to the configuration file.
 * To do this it needs to find out the location of the configuration
 * for the secific platform.
 *
 * For each element of the settings that has to be stored
 * in the configuration file there are getters and setters defined.
 * The setters will automaticaly save the changed values to the configuration.
 * If the configuration does not exist or is uncomplete
 * it will be generated and the needed node will be added.
 *
 * This is a singleton class.
 */
class cSettings
{
public:
	/**
	 * Provides access to the one and only instance of this class.
	 * On the first call this method will initialize the instance.
	 * @return The static instance.
	 */
	static cSettings& getInstance();

	/**
	 * Checks whether the class has been initialized already and successful
	 * (what means the configuration file has been read).
	 * @return True if the settings have been initialized.
	 */
	bool isInitialized() const;

	// Some save methods for settings that are not stored
	// in this class but nevertheless
	// have to be written to the configuration file.
	void saveResolution();
	void saveColorDepth();
	void saveDisplayIndex();
	void saveWindowMode();

	// Game

	bool isDebug() const;
	void setDebug (bool debug, bool save = true);

	bool shouldAutosave() const;
	void setAutosave (bool autosave, bool save = true);

	bool isAnimations() const;
	void setAnimations (bool animations, bool save = true);

	bool isShadows() const;
	void setShadows (bool shadows, bool save = true);

	bool isAlphaEffects() const;
	void setAlphaEffects (bool alphaEffects, bool save = true);

	bool shouldShowDescription() const;
	void setShowDescription (bool showDescription, bool save = true);

	bool isDamageEffects() const;
	void setDamageEffects (bool damageEffects, bool save = true);

	bool isDamageEffectsVehicles() const;
	void setDamageEffectsVehicles (bool damageEffectsVehicle, bool save = true);

	bool isMakeTracks() const;
	void setMakeTracks (bool makeTracks, bool save = true);

	int getScrollSpeed() const;
	void setScrollSpeed (int scrollSpeed, bool save = true);

	const std::string& getNetLogPath() const;
	void setNetLogPath (const char* netLog);

	const std::string& getDataDir() const;
	void setDataDir (const char* dataDir, bool save = true);

	const std::string& getExePath() const;
	void setExePath (const char* exePath);

	const std::string& getLogPath() const;
	void setLogPath (const char* logPath);

	const std::string& getHomeDir() const;
	void setHomeDir (const char* homeDir);

	// Network

	const std::string& getIP() const;
	void setIP (const char* ip, bool save = true);

	unsigned short getPort() const;
	void setPort (unsigned short port, bool save = true);

	const std::string& getPlayerName() const;
	void setPlayerName (const char* playerName, bool save = true);

	const cRgbColor& getPlayerColor () const;
	void setPlayerColor (const cRgbColor& color, bool save = true);

	bool isSoundEnabled() const;
	void setSoundEnabled (bool soundEnabled, bool save = true);

	// Sound

	int getMusicVol() const;
	void setMusicVol (int musicVol, bool save = true);

	int getSoundVol() const;
	void setSoundVol (int soundVol, bool save = true);

	int getVoiceVol() const;
	void setVoiceVol (int voiceVol, bool save = true);

	int getChunkSize() const;
	void setChunkSize (int chunkSize, bool save = true);

	int getFrequency() const;
	void setFrequence (int frequency, bool save = true);

	bool isMusicMute() const;
	void setMusicMute (bool musicMute, bool save = true);

	bool isSoundMute() const;
	void setSoundMute (bool soundMute, bool save = true);

	bool isVoiceMute() const;
	void setVoiceMute (bool voiceMute, bool save = true);

	bool is3DSound () const;
	void set3DSound (bool sound3d, bool save = true);

	// Startup

	bool shouldShowIntro() const;
	void setShowIntro (bool showIntro, bool save = true);

	bool shouldUseFastMode() const;
	void setFastMode (bool fastMode, bool save = true);

	bool shouldDoPrescale() const;
	void setDoPrescale (bool preScale, bool save = true);

	const std::string& getLanguage() const;
	void setLanguage (const char* language, bool save = true);

	const std::string& getVoiceLanguage() const;
	void setVoiceLanguage (const char* language, bool save = true);

	unsigned int getCacheSize() const;
	void setCacheSize (unsigned int cacheSize, bool save = true);

	// Paths

	const std::string& getFontPath() const;
	void setFontPath (const char* fontPath, bool save = true);

	const std::string& getFxPath() const;
	void setFxPath (const char* fxPath, bool save = true);

	const std::string& getGfxPath() const;
	void setGfxPath (const char* gfxPath, bool save = true);

	const std::string& getLangPath() const;
	void setLangPath (const char* langPath, bool save = true);

	const std::string& getMapsPath() const;
	void setMapsPath (const char* mapsPath, bool save = true);

	const std::string& getSavesPath() const;
	void setSavesPath (const char* savesPath, bool save = true);

	const std::string& getSoundsPath() const;
	void setSoundsPath (const char* soundsPath, bool save = true);

	const std::string& getVoicesPath() const;
	void setVoicesPath (const char* voicesPath, bool save = true);

	const std::string& getMusicPath() const;
	void setMusicPath (const char* musicPath, bool save = true);

	const std::string& getVehiclesPath() const;
	void setVehiclesPath (const char* vehiclesPath, bool save = true);

	const std::string& getBuildingsPath() const;
	void setBuildingsPath (const char* buildingsPath, bool save = true);

	const std::string& getMvePath() const;
	void setMvePath (const char* mvePath, bool save = true);

	mutable cSignal<void ()> animationsChanged;
	// TODO: add signals for other settings
private:

	/**
	 * Private constructor for singleton design pattern.
	 */
	cSettings();
	/**
	 * Private copy constructor for singleton design pattern.
	 * Does not need to be implemented!
	 */
	cSettings (cSettings& other);

	/**
	 * True if the object has been initialized already.
	 */
	bool initialized;
	/**
	 * True if initialization is running at the moment.
	 * Used to prevent the class to start multible initializations.
	 */
	bool initializing;

	tinyxml2::XMLDocument configFile;
	cMutex xmlDocMutex;

	/**
	 * The static instance of this object.
	 */
	static cSettings instance;

	// START-Node
	/** enable intro on start */
	bool showIntro;
	/** start in fastmode */
	bool fastMode;
	/** prescale gfx */
	bool preScale;
	/** translation file */
	std::string language;
	/** language code for voice files */
	std::string voiceLanguage;
	/** cache size */
	unsigned int cacheSize;

	// GAME-Node
	/** enable debug */
	bool debug;
	/** enable autosafe */
	bool autosave;
	/** enable animations */
	bool animations;
	/** enable shadows */
	bool shadows;
	/** enable alpha effects */
	bool alphaEffects;
	/** enable describtions (e.g. in buildmenues) */
	bool showDescription;
	/** enable damage effects (smoke'n stuff) */
	bool damageEffects;
	/** enable damage effects for vehicles (smoke'n stuff) */
	bool damageEffectsVehicles;
	/** enable tracks (units leave tracks on the floor) */
	bool makeTracks;
	/** scrollspeed on map */
	int scrollSpeed;

	/** sConfig is where the config is read from - set in setPaths() **/
	std::string configPath;
	/** sExePath is where the exe is located - set in setPaths */
	std::string exePath;
	/** sDataDir is where the data files are stored */
	std::string dataDir;
	/** sLog is where the log goes - set in setPaths() **/
	std::string logPath;
	/** sNetLog is where the netlog goes - set in setPaths() **/
	std::string netLogPath;
	/** sHome is where the user has his $HOME dir - set in setPaths() **/
	std::string homeDir;

	// NET
	/** Last/default ip used for network game */
	std::string ip; // string? why not int array? --beko
	/** Last/default port  used for network game */
	unsigned short port;
	/** Last/default player's name used for network game */
	std::string playerName;
	/** Last color chosen by player */
	cRgbColor playerColor;

	// SOUND
	/** sound enabled */
	bool soundEnabled;
	/** volume music */
	int musicVol;
	/** volume sound effects */
	int soundVol;
	/** volume voices */
	int voiceVol;
	/** chunk size */
	int chunkSize;
	/** frequenzy */
	int frequency;
	/** mute music */
	bool musicMute;
	/** mute sound effects */
	bool soundMute;
	/** mute voices */
	bool voiceMute;
	/** in-game sound effects should respect position*/
	bool sound3d;

	// PATHS
	std::string fontPath;      // Path to the fonts
	std::string fxPath;        // Path to the effects
	std::string gfxPath;       // Path to the graphics
	std::string langPath;      //Path to language files
	std::string mapsPath;      // Path to the maps
	std::string savesPath;     // Path to the saves
	std::string soundsPath;    // Path to the sound-files
	std::string voicesPath;    // Path to the voice-files
	std::string musicPath;     // Path to the music-files
	std::string vehiclesPath;  // Path to the vehicles
	std::string buildingsPath; // Path to the buildings
	std::string mvePath;       // Path to the in-game movies (*.mve)

	/**
	 * Gets the platform dependent user paths for the configuration file
	 * and reads the file.
	 * If the file (or parts of it) does not exist
	 * default values will be used and written to the (may generated) file.
	 */
	void initialize();

	/**
	 * Sets the platform dependend config, log and save paths.
	 */
	void setPaths();

	/**
	 * Platform dependend implementations.
	 * On most plattforms just the executable folder is used.
	 * On linux it tries to verify the path from the configuration file
	 * @param sDataDirFromConf The data location that has been read
	 *        from the configuration file.
	 * @return The really selected data location.
	 */
	std::string searchDataDir (const std::string& sDataDirFromConf = "");

	/**
	 * Creates a new configuration file and adds the root node to it.
	 * @return True on success. Else false.
	 */
	bool createConfigFile();



	/**
	 * Template function for saving a setting.
	 * @param path See #getXmlNode() for more information on
	 *        how to use this parameter.
	 * @param value The value to set as attribute to the setting node.
	 * @param valueName The name of the attribute to set to the setting node.
	 */
	template<typename T>
	void saveSetting (const std::string& path, T value, const char* valueName);

	// Overloadings for the saveSetting template function.
	// Each type has to call the template saveSetting() method and pass the
	// corresponding attribute name to it.
	void saveSetting (const std::string& path, const char* value);
	void saveSetting (const std::string& path, int value);
	void saveSetting (const std::string& path, unsigned int value);
	void saveSetting (const std::string& path, bool value);
};

#endif // SETTINGS_H
