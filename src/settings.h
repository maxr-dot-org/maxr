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

#include <mutex>
#include <string>

struct sVideoSettings
{
	std::optional<cPosition> resolution;
	int colourDepth = 32;
	int displayIndex = 0;
	bool windowMode = true;
};

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

	void saveInFile() /*const*/;

	/**
	 * Checks whether the class has been initialized already and successful
	 * (what means the configuration file has been read).
	 * @return True if the settings have been initialized.
	 */
	bool isInitialized() const;

	// Game

	bool isDebug() const;
	void setDebug (bool debug);

	bool shouldAutosave() const;
	void setAutosave (bool autosave);

	bool isAnimations() const;
	void setAnimations (bool animations);

	bool isShadows() const;
	void setShadows (bool shadows);

	bool isAlphaEffects() const;
	void setAlphaEffects (bool alphaEffects);

	bool shouldShowDescription() const;
	void setShowDescription (bool showDescription);

	bool isDamageEffects() const;
	void setDamageEffects (bool damageEffects);

	bool isDamageEffectsVehicles() const;
	void setDamageEffectsVehicles (bool damageEffectsVehicle);

	bool isMakeTracks() const;
	void setMakeTracks (bool makeTracks);

	int getScrollSpeed() const;
	void setScrollSpeed (int scrollSpeed);

	const std::string& getNetLogPath() const;
	void setNetLogPath (const char* netLog);

	const std::string& getDataDir() const;
	const std::string& getLogPath() const;
	const std::string& getHomeDir() const;

	const sNetworkAddress& getNetworkAddress() const { return networkAddress; }
	void setNetworkAddress (const sNetworkAddress&);

	const sPlayerSettings& getPlayerSettings() const { return playerSettings; }
	void setPlayerSettings (const sPlayerSettings&);

	const sVideoSettings& getVideoSettings() const { return videoSettings; }
	sVideoSettings& getVideoSettings() { return videoSettings; }

	// Sound

	bool isSoundEnabled() const;
	void setSoundEnabled (bool soundEnabled);

	int getMusicVol() const;
	void setMusicVol (int musicVol);

	int getSoundVol() const;
	void setSoundVol (int soundVol);

	int getVoiceVol() const;
	void setVoiceVol (int voiceVol);

	int getChunkSize() const;
	void setChunkSize (int chunkSize);

	int getFrequency() const;
	void setFrequence (int frequency);

	bool isMusicMute() const;
	void setMusicMute (bool musicMute);

	bool isSoundMute() const;
	void setSoundMute (bool soundMute);

	bool isVoiceMute() const;
	void setVoiceMute (bool voiceMute);

	bool is3DSound() const;
	void set3DSound (bool sound3d);

	// Startup

	bool shouldShowIntro() const;
	void setShowIntro (bool showIntro);

	bool shouldUseFastMode() const;
	bool shouldDoPrescale() const;

	const std::string& getLanguage() const;
	void setLanguage (const char* language);

	const std::string& getVoiceLanguage() const;

	unsigned int getCacheSize() const;
	void setCacheSize (unsigned int cacheSize);

	// Paths
	std::string getFontPath() const;
	std::string getFxPath() const;
	std::string getGfxPath() const;
	std::string getLangPath() const;
	std::string getMapsPath() const;
	std::string getSavesPath() const;
	std::string getSoundsPath() const;
	std::string getVoicesPath() const;
	std::string getMusicPath() const;
	std::string getVehiclesPath() const;
	std::string getBuildingsPath() const;
	std::string getMvePath() const;

	mutable cSignal<void()> animationsChanged;
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

	void setDataDir (const char* dataDir);

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
		std::string fontPath = "fonts";          // Path to the fonts
		std::string fxPath = "fx";               // Path to the effects
		std::string gfxPath = "gfx";             // Path to the graphics
		std::string langPath = "languages";      // Path to language files
		std::string mapsPath = "maps";           // Path to the maps
		std::string savesPath = "saves";         // Path to the saves
		std::string soundsPath = "sounds";       // Path to the sound-files
		std::string voicesPath = "voices";       // Path to the voice-files
		std::string musicPath = "music";         // Path to the music-files
		std::string vehiclesPath = "vehicles";   // Path to the vehicles
		std::string buildingsPath = "buildings"; // Path to the buildings
		std::string mvePath = "mve";             // Path to the in-game movies (*.mve)
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
	sVideoSettings videoSettings;
};

#endif // SETTINGS_H
