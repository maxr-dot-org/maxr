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

#include "utility/deref.h"
#include "utility/listhelpers.h"
#include "utility/scopedoperation.h"
#include "utility/signal/signalconnection.h"
#include "utility/signal/slot.h"
#include "utility/thread/dummymutex.h"

#include <cassert>
#include <limits>
#include <list>
#include <mutex>
#include <utility>

/**
 * Basic signal class. This class should never be instantiated directly.
 * It has a specialization for function types.
 *
 * @tparam FunctionSignatureType Should be a function signature type.
 * @tparam MutexType Type of the mutex to be used in the signal class.
 *                   Can be a dummy mutex if you do not need this signal to be thread safe
 *                   and/or you can not effort the cost of e.g. creating the mutex.
 *                   If a working mutex is used, it should be a recursive mutex to make sure no
 *                   deadlocks arise when a slot functions tries to access the signal recursively.
 */
template <typename FunctionSignatureType, typename MutexType = cDummyMutex>
class cSignal;

/**
 * Base class interface of signals used for type erasure and adds
 * the possibility to disconnect connections without knowing their specific type.
 */
class cSignalBase
{
public:
	virtual ~cSignalBase() = default;
	virtual void disconnect (const cSignalConnection& connection) = 0;
};

class cSignalReference
{
public:
	explicit cSignalReference (cSignalBase& signal_) :
		signal (signal_)
	{}

	cSignalReference (const cSignalReference&) = delete;
	cSignalReference& operator= (const cSignalReference&) = delete;

	cSignalBase& getSignal()
	{
		return signal;
	}

	bool operator== (const cSignalReference& other) const
	{
		return &signal == &other.signal;
	}

private:
	cSignalBase& signal;
};

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
 * @tparam ...Args The arguments of the signal function.
 */
template <typename... Args, typename MutexType>
class cSignal<void (Args...), MutexType> : public cSignalBase
{
	using SlotType = cSlot<void (Args...)>;
	using SlotsContainerType = std::list<SlotType>;

public:
	cSignal() :
		thisReference (std::make_shared<cSignalReference> (*this)) {}
	cSignal (const cSignal&) = delete;
	cSignal& operator= (const cSignal&) = delete;

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
	template <typename F, std::enable_if_t<std::is_assignable_v<std::function<void (Args...)>, F>, int> = 0>
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
	template <typename F>
	void disconnect (const F& f);

	/**
	 * Disconnect a function by the connection object that has been returned
	 * when the connection was established.
	 *
	 * If the connection object does not belong to this signal, or the connection has been
	 * disconnected already this function will just do nothing.
	 */
	void disconnect (const cSignalConnection& connection) override;

	/**
	 * Invokes the signal.
	 *
	 * This will call all the connected functions.
	 *
	 * @param ...args The arguments to call the functions with.
	 */
	template <typename... Args2, std::enable_if_t<std::is_invocable_v<std::function<void (Args...)>, Args2&&...>, int> = 0>
	void operator() (Args2&&... args);

private:
	void cleanUpConnections();

private:
	SlotsContainerType slots;

	// NOTE: may could be implemented as kind of "identifier pool" but it seems kind of
	//       overkill here for me since I don't think we will ever create as many
	//       connections as an integer can represent numbers.
	unsigned long long nextIdentifer = 0;

	bool isInvoking = false;

	std::shared_ptr<cSignalReference> thisReference;

	// NOTE: is important that this one is a recursive mutex (as e.g the SDL_Mutex or std::recursive_mutex).
	MutexType mutex;
};

//------------------------------------------------------------------------------
template <typename... Args, typename MutexType>
template <typename F, std::enable_if_t<std::is_assignable_v<std::function<void (Args...)>, F>, int>>
cSignalConnection cSignal<void (Args...), MutexType>::connect (F&& f)
{
	std::unique_lock<MutexType> lock (mutex);

	std::weak_ptr<cSignalReference> weakSignalRef (thisReference);
	cSignalConnection connection (nextIdentifer++, weakSignalRef);
	assert (nextIdentifer < std::numeric_limits<unsigned int>::max());

	assert (!isInvoking); // FIXME: can lead to endless loop! fix this and remove the assert

	auto slotFunction = typename SlotType::function_type (std::forward<F> (f));
	slots.emplace_back (connection, std::move (slotFunction));

	return connection;
}

//------------------------------------------------------------------------------
template <typename... Args, typename MutexType>
template <typename F>
void cSignal<void (Args...), MutexType>::disconnect (const F& f)
{
	using test_type = std::conditional_t<std::is_function<F>::value, std::add_pointer_t<F>, F>;

	using should_deref = std::conditional_t<
		std::is_pointer<test_type>::value,
		std::is_function<std::remove_pointer_t<test_type>>,
		std::false_type>;

	std::unique_lock<MutexType> lock (mutex);

	for (auto& slot : slots)
	{
		// NOTE: This may depend on the concrete implementation of std::function
		//       and therefor is not platform independent.
		//       This has to be rechecked with the C++ standard. If it is not
		//       platform independent we may choose to discard the disconnection
		//       by the original function object and just allow disconnection by
		//       the connection objects.
		test_type* target = slot.function.template target<test_type>();
		if (target != nullptr)
		{
			auto& t1 = conditionalDeref (target, should_deref());
			auto& t2 = conditionalDeref (&f, should_deref());
			if (*t1 == *t2)
			{
				slot.disconnected = true;
			}
		}
	}
}

//------------------------------------------------------------------------------
template <typename... Args, typename MutexType>
void cSignal<void (Args...), MutexType>::disconnect (const cSignalConnection& connection)
{
	std::unique_lock<MutexType> lock (mutex);

	for (auto& slot : slots)
	{
		if (slot.connection == connection)
		{
			slot.disconnected = true;
		}
	}

	cleanUpConnections();
}

//------------------------------------------------------------------------------
template <typename... Args, typename MutexType>
template <typename... Args2, std::enable_if_t<std::is_invocable_v<std::function<void (Args...)>, Args2&&...>, int>>
void cSignal<void (Args...), MutexType>::operator() (Args2&&... args)
{
	std::unique_lock<MutexType> lock (mutex);

	auto wasInvoking = isInvoking;
	isInvoking = true;
	auto resetter = makeScopedOperation ([&]() { isInvoking = wasInvoking; this->cleanUpConnections(); });

	for (auto& slot : slots)
	{
		if (slot.disconnected) continue;
		slot.function (args...);
	}
}

//------------------------------------------------------------------------------
template <typename... Args, typename MutexType>
void cSignal<void (Args...), MutexType>::cleanUpConnections()
{
	if (isInvoking) return; // it is not safe to clean up yet

	EraseIf (slots, [] (const auto& slot) { return slot.disconnected; });
}

#endif
