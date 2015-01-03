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

#ifndef utility_signal_novariadic_signal_3H
#define utility_signal_novariadic_signal_3H

template<typename R, typename Arg1, typename Arg2, typename Arg3, typename MutexType, typename ResultCombinerType>
class cSignal<R (Arg1, Arg2, Arg3), MutexType, ResultCombinerType> : public cSignalBase
{
	typedef cSlot<R (Arg1, Arg2, Arg3)> SlotType;
	typedef std::list<SlotType> SlotsContainerType;
	typedef std::tuple<Arg1, Arg2, Arg3> ArgumentsContainerType;
	typedef sSignalCallIterator<R, ArgumentsContainerType, typename SlotsContainerType::const_iterator> CallIteratorType;

public:
	typedef typename ResultCombinerType::result_type result_type;

	cSignal() :
		nextIdentifer (0),
		isInvoking (false)
	{
		thisReference = std::make_shared<cSignalReference> (*this);
	}

	template<typename F>
	cSignalConnection connect (F&& f)
	{
		cLockGuard<cMutex> lock (mutex);

		cSignalConnection connection (nextIdentifer++, std::weak_ptr<cSignalReference> (thisReference));
		assert (nextIdentifer < std::numeric_limits<unsigned int>::max());

		assert (!isInvoking); // FIXME: can lead to endless loop! fix this and remove the assert

		auto slotFunction = typename SlotType::function_type (std::forward<F> (f));
		slots.emplace_back (connection, std::move (slotFunction));

		return connection;
	}

	virtual void disconnect (const cSignalConnection& connection) MAXR_OVERRIDE_FUNCTION
	{
		cLockGuard<cMutex> lock (mutex);

		for (auto& slot : slots)
		{
			if (slot.connection == connection)
			{
				slot.disconnected = true;
			}
		}
	}

	template<typename Args21, typename Args22, typename Args23>
	result_type operator() (Args21&& arg1, Args22&& arg2, Args23&& arg3)
	{
		cLockGuard<cMutex> lock (mutex);

		auto arguments = ArgumentsContainerType (std::forward<Args21> (arg1), std::forward<Args22> (arg2), std::forward<Args23> (arg3));

		auto wasInvoking = isInvoking;
		isInvoking = true;
		auto resetter = makeScopedOperation ([&]() { isInvoking = wasInvoking; cleanUpConnections(); });

		CallIteratorType begin (arguments, slots.begin(), slots.end());
		CallIteratorType end (arguments, slots.end(), slots.end());

		return ResultCombinerType() (begin, end);
	}
private:
	cSignal (const cSignal& other) MAXR_DELETE_FUNCTION;
	cSignal& operator= (const cSignal& other) MAXR_DELETE_FUNCTION;

	SlotsContainerType slots;

	unsigned long long nextIdentifer;

	bool isInvoking;

	std::shared_ptr<cSignalReference> thisReference;

	MutexType mutex;

	void cleanUpConnections()
	{
		if (isInvoking) return; // it is not safe to clean up yet

		for (auto i = slots.begin(); i != slots.end();)
		{
			if (i->disconnected)
			{
				i = slots.erase (i);
			}
			else
			{
				++i;
			}
		}
	}
};

#endif // utility_signal_novariadic_signal_3H
