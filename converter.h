/*==============================================================================*
|																				|
|									converter.h									|
*===============================================================================*/
#ifndef ConverterH
#define ConverterH
#include <sdl.h>
#include "defines.h"
#include "main.h"

struct sPixel
{
	unsigned char Blue, Green, Red;
};

struct cImageData
{
	short sWidth;
	short sHeight;
	short sHotX;
	short sHotY;
	short sUHotX;
	short sUHotY;

	cImageData *Images;

	unsigned char *data;
	unsigned char *alpha;

	SDL_Surface* surface;
};

class cImage
{
	bool bDecoded;
	short sWidth;
	short sHeight;
	cImageData *Images;

	int iImageCount;

	short sHotX;
	short sHotY;

	bool decodeSimpleImage();
	bool decodeMultiShadow();
	bool decodeMultiImage();
	bool decodeBigImage();

public:
	
	cImage::cImage();
	cImage::~cImage();

	char name[9];
	long lPos;
	long lLenght;

	sPixel *palette;

	void decodeFile();
	void resampleFile();
	void saveFile();
	SDL_Surface* getSurface(int imageNr = 0);
};

SDL_Surface* getImage(string file_name, int imageNr = 0);
void removePlayerColor( SDL_Surface *surface);
int saveAllFiles();
int copyFileFromRes ( string src, string dst, int number = 0 );
int copyFileFromRes_rpc(string src, string dst, int number = 0 );
int copyImageFromFLC(string fileName, string dst);

#endif // ConvertH
