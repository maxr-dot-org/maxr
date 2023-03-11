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

#ifndef CRASHREPORTER_DEBUG_H
#define CRASHREPORTER_DEBUG_H

#ifdef USE_CRASH_RPT

# include <memory>

class CR_RPT_RAII
{
public:
	CR_RPT_RAII();
	~CR_RPT_RAII() = default;

	CR_RPT_RAII (const CR_RPT_RAII&) = delete;
	CR_RPT_RAII& operator= (const CR_RPT_RAII&) = delete;

private:
	std::shared_ptr<void> pimpl;
};

# define CR_ENABLE_CRASH_RPT_CURRENT_THREAD() CR_RPT_RAII cr_rpt_raii
void CR_EMULATE_CRASH();
void CR_INIT_CRASHREPORTING();

#else

inline void CR_ENABLE_CRASH_RPT_CURRENT_THREAD() {}
inline void CR_EMULATE_CRASH() {}
inline void CR_INIT_CRASHREPORTING() {}

#endif

#endif
