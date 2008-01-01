/*==============================================================================*
|																				|
|									defines.h									|
*===============================================================================*/

#ifndef DefinesH
#define DefinesH

#ifdef __main__
#define EX
#define ZERO =0
#define ONE =1
#else
#define EX extern
#define ZERO
#define ONE 
#endif

#endif // DefinesH

/* spezial meanings of colors in the palette:

Playercolors:  32,33,34,35,36,37,38,39

Effektcolors:  11,14,21,22,	...

most likely, all colors < 64 have a special meaning

*/