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

#ifndef utility_signal_signalconnectionH
#define utility_signal_signalconnectionH

#include <memory>

class cSignalReference;

/**
 * A signal connection that can be used to disconnect an
 * established connection.
 */
class cSignalConnection
{
public:
	template <typename F, typename M>
	friend class cSignal;

	/**
	 * Returns whether this two signal connection object to reference
	 * the same signal-slot-connection.
	 *
	 * If one of the signals of the signal connections involved in the comparison
	 * does not live anymore this function will always return false.
	 */
	bool operator== (const cSignalConnection& other) const;

	/**
	 * Disconnects the connection in the signal that created this connection.
	 *
	 * If the signal that created this connection is not alive anymore this function just
	 * does nothing.
	 */
	void disconnect();

private:
	cSignalConnection (unsigned long long identifier_, std::weak_ptr<cSignalReference>& signalReference);

	unsigned long long identifier;
	std::weak_ptr<cSignalReference> signalReference;
};

#endif // utility_signal_signalconnectionH
