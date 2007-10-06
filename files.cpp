//////////////////////////////////////////////////////////////////////////////
// M.A.X. - files.cpp
//////////////////////////////////////////////////////////////////////////////
#include "files.h"
#include "log.h"

// Prüft ob die Datei 'name' existiert
bool FileExists ( const char* name )
{
	SDL_RWops *file;
	file=SDL_RWFromFile ( name, "r" );
	if ( file==NULL )
	{
		cLog::write(SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		return false;
	}
	SDL_RWclose ( file );
	return true;
}

string LoadFileToString ( const char* filename )
{
	SDL_RWops *file;
	char temp[256];
	string str;
	long end;

	//sanitychecks
	if (!FileExists(filename)) return NULL;

	file=SDL_RWFromFile ( filename, "r" );
	if ( file==NULL )
	{
		cLog::write(SDL_GetError(), cLog::eLOG_TYPE_ERROR);
		return NULL;
	}
	SDL_RWseek ( file,0,SEEK_END );
	end = SDL_RWtell ( file );
	SDL_RWseek ( file,0,SEEK_SET );
	while ( SDL_RWtell ( file ) <end )
	{
		int read=SDL_RWread ( file,temp,sizeof ( char ),255 );
		temp[read]='\0';
		str+=temp;
	}
	SDL_RWclose ( file );
	return str;
}

long StrPosToFilePos ( string filestring, long strpos )
{
	long filepos=0;

	if ( strpos!=string::npos )
	{
		int lastfind=0;
		while ( lastfind<strpos && lastfind!=string::npos )
		{
			lastfind= ( int ) filestring.find ( "\n",lastfind+1 );
			if ( lastfind<strpos && lastfind!=string::npos )
				filepos++;
		}
		filepos+=strpos;
		return filepos;
	}
	return -1;
}

long SearchFor ( const char* searchstr, long startpos, const char* filename )
{
	SDL_RWops *file;
	char temp[256];
	string str;
	long end, pos, filepos=0;

	//sanitychecks
	if (!FileExists(filename)) return -1;

	file=SDL_RWFromFile ( filename, "r" );
	SDL_RWseek ( file,0,SEEK_END );
	end = SDL_RWtell ( file );
	SDL_RWseek ( file,startpos,SEEK_SET );
	while ( SDL_RWtell ( file ) <end )
	{
		int read=SDL_RWread ( file,temp,sizeof ( char ),255 );
		temp[read]='\0';
		str+=temp;
	}
	pos= ( long ) str.find ( searchstr,0 );
	if ( pos!=string::npos )
	{
		int lastfind=0;
		while ( lastfind<pos && lastfind!=string::npos )
		{
			lastfind= ( int ) str.find ( "\n",lastfind+1 );
			if ( lastfind<pos && lastfind!=string::npos )
				filepos++;
		}
		filepos+=pos+startpos;
		SDL_RWclose ( file );
		return filepos;
	}
	SDL_RWclose ( file );
	/*
		while(SDL_RWtell(file) != end)
		{
			for(int i=0;i<256;i++){
				filepos=SDL_RWtell(file);
				SDL_RWread(file,tmp,1,1);
				temp[i]=tmp[0];
				if(tmp[0]=='\n'){
					temp[i+1]='\0';
					break;
				}
			}
			str=temp;
			pos=(long)str.find(searchstr,0);
			if(pos!=string::npos){
				pos=(long)str.length()+1-pos;
				filepos-=pos;
				SDL_RWclose(file);
				return filepos;
			}
		}*/
	return -1;
}

int FileEraseData ( long startpos, long endpos, const char* filename )
{
	SDL_RWops *file;
	char tmp[2];
	char *tmp2;
	string filedata;
	long end;
	int n=1;

	//sanitychecks
	if ( endpos!=0 && endpos<startpos ) return 0;
	if (!FileExists(filename)) return 0;



	SDL_RWseek ( file,0,SEEK_END );
	end = SDL_RWtell ( file );
	if ( endpos==0 )
		endpos=end;
	SDL_RWseek ( file,0,SEEK_SET );
	while ( SDL_RWtell ( file ) !=end )
	{
		SDL_RWread ( file,tmp,1,1 );
		if ( tmp[0]=='\n' && startpos>=SDL_RWtell ( file )-n )
		{
			startpos--;
			endpos--;
			n++;
		}
		tmp[1]='\0';
		filedata+=tmp;
	}
	SDL_RWclose ( file );
	filedata.erase ( startpos,endpos-startpos );

	file=SDL_RWFromFile ( filename, "w" );
	if ( file==NULL )
		return 0;
	string str;
	for ( int i=0;i< ( int ) filedata.length();i++ )
	{
		str=filedata.substr ( i,1 ).c_str();
		tmp2= ( char * ) str.c_str();
		SDL_RWwrite ( file,tmp2,1,1 );
	}
	SDL_RWclose ( file );
	return 1;
}

int FileInsertData ( long pos, const char* data, const char* filename )
{
	SDL_RWops *file;
	char tmp[2];
	char *tmp2;
	string filedata;
	long end;
	int n=1;

	//sanitychecks
	if (!FileExists(filename)) return 0;

	file=SDL_RWFromFile ( filename, "r" );
	SDL_RWseek ( file,0,SEEK_END );
	end = SDL_RWtell ( file );
	SDL_RWseek ( file,0,SEEK_SET );
	while ( SDL_RWtell ( file ) !=end )
	{
		SDL_RWread ( file,tmp,1,1 );
		if ( tmp[0]=='\n' && pos>=SDL_RWtell ( file )-n )
		{
			pos--;
			n++;
		}
		tmp[1]='\0';
		filedata+=tmp;
	}
	SDL_RWclose ( file );
	if ( pos> ( int ) filedata.length() )
		return 0;
	filedata.insert ( pos,data );

	file=SDL_RWFromFile ( filename, "w" );
	if ( file==NULL )
		return 0;
	string str;
	for ( int i=0;i< ( int ) filedata.length();i++ )
	{
		str=filedata.substr ( i,1 ).c_str();
		tmp2= ( char * ) str.c_str();
		SDL_RWwrite ( file,tmp2,1,1 );
	}
	SDL_RWclose ( file );
	return 1;
}

// Gibt alle Sektionen einer Initialisierungsdatei zurück
TList *ReadIniSections ( const char* name )
{
	SDL_RWops *file;
	long end;
	char tmp[2];
	char temp[256];
	TList *sections;
	string str;

	file=SDL_RWFromFile ( name, "r" );
	if ( file==NULL )
		return NULL;

	sections = new TList;
	SDL_RWseek ( file,0,SEEK_END );
	end = SDL_RWtell ( file );
	SDL_RWseek ( file,0,SEEK_SET );
	while ( SDL_RWtell ( file ) != end )
	{
		for ( int i=0;i<256;i++ )
		{
			SDL_RWread ( file,tmp,1,1 );
			temp[i]=tmp[0];
			if ( tmp[0]=='\n' )
			{
				temp[i]='\0';
				break;
			}
		}
		if ( temp[0] == '[' )
		{
			str.erase ( 0 );
			str.insert ( 0,temp );
			str.erase ( 0,1 );
			str.erase ( str.length()-1,1 );
			sections->Add ( str );
		}
		SDL_RWseek ( file,0,SEEK_CUR );
	}
	SDL_RWclose ( file );
	return sections;
}

// Liest den String des Schlüssels 'key' der Sektion 'section' aus und gibt diesen zurück.
// Kann dieser nich gelesen werden wird 'dafaultValue' zurückgegeben
char *ReadIniString ( const char* section, const char* key, const char* defaultValue, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string filestring, stmp;
	char sztmp[256];
	char *returnkey;

	file=SDL_RWFromFile ( filename, "r" );
	if ( file==NULL )
	{
		cLog::write("Missing file",1);
		cLog::write(filename);
		return ( char * ) defaultValue;
	}
	filestring = LoadFileToString ( filename );
	// Section Länge ermitteln
	stmp="[";stmp+=section;stmp+="]";
	startpos = ( long ) filestring.find ( stmp );
	if ( startpos==-1 )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return ( char * ) defaultValue;
	}
	startpos+= ( long ) stmp.length();
	endpos = ( long ) filestring.find ( "[",startpos );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	keypos= ( long ) filestring.find ( key,startpos );
	if ( keypos==-1 || keypos>endpos )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return ( char * ) defaultValue;
	}
	keypos=StrPosToFilePos ( filestring, ( keypos+ ( long ) strlen ( key ) +1 ) );
	SDL_RWseek ( file,keypos,SEEK_SET );
	returnkey= ( char * ) malloc ( 256 );
	SDL_RWread ( file,sztmp,1,256 );
	stmp=sztmp;
	stmp=stmp.substr ( 0,stmp.find ( "\n",0 ) );
	strcpy ( returnkey,stmp.c_str() );
	SDL_RWclose ( file );
	return returnkey;
}

// Liest den Integer des Schlüssels 'key' der Sektion 'section' aus und gibt diesen zurück.
// Kann dieser nich gelesen werden wird 'dafaultValue' zurückgegeben
int ReadIniInteger ( const char* section, const char* key, int defaultValue, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string filestring, stmp;
	char sztmp[256];

	file=SDL_RWFromFile ( filename, "r" );
	if ( file==NULL )
	{
		cLog::write("Missing file",1);
		cLog::write(filename);
		return defaultValue;
	}
	filestring = LoadFileToString ( filename );
	// Section Länge ermitteln
	stmp="[";stmp+=section;stmp+="]";
	startpos = ( long ) filestring.find ( stmp );
	if ( startpos==-1 )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return defaultValue;
	}
	startpos+= ( long ) stmp.length();
	endpos = ( long ) filestring.find ( "[",startpos );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	keypos= ( long ) filestring.find ( key,startpos );
	if ( keypos==-1 || keypos>endpos )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(section,1);
		cLog::write(key,1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return defaultValue;
	}
	keypos=StrPosToFilePos ( filestring, ( keypos+ ( long ) strlen ( key ) +1 ) );
	SDL_RWseek ( file,keypos,SEEK_SET );
	SDL_RWread ( file,sztmp,1,256 );
	stmp=sztmp;
	stmp=stmp.substr ( 0,stmp.find ( "\n",0 ) );
	SDL_RWclose ( file );
	return atoi ( stmp.c_str() );
}

bool ReadIniBool ( const char* section, const char* key, bool defaultValue, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string filestring, stmp;
	char sztmp[256];

	file=SDL_RWFromFile ( filename, "r" );
	if ( file==NULL )
	{
		cLog::write("Missing file",1);
		cLog::write(filename);
		return defaultValue;
	}

	filestring = LoadFileToString ( filename );
	// Section Länge ermitteln
	stmp="[";
	stmp+=section;
	stmp+="]";

	startpos = ( long ) filestring.find ( stmp );
	if ( startpos==-1 )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(section,1);
		cLog::write(key,1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return defaultValue;
	}
	startpos+= ( long ) stmp.length();
	endpos= ( long ) filestring.find ( "[",startpos );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	keypos= ( long ) filestring.find ( key,startpos );
	if ( keypos==-1 || keypos>endpos )
	{
		cLog::write("File corrupted. Missing entry:",1);
		cLog::write(section,1);
		cLog::write(key,1);
		cLog::write(filename);
		SDL_RWclose ( file );
		return defaultValue;
	}
	keypos=StrPosToFilePos ( filestring, ( keypos+ ( long ) strlen ( key ) +1 ) );
	SDL_RWseek ( file,keypos,SEEK_SET );
	SDL_RWread ( file,sztmp,1,256 );
	stmp=sztmp;
	stmp=stmp.substr ( 0,stmp.find ( "\n",0 ) );
	SDL_RWclose ( file );
	//TODO: Heck, what did MM mean by this? I wish he would
	//have commented more.. Can sb please verify that I "translated"
	//the following comment correct since you can not use NULL in an 
	//arithmetic?

	//if ( strcmp ( stmp.c_str(),"false" ) == NULL || strcmp ( stmp.c_str(),"0" ) == NULL )

	if ( strcmp ( stmp.c_str(),"false" ) != 0 || strcmp ( stmp.c_str(),"0" ) != 0 )
		return false;
	else
		return true;
}

// Schreibt den String 'Value' hinter den Schlüssel 'key' der Sektion 'section'.
// Existieren die Sektion oder der Schlüssel nicht, werden diese erstellt.
// Vorhandene Werte werden überschrieben.
void WriteIniString ( const char* section, const char* key, const char* Value, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string stmp;

	file=SDL_RWFromFile ( filename, "r+" );
	if ( file==NULL )
		return ;

	// Section Länge ermitteln
	stmp="[";stmp+=section;stmp+="]";
	startpos = SearchFor ( stmp.c_str(),0,filename );
	if ( startpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		startpos=SDL_RWtell ( file );
		FileInsertData ( startpos,"\n",filename );
		FileInsertData ( startpos+2,stmp.c_str(),filename );
		FileInsertData ( startpos+ ( long ) stmp.length() +2,"\n",filename );
	}
	startpos+= ( long ) stmp.length() +4;
	endpos = SearchFor ( "[",startpos,filename );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	stmp=key;stmp+="=";
	keypos=SearchFor ( stmp.c_str(),startpos,filename );
	if ( keypos==-1 || keypos>endpos )
	{
		FileInsertData ( startpos,key,filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ),"=",filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ) +1,Value,filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ) +1+ ( long ) strlen ( Value ),"\n",filename );
		SDL_RWclose ( file );
	}
	else
	{
		keypos=keypos+ ( long ) strlen ( key ) +2;
		FileInsertData ( keypos,"=",filename );
		FileInsertData ( keypos+1,Value,filename );
		FileInsertData ( keypos+ ( long ) strlen ( Value ) +1,"\n",filename );
		SDL_RWclose ( file );
		long eraseend=SearchFor ( "\n",keypos+ ( long ) strlen ( Value ) +3,filename );
		FileEraseData ( keypos+ ( long ) strlen ( Value ) +2,eraseend+2,filename );
	}
	return;
}

// Schreibt den Integer 'Value' hinter den Schlüssel 'key' der Sektion 'section'.
// Existieren die Sektion oder der Schlüssel nicht, werden diese erstellt.
// Vorhandene Werte werden überschrieben.
void WriteIniInteger ( const char* section, const char* key, int Value, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string stmp;
	char charValue[256];
	sprintf ( charValue,"%d",Value );

	file=SDL_RWFromFile ( filename, "r+" );
	if ( file==NULL )
		return ;

	// Section Länge ermitteln
	stmp="[";stmp+=section;stmp+="]";
	startpos = SearchFor ( stmp.c_str(),0,filename );
	if ( startpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		startpos=SDL_RWtell ( file );
		FileInsertData ( startpos,"\n",filename );
		FileInsertData ( startpos+2,stmp.c_str(),filename );
		FileInsertData ( startpos+ ( long ) stmp.length() +2,"\n",filename );
	}
	startpos+= ( long ) stmp.length() +4;
	endpos = SearchFor ( "[",startpos,filename );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	stmp=key;stmp+="=";
	keypos=SearchFor ( stmp.c_str(),startpos,filename );
	if ( keypos==-1 || keypos>endpos )
	{
		FileInsertData ( startpos,key,filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ),"=",filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ) +1,charValue,filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ) +1+ ( long ) strlen ( charValue ),"\n",filename );
		SDL_RWclose ( file );
	}
	else
	{
		keypos=keypos+ ( long ) strlen ( key ) +2;
		FileInsertData ( keypos,"=",filename );
		FileInsertData ( keypos+1,charValue,filename );
		FileInsertData ( keypos+ ( long ) strlen ( charValue ) +1,"\n",filename );
		SDL_RWclose ( file );
		long eraseend=SearchFor ( "\n",keypos+ ( long ) strlen ( charValue ) +3,filename );
		FileEraseData ( keypos+ ( long ) strlen ( charValue ) +2,eraseend+2,filename );
	}
	return;
}

// Schreibt den boolschen Wert 'Value' hinter den Schlüssel 'key' der Sektion 'section'.
// Existieren die Sektion oder der Schlüssel nicht, werden diese erstellt.
// Vorhandene Werte werden überschrieben.
void WriteIniBool ( const char* section, const char* key, bool Value, const char* filename )
{
	SDL_RWops *file;
	long startpos, endpos, keypos;
	string stmp;

	file=SDL_RWFromFile ( filename, "r+" );
	if ( file==NULL )
		return ;

	// Section Länge ermitteln
	stmp="[";stmp+=section;stmp+="]";
	startpos = SearchFor ( stmp.c_str(),0,filename );
	if ( startpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		startpos=SDL_RWtell ( file );
		FileInsertData ( startpos,"\n",filename );
		FileInsertData ( startpos+2,stmp.c_str(),filename );
		FileInsertData ( startpos+ ( long ) stmp.length() +2,"\n",filename );
	}
	startpos+= ( long ) stmp.length() +4;
	endpos = SearchFor ( "[",startpos,filename );
	if ( endpos==-1 )
	{
		SDL_RWseek ( file,0,SEEK_END );
		endpos=SDL_RWtell ( file );
	}
	// Wert ermitteln
	stmp=key;stmp+="=";
	keypos=SearchFor ( stmp.c_str(),startpos,filename );
	if ( keypos==-1 || keypos>endpos )
	{
		FileInsertData ( startpos,key,filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ),"=",filename );
		if ( Value )
			FileInsertData ( startpos+ ( long ) strlen ( key ) +1,"1",filename );
		else
			FileInsertData ( startpos+ ( long ) strlen ( key ) +1,"0",filename );
		FileInsertData ( startpos+ ( long ) strlen ( key ) +1+1,"\n",filename );
		SDL_RWclose ( file );
	}
	else
	{
		keypos=keypos+ ( long ) strlen ( key ) +2;
		FileInsertData ( keypos,"=",filename );
		if ( Value )
			FileInsertData ( keypos+1,"1",filename );
		else
			FileInsertData ( keypos+1,"0",filename );
		FileInsertData ( keypos+1+1,"\n",filename );
		SDL_RWclose ( file );
		long eraseend=SearchFor ( "\n",keypos+1+3,filename );
		FileEraseData ( keypos+1+2,eraseend+2,filename );
	}
	return;
}
