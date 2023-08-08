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

#ifndef utility_signal_signalconnectionmanagerH
#define utility_signal_signalconnectionmanagerH

#include "utility/signal/signalconnection.h"

#include <vector>

/**
 * A RAII signal connection manager class that can be used to bind
 * the lifetime of connections to the lifetime of this connection manager.
 *
 * All connections that are established through this connection manager
 * will automatically disconnected when the connection manager will
 * be destroyed.
 */
class cSignalConnectionManager
{
public:
	cSignalConnectionManager() = default;
	cSignalConnectionManager (const cSignalConnectionManager&) = delete;
	cSignalConnectionManager& operator= (const cSignalConnectionManager&) = delete;
	cSignalConnectionManager (cSignalConnectionManager&&) = default;
	cSignalConnectionManager& operator= (cSignalConnectionManager&&) = default;
	~cSignalConnectionManager();

	/**
	 * Connects the passed function object to the passed signal and
	 * stores the connection within this connection manager so that
	 * it will be disconnected when the connection manager gets destroyed.
	 *
	 * @tparam SignalType The type of the signal.
	 * @tparam FunctionType The type of the function.
	 * @param signal The signal to connect the function to.
	 * @param function The callable object to connect to the signal.
	 */
	template <typename SignalType, typename FunctionType>
	auto connect (SignalType& signal, FunctionType&& function)
		-> decltype (signal.connect (std::forward<FunctionType> (function)));

	/**
	 * Disconnects a single connection and removes it from this manager.
	 *
	 * @param connection The connection to disconnect.
	 *                   Should be a connection that has been returned by the @ref connect
	 *                   method of this class.
	 *                   Any other connections will be ignored.
	 * @return true if the connection could be found in the stored connections and
	 *         hence has been disconnected.
	 */
	bool disconnect (cSignalConnection& connection);

	/**
	 * Disconnects all the stored connections.
	 */
	void disconnectAll();

private:
	std::vector<cSignalConnection> connections;
};

//------------------------------------------------------------------------------
template <typename SignalType, typename FunctionType>
auto cSignalConnectionManager::connect (SignalType& signal, FunctionType&& function)
	-> decltype (signal.connect (std::forward<FunctionType> (function)))
{
	connections.push_back (signal.connect (std::forward<FunctionType> (function)));
	return connections.back();
}

#endif // utility_signal_signalconnectionmanagerH
