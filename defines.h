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

#define SHOW_SCREEN SDL_BlitSurface(buffer,NULL,screen,NULL);if(GameSettingsData.WindowMode)SDL_UpdateRect(screen,0,0,0,0);

#endif


#ifdef WIN32
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "\\"
	#endif
#else
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "//"
	#endif
#endif
