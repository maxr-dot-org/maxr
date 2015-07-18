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

#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef USE_CRASH_RPT

# include <CrashRpt.h>

#define CR_ENABLE_CRASH_RPT_CURRENT_THREAD() CrThreadAutoInstallHelper(0)
#define CR_EMULATE_CRASH() crEmulateCrash(CR_SEH_EXCEPTION)
#define CR_INIT_CRASHREPORTING() initCrashreporting()

void initCrashreporting();

#else

#define CR_ENABLE_CRASH_RPT_CURRENT_THREAD() 
#define CR_EMULATE_CRASH()
#define CR_INIT_CRASHREPORTING()

#endif



#endif //_DEBUG_H