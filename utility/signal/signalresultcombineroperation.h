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

#ifndef utility_signal_signalresultcombinderoperationH
#define utility_signal_signalresultcombinderoperationH

#include "signalresultcombinervoid.h"

template<typename T, typename OperationType>
struct sSignalResultCombinerOperation
{
	typedef T result_type;

	template<typename Iter>
	result_type operator() (Iter begin, Iter end)
	{
		result_type currentValue;
		const OperationType operation;

		bool first = true;
		while (begin != end)
		{
			if (first)
			{
				currentValue = *begin;
				first = false;
			}
			else
			{
				currentValue = operation (currentValue, *begin);
			}
			++begin;
		}
		return currentValue;
	}
};

template<typename OperationType>
struct sSignalResultCombinerOperation<void, OperationType> : public sSignalResultCombinerVoid{};

#endif // utility_signal_signalresultcombinderoperationH