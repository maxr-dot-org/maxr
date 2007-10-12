//////////////////////////////////////////////////////////////////////////////
// M.A.X. - defines.h
//////////////////////////////////////////////////////////////////////////////
#ifndef definesH
#define definesH

#ifdef __main__
#define EX
#define ZERO =0
#define ONE =1
#else
#define EX extern
#define ZERO
#define ONE 
#endif

#define SHOW_SCREEN SDL_BlitSurface(buffer,NULL,screen,NULL);if(SettingsData.bWindowMode)SDL_UpdateRect(screen,0,0,0,0);

#endif


#ifdef WIN32
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "\\"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\r\n"
	#endif
#else
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "//"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\n"
	#endif
#endif

// We have to take care of these manually !
#define MAXVERSION      "M.A.X. Reloaded 0.52.0 SVN"
#define MAX_VERSION     "0.52.0"
#define MAX_BUILD_DATE  "2007-10-05 22:30:00"

//#define MAXVERSION "M.A.X. Reloaded 0.52.0 SVN BUILD 200710052230"
//#define MAXVERSION "M.A.X. Reloaded 0.52.1"
