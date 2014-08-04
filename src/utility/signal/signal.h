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

#ifndef utility_signal_signalH
#define utility_signal_signalH

#include <list>
#include <tuple>
#include <utility>
#include <cassert>

#include "maxrconfig.h"
#include "utility/dependentfalse.h"
#include "utility/deref.h"
#include "utility/scopedoperation.h"
#include "utility/functiontraits.h"

#include "utility/signal/signalconnection.h"
#include "utility/signal/signalresultcombinerlast.h"
#include "utility/signal/slot.h"
#include "utility/signal/signalcalliterator.h"

/**
 * Basic signal class. This class should never be instantiated directly.
 * It has a specialization for function types.
 *
 * @tparam F Should be a function type.
 * @tparam C The result combiner.
 */
template<typename F, typename C = sSignalResultCombinerLast<typename sFunctionTraits<F>::result_type>>
class cSignal
{
	static_assert(sDependentFalse<F>::value, "cSignal not allowed with this template arguments!");
};

/**
 * Base class interface of signals used for type erasure and adds
 * the possibility to disconnect connections without knowing their specific type.
 */
class cSignalBase
{
public:
	virtual ~cSignalBase () {}
	virtual void disconnect (const cSignalConnection& connection) = 0;
};

class cSignalReference
{
public:
	cSignalReference (cSignalBase& signal_) :
		signal (signal_)
	{}

	cSignalBase& getSignal ()
	{
		return signal;
	}

	bool operator==(const cSignalReference& other) const
	{
		return &signal == &other.signal;
	}
private:
	cSignalBase& signal;
};

#if MAXR_NO_VARIADIC_TEMPLATES

#include "utility/signal/novariadic/signal_0.h"
#include "utility/signal/novariadic/signal_1.h"
#include "utility/signal/novariadic/signal_2.h"
#include "utility/signal/novariadic/signal_3.h"
#include "utility/signal/novariadic/signal_4.h"

#else

#include <limits>

/**
 * Generic signal.
 *
 * A signal object provides the possibility to connect a specific type
 * of function to it.
 * Those connected functions will be remembered by the signal object.
 *
 * When the signal object is invoked later on, it calls all the connected
 * functions with the arguments that have be passed to the signal invoke
 * command.
 *
 * The signal object does not get the ownership of the connected functions
 * and hence the user of the signal has to make sure that all connected
 * functions are outliving the signal object.
 *
 * @tparam R The return value of the signal.
 * @tparam ...Args The arguments of the signal function.
 * @tparam ResultCombinerType
 */
template<typename R, typename... Args, typename ResultCombinerType>
class cSignal<R (Args...), ResultCombinerType> : public cSignalBase
{
	typedef cSlot<R (Args...)> SlotType;
	typedef std::list<SlotType> SlotsContainerType;
	typedef std::tuple<Args...> ArgumentsContainerType;
	typedef sSignalCallIterator<R, ArgumentsContainerType, typename SlotsContainerType::const_iterator> CallIteratorType;

public:
	typedef typename ResultCombinerType::result_type result_type;

	cSignal ();

	/**
	 * Connects a new function to the signal.
	 *
	 * @tparam F Type of the function to connect.
	 *           Can be a function pointer on any function object (including lambdas).
	 *           The function signature has to match the one of the signal.
	 * @param f The function object to add a connection to.
	 * @return A connection object.
	 *         This connection object can be used to disconnect the function, even when
	 *         the original function object is not available any longer.
	 *         The user has to make sure the signal object outlives all the connection objects
	 *         that are created by the signal.
	 */
	template<typename F>
	cSignalConnection connect (F&& f);

	/**
	 * Disconnects a function object that has been connected.
	 *
	 * If the passed function object does not match any connected function
	 * it will just be ignored.
	 *
	 * @tparam F Type of the function. Has to match the type of the stored function.
	 * @param f The function to disconnect.
	 */
	template<typename F>
	void disconnect (const F& f);

	/**
	 * Disconnect a function by the connection object that has been returned
	 * when the connection was established.
	 *
	 * If the connection object does not belong to this signal, or the connection has been
	 * disconnected already this function will just do nothing.
	 */
	virtual void disconnect (const cSignalConnection& connection) MAXR_OVERRIDE_FUNCTION;

	/**
	 * Invokes the signal.
	 *
	 * This will call all the connected functions.
	 *
	 * @param ...args The arguments to call the functions with.
	 */
	template<typename... Args2>
	result_type operator()(Args2&&... args);
private:
	cSignal (const cSignal& other) MAXR_DELETE_FUNCTION;
	cSignal& operator=(const cSignal& other) MAXR_DELETE_FUNCTION;

	SlotsContainerType slots;

	// NOTE: may could be implemented as kind of "identifier pool" but it seems kind of
	//       overkill here for me since I don't think we will ever create as many
	//       connections as an integer can represent numbers.
	unsigned int nextIdentifer;

	bool isInvoking;

	void cleanUpConnections ();

	std::shared_ptr<cSignalReference> thisReference;
};

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
cSignal<R (Args...), ResultCombinerType>::cSignal () :
	nextIdentifer (0),
	isInvoking (false)
{
	thisReference = std::make_shared<cSignalReference> (*this);
}

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
template<typename F>
cSignalConnection cSignal<R (Args...), ResultCombinerType>::connect (F&& f)
{
	std::weak_ptr<cSignalReference> weakSignalRef(thisReference);
	cSignalConnection connection (nextIdentifer++, weakSignalRef);
	assert (nextIdentifer < std::numeric_limits<unsigned int>::max ());

	assert (!isInvoking); // FIXME: can lead to endless loop! fix this and remove the assert

	auto slotFunction = typename SlotType::function_type (std::forward<F> (f));
	slots.emplace_back (connection, std::move (slotFunction));

    return connection;
}

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
template<typename F>
void cSignal<R (Args...), ResultCombinerType>::disconnect (const F& f)
{
	typedef typename std::conditional<std::is_function<F>::value, typename std::add_pointer<F>::type, F>::type test_type;

	typedef typename std::conditional
	<
		std::is_pointer<test_type>::value,
		typename std::conditional
		<
			std::is_function<typename std::remove_pointer<test_type>::type>::value,
			std::true_type,
			std::false_type
		>::type,
		std::false_type
	>::type should_deref;

	for (auto& slot : slots)
	{
		// NOTE: This may depend on the concrete implementation of std::function
		//       and therefor is not platform independent.
		//       This has to be rechecked with the C++ standard. If it is not
		//       platform independent we may choose to discard the disconnection
		//       by the original function object and just allow disconnection by
		//       the connection objects.
		test_type* target = slot.function.template target<test_type> ();
		if (target != nullptr)
		{
			auto& t1 = conditionalDeref (target, should_deref ());
			auto& t2 = conditionalDeref (&f, should_deref ());
			if (*t1 == *t2)
			{
				slot.disconnected = true;
			}
		}
	}
}

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
void cSignal<R (Args...), ResultCombinerType>::disconnect (const cSignalConnection& connection)
{
	for (auto& slot : slots)
	{
		if (slot.connection == connection)
		{
			slot.disconnected = true;
		}
	}

	if (!isInvoking) cleanUpConnections ();
}

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
template<typename... Args2>
typename cSignal<R (Args...), ResultCombinerType>::result_type cSignal<R (Args...), ResultCombinerType>::operator()(Args2&&... args)
{
	cleanUpConnections ();

	auto arguments = ArgumentsContainerType(std::forward<Args2>(args)...);

	auto wasInvoking = isInvoking;
	isInvoking = true;
	auto resetter = makeScopedOperation ([&](){ isInvoking = wasInvoking; });

	CallIteratorType begin (arguments, slots.begin (), slots.end ());
	CallIteratorType end (arguments, slots.end (), slots.end ());

	if (slots.begin () != slots.end () && slots.begin ()->disconnected) ++begin; // skips all disconnected slots in the beginning

	return ResultCombinerType () (begin, end);
}

//------------------------------------------------------------------------------
template<typename R, typename... Args, typename ResultCombinerType>
void cSignal<R (Args...), ResultCombinerType>::cleanUpConnections ()
{
	if (isInvoking) return; // it is not safe to clean up yet

	for (auto i = slots.begin (); i != slots.end ();)
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

#endif // MAXR_NO_VARIADIC_TEMPLATES

#endif // utility_signal_signalH
