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

#include "game/data/player/playersettings.h"
#include "game/networkaddress.h"
#include "utility/signal/signal.h"

#include <3rd/tinyxml2/tinyxml2.h>

#include <mutex>
#include <string>

/**
 * cSettings class stores all settings for the game and handles reading
 * and writing them from and to the configuration file.
 * To do this it needs to find out the location of the configuration
 * for the specific platform.
 *
 * For each element of the settings that has to be stored
 * in the configuration file there are getters and setters defined.
 * The setters will automatically save the changed values to the configuration.
 * If the configuration does not exist or is incomplete
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
	void setDebug (bool debug, bool save);

	bool shouldAutosave() const;
	void setAutosave (bool autosave, bool save);

	bool isAnimations() const;
	void setAnimations (bool animations, bool save);

	bool isShadows() const;
	void setShadows (bool shadows, bool save);

	bool isAlphaEffects() const;
	void setAlphaEffects (bool alphaEffects, bool save);

	bool shouldShowDescription() const;
	void setShowDescription (bool showDescription, bool save);

	bool isDamageEffects() const;
	void setDamageEffects (bool damageEffects, bool save);

	bool isDamageEffectsVehicles() const;
	void setDamageEffectsVehicles (bool damageEffectsVehicle, bool save);

	bool isMakeTracks() const;
	void setMakeTracks (bool makeTracks, bool save);

	int getScrollSpeed() const;
	void setScrollSpeed (int scrollSpeed, bool save);

	const std::string& getNetLogPath() const;
	void setNetLogPath (const char* netLog);

	const std::string& getDataDir() const;
	const std::string& getLogPath() const;
	const std::string& getHomeDir() const;

	const sNetworkAddress& getNetworkAddress() const { return networkAddress; }
	void setNetworkAddress (const sNetworkAddress&, bool save);

	const sPlayerSettings& getPlayerSettings() const { return playerSettings; }
	void setPlayerSettings (const sPlayerSettings&, bool save);

	// Sound

	bool isSoundEnabled() const;
	void setSoundEnabled (bool soundEnabled, bool save);

	int getMusicVol() const;
	void setMusicVol (int musicVol, bool save);

	int getSoundVol() const;
	void setSoundVol (int soundVol, bool save);

	int getVoiceVol() const;
	void setVoiceVol (int voiceVol, bool save);

	int getChunkSize() const;
	void setChunkSize (int chunkSize, bool save);

	int getFrequency() const;
	void setFrequence (int frequency, bool save);

	bool isMusicMute() const;
	void setMusicMute (bool musicMute, bool save);

	bool isSoundMute() const;
	void setSoundMute (bool soundMute, bool save);

	bool isVoiceMute() const;
	void setVoiceMute (bool voiceMute, bool save);

	bool is3DSound() const;
	void set3DSound (bool sound3d, bool save);

	// Startup

	bool shouldShowIntro() const;
	void setShowIntro (bool showIntro, bool save);

	bool shouldUseFastMode() const;
	bool shouldDoPrescale() const;

	const std::string& getLanguage() const;
	void setLanguage (const char* language, bool save);

	const std::string& getVoiceLanguage() const;

	unsigned int getCacheSize() const;
	void setCacheSize (unsigned int cacheSize, bool save);

	// Paths
	const std::string& getFontPath() const;
	const std::string& getFxPath() const;
	const std::string& getGfxPath() const;
	const std::string& getLangPath() const;
	const std::string& getMapsPath() const;
	const std::string& getSavesPath() const;
	const std::string& getSoundsPath() const;
	const std::string& getVoicesPath() const;
	const std::string& getMusicPath() const;
	const std::string& getVehiclesPath() const;
	const std::string& getBuildingsPath() const;
	const std::string& getMvePath() const;

	mutable cSignal<void()> animationsChanged;
	// TODO: add signals for other settings
private:

	/**
	 * Private constructor for singleton design pattern.
	 */
	cSettings() = default;
	/**
	 * Private copy constructor for singleton design pattern.
	 * Does not need to be implemented!
	 */
	cSettings (const cSettings&) = delete;

	/**
	 * Gets the platform dependent user paths for the configuration file
	 * and reads the file.
	 * If the file (or parts of it) does not exist
	 * default values will be used and written to the (may generated) file.
	 */
	void initialize();

	/**
	 * Sets the platform dependent config, log and save paths.
	 */
	void setPaths();

	/**
	 * Creates a new configuration file and adds the root node to it.
	 * @return True on success. Else false.
	 */
	bool createConfigFile();

	void setDataDir (const char* dataDir, bool save);

	/**
	 * Template function for saving a setting.
	 * @param path See #getXmlNode() for more information on
	 *        how to use this parameter.
	 * @param value The value to set as attribute to the setting node.
	 * @param valueName The name of the attribute to set to the setting node.
	 */
	template <typename T>
	void saveSetting (const std::string& path, T value, const char* valueName);

	// Overloads for the saveSetting template function.
	// Each type has to call the template saveSetting() method and pass the
	// corresponding attribute name to it.
	void saveSetting (const std::string& path, const char* value);
	void saveSetting (const std::string& path, int value);
	void saveSetting (const std::string& path, unsigned int value);
	void saveSetting (const std::string& path, bool value);
private:
	struct sStartSettings
	{
		/** enable intro on start */
		bool showIntro = true;
		/** start in fastmode */
		bool fastMode = false;
		/** prescale gfx */
		bool preScale = false;
		/** translation file */
		std::string language = "en";
		/** language code for voice files */
		std::string voiceLanguage;
		/** cache size */
		unsigned int cacheSize = 400;
	};

	struct sSoundSettings
	{
		/** sound enabled */
		bool soundEnabled = true;
		/** volume music */
		int musicVol = 128;
		/** volume sound effects */
		int soundVol = 128;
		/** volume voices */
		int voiceVol = 128;
		/** chunk size */
		int chunkSize = 2048;
		/** frequency */
		int frequency = 44100;
		/** mute music */
		bool musicMute = false;
		/** mute sound effects */
		bool soundMute = false;
		/** mute voices */
		bool voiceMute = false;
		/** in-game sound effects should respect position*/
		bool sound3d = true;
	};

	struct sPathSettings
	{
		std::string fontPath;      // Path to the fonts
		std::string fxPath;        // Path to the effects
		std::string gfxPath;       // Path to the graphics
		std::string langPath;      // Path to language files
		std::string mapsPath;      // Path to the maps
		std::string savesPath;     // Path to the saves
		std::string soundsPath;    // Path to the sound-files
		std::string voicesPath;    // Path to the voice-files
		std::string musicPath;     // Path to the music-files
		std::string vehiclesPath;  // Path to the vehicles
		std::string buildingsPath; // Path to the buildings
		std::string mvePath;       // Path to the in-game movies (*.mve)
	};

	struct sInGameSettings
	{
		/** enable debug */
		bool debug = true;
		/** enable autosave */
		bool autosave = true;
		/** enable animations */
		bool animations = true;
		/** enable shadows */
		bool shadows = true;
		/** enable alpha effects */
		bool alphaEffects = true;
		/** enable descriptions (e.g. in build menus) */
		bool showDescription = true;
		/** enable damage effects (smoke'n stuff) */
		bool damageEffects = true;
		/** enable damage effects for vehicles (smoke'n stuff) */
		bool damageEffectsVehicles = true;
		/** enable tracks (units leave tracks on the floor) */
		bool makeTracks = true;
		/** scrollspeed on map */
		int scrollSpeed = 32;
	};

private:
	/**
	 * The static instance of this object.
	 */
	static cSettings instance;

	/**
	 * True if the object has been initialized already.
	 */
	bool initialized = false;
	/**
	 * True if initialization is running at the moment.
	 * Used to prevent the class to start multiple initializations.
	 */
	bool initializing = false;

	tinyxml2::XMLDocument configFile;
	std::recursive_mutex xmlDocMutex;

	sStartSettings startSettings;

	/** sConfig is where the config is read from - set in setPaths() **/
	std::string configPath;
	/** sDataDir is where the data files are stored */
	std::string dataDir;
	/** sLog is where the log goes - set in setPaths() **/
	std::string logPath;
	/** sNetLog is where the netlog goes - set in setPaths() **/
	std::string netLogPath;
	/** sHome is where the user has his $HOME dir - set in setPaths() **/
	std::string homeDir;

	sNetworkAddress networkAddress;
	sPlayerSettings playerSettings;
	sSoundSettings soundSettings;
	sPathSettings pathSettings;
	sInGameSettings gameSettings;
};

#endif // SETTINGS_H
