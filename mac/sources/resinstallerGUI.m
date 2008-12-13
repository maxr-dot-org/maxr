/***************************************************************************
 *              Resinstaller - installs missing GFX for MAXR               *
 *              This file is part of the resinstaller project              *
 *   Copyright (C) 2007, 2008 Eiko Oltmanns                                *
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

// author: Paul Grathwohl

#include "resinstallerGUI.h"
#include <SDL.h>
#import <Cocoa/Cocoa.h>

//-------------------------------------------------------------
std::string askForCDPath (bool (*validationFunction)(std::string&))
{
	while (1)
	{
		NSOpenPanel* openPanel = [NSOpenPanel openPanel];
		[openPanel setCanChooseFiles:false];
		[openPanel setCanChooseDirectories:true];
		[openPanel setAllowsMultipleSelection:false];
		[openPanel setTitle:@"Select folder of a M.A.X. installation or M.A.X.-CD"];
		
		int result = [openPanel runModalForTypes:nil];
		if (result == NSOKButton)
		{
			NSArray* files = [openPanel filenames]; 
			NSString* maxSrcPath = [files objectAtIndex:0];
			std::string userPath = [maxSrcPath UTF8String];
			if ((*validationFunction)(userPath))
			{
				[openPanel release];
				return userPath;
			}
			else
			{
				NSAlert* alert = [NSAlert new];
				[alert setMessageText: @"Please select another folder!"];
				[alert setInformativeText: @"Couldn't find a valid M.A.X. installation or M.A.X. CD in the given folder.\nThe file max.res was not found."];
				[alert runModal];
				[alert release];
			}
		}
		else
		{
			NSAlert* alert = [NSAlert new];
			[alert setMessageText: @"You canceled the installation..."];
			[alert setInformativeText: @"Feel free to come back later!"];
			[alert runModal];
			[alert release];
			[openPanel release];
			return "";
		}
	}
}

//-------------------------------------------------------------
std::string askForOutputPath (bool (*validationFunction)(std::string&))
{
	while (1)
	{
		NSOpenPanel* openPanel = [NSOpenPanel openPanel];
		[openPanel setCanChooseFiles:false];
		[openPanel setCanChooseDirectories:true];
		[openPanel setAllowsMultipleSelection:false];
		[openPanel setTitle:@"Select target M.A.X.R. folder"];
		
		int result = [openPanel runModalForTypes:nil];
		if (result == NSOKButton)
		{
			NSArray* files = [openPanel filenames]; 
			NSString* outputPath = [files objectAtIndex:0];
			std::string userPath = [outputPath UTF8String];
			if ((*validationFunction)(userPath))
			{
				[openPanel release];
				return userPath;
			}
			else
			{
				NSAlert* alert = [NSAlert new];
				[alert setMessageText: @"No valid M.A.X.R installation in the given folder!"];
				[alert setInformativeText: @"The folder doesn't contain a M.A.X.R. installation. The files have to be extracted into the folder of an existing M.A.X.R. installation."];
				[alert runModal];
				[alert release];
			}
		}
		else
		{
			NSAlert* alert = [NSAlert new];
			[alert setMessageText: @"You canceled the installation..."];
			[alert setInformativeText: @"Feel free to come back later!"];
			[alert runModal];
			[alert release];
			[openPanel release];
			return "";
		}
	}
}

//-------------------------------------------------------------
int askForLanguage (bool germanAvailable, bool italianAvailable, bool frenchAvailable)
{
	NSAlert* alert = [NSAlert new];
	[alert setMessageText: @"Which language do you want to install?"];
	[alert setInformativeText: @"The following languages exist on your installation medium:"];
	
	NSMatrix* accessory = [[NSMatrix alloc] initWithFrame:NSMakeRect (0, 0, 150, 100)];

	int languages = 0;
	int english = -1;
	int german = -1;
	int italian = -1;
	int french = -1;
	
	// english is always available:
	NSButton *buttonEng = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 150, 20)];
	[buttonEng setTitle: @"English"];
	[buttonEng setButtonType:NSRadioButton];
	[accessory addRow];
	[accessory putCell:[buttonEng cell] atRow:0 column:0];
	english = languages;
	languages++;
	
	if (germanAvailable)
	{
		NSButton *buttonGer = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 150, 20)];
		[buttonGer setTitle: @"Deutsch"];
		[buttonGer setButtonType:NSRadioButton];
		[accessory addRow];
		[accessory putCell:[buttonGer cell] atRow:languages column:0];
		german = languages;
		languages++;
	}
	if (italianAvailable)
	{
		NSButton *buttonIt = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 150, 20)];
		[buttonIt setTitle: @"Italiano"];
		[buttonIt setButtonType:NSRadioButton];
		[accessory addRow];
		[accessory putCell:[buttonIt cell] atRow:languages column:0];
		italian = languages;
		languages++;
	}
	if (frenchAvailable)
	{
		NSButton *buttonFr = [[NSButton alloc] initWithFrame:NSMakeRect(0, 0, 150, 20)];
		[buttonFr setTitle: @"Francais"];
		[buttonFr setButtonType:NSRadioButton];
		[accessory addRow];
		[accessory putCell:[buttonFr cell] atRow:languages column:0];
		french = languages;
		languages++;
	}
	
	[accessory setCellSize:NSMakeSize(150, 30)];
	[accessory selectCellAtRow:0 column:0];
	
	[alert setAccessoryView: accessory];
	
	[alert runModal];
	
	int result = [accessory selectedRow];

	[alert release];

	
	if (result < 0)
		result = 0;
	if (result == english) 
		return 0;
	if (result == german) 
		return 1;
	if (result == italian) 
		return 2;
	if (result == french)
		return 3;
	return 0;
}

// variables used for progress window
std::string curTaskDescription = "";
int curTotalFiles = 0;
int curInstalledFiles = 0;
bool progressChanged = false;
SDL_mutex* mutex = 0; // for thread safety (when updating the progress window)

//-------------------------------------------------------------
void displayProgressWindow (bool& installationFinished)
{	
	NSWindow* progressWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 400, 120) styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:false];
	[progressWindow center];
	[progressWindow setCanHide:false];
	[progressWindow setAutodisplay:true];
	//[progressWindow display];
	[progressWindow setTitle:@"Installing - please wait..."];
	
	NSImage* image = [[NSImage alloc] initByReferencingFile:@"maxr_mac.bmp"];
	NSImageView* imageView = [[NSImageView alloc] initWithFrame: NSMakeRect(20, 36, 64, 64)];
	[imageView setImage:image];
	[[progressWindow contentView] addSubview:imageView];

	NSTextView* installingLabel = [[NSTextView alloc] initWithFrame: NSMakeRect(100, 80, 250, 20)];
	[[installingLabel textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:@"Installing:"]];
	[installingLabel setEditable:false];
	[installingLabel setDrawsBackground:false];
	[[progressWindow contentView] addSubview:installingLabel];
	
	NSTextView* label = [[NSTextView alloc] initWithFrame: NSMakeRect(160, 80, 230, 20)];
	[[label textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:curTaskDescription.c_str()]]];
	[label setEditable:false];
	[label setDrawsBackground:false];
	[[progressWindow contentView] addSubview:label];
	
	NSProgressIndicator* progressBar = [[NSProgressIndicator alloc] initWithFrame: NSMakeRect(100, 50, 250, 20)];
	[progressBar setIndeterminate:false];
	[progressBar setMinValue:0];
	[progressBar setMaxValue:curTotalFiles];
	[progressBar setDoubleValue:curInstalledFiles];	
	[[progressWindow contentView] addSubview: progressBar];
	
	if (mutex == 0)
		mutex = SDL_CreateMutex ();
	
	NSModalSession session = [NSApp beginModalSessionForWindow:progressWindow];
	while (1)
	{
		if ([NSApp runModalSession:session] != NSRunContinuesResponse)
			break;
		if (installationFinished == true)
			break;
		
		if (mutex != 0)
			SDL_LockMutex (mutex);
		if (progressChanged)
		{
			[[label textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:[NSString stringWithUTF8String:curTaskDescription.c_str()]]];
			[progressBar setMaxValue:curTotalFiles];
			[progressBar setDoubleValue:curInstalledFiles];	
			progressChanged = false;
		}
		if (mutex != 0)
			SDL_UnlockMutex (mutex);
		
		[NSThread sleepForTimeInterval: 0.05]; // sleep for 50 ms
	}
	if (mutex != 0)
	{
		SDL_DestroyMutex (mutex);
		mutex = 0;
	}
	
	[NSApp endModalSession:session];
	[progressWindow close];
}

//-------------------------------------------------------------
void updateProgressWindow (std::string taskDescription, int totalFiles, int installedFiles)
{
	if (mutex == 0)
		mutex = SDL_CreateMutex ();

	if (mutex != 0)
		SDL_LockMutex (mutex);

	bool somethingChanged = false; // used to bundle all changes as one update.
	if (taskDescription.size () != 0 && taskDescription != curTaskDescription)
	{
		curTaskDescription = taskDescription;
		somethingChanged = true;
	}
	if (curTotalFiles != totalFiles)
	{
		curTotalFiles = totalFiles;
		somethingChanged = true;
	}
	if (curInstalledFiles != installedFiles)
	{
		curInstalledFiles = installedFiles;
		somethingChanged = true;
	}
	if (somethingChanged)
		progressChanged = true;

	if (mutex != 0)
		SDL_UnlockMutex (mutex);
}

//-------------------------------------------------------------
void showMessage (std::string msgText, std::string informativeText)
{
	NSAlert* alert = [NSAlert new];
	[alert setMessageText: [NSString stringWithUTF8String:msgText.c_str()]];
	[alert setInformativeText: [NSString stringWithUTF8String:informativeText.c_str()]];
	[alert runModal];
	[alert release];	
}

//-------------------------------------------------------------
void showIntroductionMAC (std::string strText)
{
	showMessage ("Welcome to the M.A.X.R. resource installer.", strText);
}
