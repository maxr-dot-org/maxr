//////////////////////////////////////////////////////////////////////////////
// M.A.X. - fonts.cpp
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include "fonts.h"
#include "main.h"

// Funktionen der Font-Klasse ////////////////////////////////////////////////
cFonts::cFonts ( void )
{
	int i,last,anz;
	// Die Zeichenbreiten auslesen:
	chars=NULL;
	anz=0;
	last=0;
	for ( i=0;i<FontsData.font->w;i++ )
	{
		if ( ( ( unsigned int* ) FontsData.font->pixels ) [ ( FontsData.font->h-1 ) *FontsData.font->w+i]==0xFF00FF )
		{
			anz++;
			chars= ( SDL_Rect* ) realloc ( chars,sizeof ( SDL_Rect ) *anz );
			chars[anz-1].x=last;
			chars[anz-1].y=0;
			chars[anz-1].w=i-last+1;
			chars[anz-1].h=FontsData.font->h-1;
			last=i+1;
		}
	}
	chars_big=NULL;
	anz=0;
	last=0;
	for ( i=0;i<FontsData.font_big->w;i++ )
	{
		if ( ( ( unsigned int* ) FontsData.font_big->pixels ) [ ( FontsData.font_big->h-1 ) *FontsData.font_big->w+i]==0xFF00FF )
		{
			anz++;
			chars_big= ( SDL_Rect* ) realloc ( chars_big,sizeof ( SDL_Rect ) *anz );
			chars_big[anz-1].x=last;
			chars_big[anz-1].y=0;
			chars_big[anz-1].w=i-last+1;
			chars_big[anz-1].h=FontsData.font_big->h-1;
			last=i+1;
		}
	}
	chars_small=NULL;
	anz=0;
	last=0;
	for ( i=0;i<FontsData.font_small_white->w;i++ )
	{
		if ( ( ( unsigned int* ) FontsData.font_small_white->pixels ) [ ( FontsData.font_small_white->h-1 ) *FontsData.font_small_white->w+i]==0xFF00FF )
		{
			anz++;
			chars_small= ( SDL_Rect* ) realloc ( chars_small,sizeof ( SDL_Rect ) *anz );
			chars_small[anz-1].x=last;
			chars_small[anz-1].y=0;
			chars_small[anz-1].w=i-last+1;
			chars_small[anz-1].h=FontsData.font_small_white->h-1;
			last=i+1;
		}
	}
}

cFonts::~cFonts ( void )
{
	free ( chars );
	free ( chars_big );
	free ( chars_small );
}

// Konvertiert den Zeichensatz:
int cFonts::Charset ( char c )
{
	if ( c>='a'&&c<='z' ) return c-'a';
	if ( c>='0'&&c<='9' ) return 26+c-'0';
	if ( c>='A'&&c<='Z' ) return 36+c-'A';
	switch ( c )
	{
		case 'ä': return 62;
		case 'ö': return 63;
		case 'ü': return 64;
		case 'Ä': return 65;
		case 'Ö': return 66;
		case 'Ü': return 67;
		case 'ß': return 68;
		case ',': return 69;
		case '.': return 70;
		case ';': return 71;
		case ':': return 72;
		case '-': return 73;
		case '_': return 74;
		case '#': return 75;
		case '\'': return 76;
		case '?': return 76;
		case '+': return 77;
		case '*': return 78;
		case '~': return 79;
		case '|': return 80;
		case '<': return 81;
		case '>': return 82;
		case '^': return 83;
		case '°': return 84;
		case '!': return 85;
		case '"': return 86;
		case '§': return 87;
		case '$': return 88;
		case '%': return 89;
		case '&': return 90;
		case '(': return 91;
		case ')': return 92;
		case '=': return 93;
		//case '?': return 94; //see case 76!
		case '`': return 95;
		case '{': return 96;
		case '[': return 97;
		case ']': return 98;
		case '}': return 99;
		case '\\': return 100;
		case '/': return 101;
		case '²': return 102;
		case '³': return 103;
	}
	return -1;
}

// Gibt einen Text mit dem normalen Font aus:
void cFonts::OutText ( char *str,int x,int y,SDL_Surface *sf )
{
	SDL_Rect dest;
	int i=0,index;

	dest.x=x;
	dest.y=y;

	while ( str[i]!=0 )
	{
		index=Charset ( str[i++] );
		if ( index<0 )
		{
			dest.x+=5;
		}
		else
		{
			dest.w=chars[index].w;
			dest.h=chars[index].h;
			SDL_BlitSurface ( FontsData.font,chars+index,sf,&dest );
			dest.x+=dest.w;
		}
	}
}

// Konvertiert den Zeichensatz:
int cFonts::CharsetBig ( char c )
{
	if ( c>='A'&&c<='Z' ) return c-'A';
	if ( c>='a'&&c<='z' ) return 26+c-'a';
	if ( c>='0'&&c<='9' ) return 64+c-'0';
	switch ( c )
	{
		case 'ä': return 52;
		case 'ö': return 53;
		case 'ü': return 54;
		case 'Ä': return 55;
		case 'Ö': return 56;
		case 'Ü': return 57;
		case '?': return 58;
		case '(': return 59;
		case ')': return 60;
		case '/': return 61;
		case '+': return 62;
		case '-': return 63;
		case '&': return 74;
		case '.': return 75;
	}
	return -1;
}

// Gibt einen Text mit dem Big Font aus:
void cFonts::OutTextBig ( char *str,int x,int y,SDL_Surface *sf )
{
	SDL_Rect dest;
	int i=0,index;

	dest.x=x;
	dest.y=y;

	while ( str[i]!=0 )
	{
		index=CharsetBig ( str[i++] );
		if ( index<0 )
		{
			dest.x+=8;
		}
		else
		{
			dest.w=chars_big[index].w;
			dest.h=chars_big[index].h;
			SDL_BlitSurface ( FontsData.font_big,chars_big+index,sf,&dest );
			dest.x+=dest.w;
		}
	}
}

void cFonts::OutTextBigCenter ( char *str,int x,int y,SDL_Surface *sf )
{
	SDL_Rect dest,tmp;
	int i=0,index,width=0;

	while ( str[i]!=0 )
	{
		index=CharsetBig ( str[i++] );
		if ( index<0 )
		{
			width+=8;
		}
		else
		{
			width+=chars_big[index].w;
		}
	}
	i=0;
	dest.x=x-width/2;
	dest.y=y;
	while ( str[i]!=0 )
	{
		index=CharsetBig ( str[i++] );
		if ( index<0 )
		{
			dest.x+=8;
		}
		else
		{
			dest.w=chars_big[index].w;
			dest.h=chars_big[index].h;
			tmp=dest;
			SDL_BlitSurface ( FontsData.font_big,chars_big+index,sf,&tmp );
			dest.x+=dest.w;
		}
	}
}

void cFonts::OutTextBigCenterGold ( char *str,int x,int y,SDL_Surface *sf )
{
	SDL_Rect dest,tmp;
	int i=0,index,width=0;

	while ( str[i]!=0 )
	{
		index=CharsetBig ( str[i++] );
		if ( index<0 )
		{
			width+=8;
		}
		else
		{
			width+=chars_big[index].w;
		}
	}
	i=0;
	dest.x=x-width/2;
	dest.y=y;
	while ( str[i]!=0 )
	{
		index=CharsetBig ( str[i++] );
		if ( index<0 )
		{
			dest.x+=8;
		}
		else
		{
			dest.w=chars_big[index].w;
			dest.h=chars_big[index].h;
			tmp=dest;
			SDL_BlitSurface ( FontsData.font_big_gold,chars_big+index,sf,&tmp );
			dest.x+=dest.w;
		}
	}
}

// Konvertiert den Zeichensatz:
int cFonts::CharsetSmall ( char c )
{
	if ( c>='0'&&c<='9' ) return c-'0';
	if ( c>='A'&&c<='Z' ) return c-'A'+10;
	if ( c>='a'&&c<='z' ) return c-'a'+10;
	switch ( c )
	{
		case '/': return 36;
		case 'Ä': return 37;
		case 'ä': return 37;
		case 'Ö': return 38;
		case 'ö': return 38;
		case 'Ü': return 39;
		case 'ü': return 39;
		case '_': return 40;
		case '+': return 41;
		case '*': return 42;
		case '-': return 43;
		case '!': return 44;
		case '"': return 45;
		case '%': return 46;
		case '\\': return 47;
		case '(': return 48;
		case ')': return 49;
		case '[': return 50;
		case ']': return 51;
		case '=': return 52;
		case '\'': return 53;
		case '?': return 53;
		case '`': return 53;
		case '~': return 54;
		case '#': return 55;
		case '.': return 56;
		case ',': return 57;
		case ':': return 58;
		case '<': return 59;
		case '>': return 60;
		case '^': return 61;
		case '°': return 62;
		case 'ß': return 63;
		case '|': return 1;
		case '\n': return -2;
	}
	return -1;
}

// Gibt einen Text mit dem Small Font aus:
void cFonts::OutTextSmall ( char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf )
{
	SDL_Rect dest;
	int i=0,index;

	dest.x=x;
	dest.y=y;

	while ( str[i]!=0 )
	{
		index=CharsetSmall ( str[i++] );
		if ( index<0 )
		{
			if ( index==-2 )
			{
				dest.x=x;
				dest.y+=8;
			}
			else
			{
				dest.x+=4;
			}
		}
		else
		{
			dest.w=chars_small[index].w;
			dest.h=chars_small[index].h;
			switch ( color )
			{
				case ClWhite:
					SDL_BlitSurface ( FontsData.font_small_white,chars_small+index,sf,&dest );
					break;
				case ClRed:
					SDL_BlitSurface ( FontsData.font_small_red,chars_small+index,sf,&dest );
					break;
				case ClGreen:
					SDL_BlitSurface ( FontsData.font_small_green,chars_small+index,sf,&dest );
					break;
				case ClYellow:
					SDL_BlitSurface ( FontsData.font_small_yellow,chars_small+index,sf,&dest );
					break;
			}
			dest.x+=dest.w;
		}
	}
}

// Gibt den Text mit dem normalen Font zentriert aus:
void cFonts::OutTextCenter ( char *str,int x,int y,SDL_Surface *sf )
{
	SDL_Rect dest,tmp;
	int i=0,index,width=0;

	while ( str[i]!=0 )
	{
		index=Charset ( str[i++] );
		if ( index<0 )
		{
			width+=5;
		}
		else
		{
			width+=chars[index].w;
		}
	}
	i=0;
	dest.x=x-width/2;
	dest.y=y;
	while ( str[i]!=0 )
	{
		index=Charset ( str[i++] );
		if ( index<0 )
		{
			dest.x+=5;
		}
		else
		{
			dest.w=chars[index].w;
			dest.h=chars[index].h;
			tmp=dest;
			SDL_BlitSurface ( FontsData.font,chars+index,sf,&tmp );
			dest.x+=dest.w;
		}
	}
}

// Ermittelt die Text-Länge:
int cFonts::GetTextLen ( char *str )
{
	int i=0,index,width=0;
	while ( str[i]!=0 )
	{
		index=Charset ( str[i++] );
		if ( index<0 )
		{
			width+=5;
		}
		else
		{
			width+=chars[index].w;
		}
	}
	return width;
}

// Ermittelt die Text-Länge:
int cFonts::GetTextLenSmall ( char *str )
{
	int i=0,index,width=0;
	while ( str[i]!=0 )
	{
		index=CharsetSmall ( str[i++] );
		if ( index<0 )
		{
			width+=4;
		}
		else
		{
			width+=chars_small[index].w;
		}
	}
	return width;
}

// Gibt den Text mit dem small Font zentriert aus:
void cFonts::OutTextSmallCenter ( char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf )
{
	SDL_Rect dest;
	int i=0,index,width=0;

	while ( str[i]!=0 )
	{
		index=CharsetSmall ( str[i++] );
		if ( index<0 )
		{
			width+=4;
		}
		else
		{
			width+=chars_small[index].w;
		}
	}
	i=0;
	dest.x=x-width/2;
	dest.y=y;
	while ( str[i]!=0 )
	{
		index=CharsetSmall ( str[i++] );
		if ( index<0 )
		{
			dest.x+=4;
		}
		else
		{
			dest.w=chars_small[index].w;
			dest.h=chars_small[index].h;
			switch ( color )
			{
				case ClWhite:
					SDL_BlitSurface ( FontsData.font_small_white,chars_small+index,sf,&dest );
					break;
				case ClRed:
					SDL_BlitSurface ( FontsData.font_small_red,chars_small+index,sf,&dest );
					break;
				case ClGreen:
					SDL_BlitSurface ( FontsData.font_small_green,chars_small+index,sf,&dest );
					break;
				case ClYellow:
					SDL_BlitSurface ( FontsData.font_small_yellow,chars_small+index,sf,&dest );
					break;
			}
			dest.x+=dest.w;
		}
	}
}

// Gibt den Text mit automatischem Zeilenumbruch aus:
void cFonts::OutTextBlock ( char *str,SDL_Rect block,SDL_Surface *sf )
{
	char word[50],*ptr,*p,*p2;
	int len,x;
	ptr=str;
	x=block.x;
	// Wort für Wort durch den String gehen:
	while ( ptr )
	{
		p=strchr ( ptr,' ' );
		p2=strchr ( ptr,'\n' );
		if ( ( p2&&p&&p2<p ) || ( p2&&!p ) )
		{
			p=p2;
		}
		if ( p )
		{
			len=p-ptr;
			if ( len>49 ) len=49;
			strncpy ( word,ptr,len );
			word[len]=0;
			ptr=p+1;
		}
		else
		{
			if ( strlen ( ptr ) >=50 )
			{
				strncpy ( word,ptr,49 );
				word[49]=0;
			}
			else
			{
				strcpy ( word,ptr );
			}
			ptr=0;
		}
		len=GetTextLen ( word );
		if ( block.x+len>x+block.w )
		{
			block.x=x;
			block.y+=11;
		}
		OutText ( word,block.x,block.y,sf );
		block.x+=len+5;
		if ( p&&*p=='\n' )
		{
			block.x=x;
			block.y+=11;
		}
	}
}

