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

#ifndef utility_scopedoperationH
#define utility_scopedoperationH

#include <functional>
#include <utility>

/**
 * Generic RAII-class that calls a function object on its destruction.
 *
 * @tparam FunctionType The type of the function to be executed.
 *                      Can be any callable object (including lambdas).
 */
template <typename FunctionType = std::function<void()>>
class cScopedOperation
{
public:
	explicit cScopedOperation (const FunctionType& function_) :
		function (function_)
	{}
	cScopedOperation (const cScopedOperation<FunctionType>&) = delete;
	cScopedOperation& operator= (const cScopedOperation<FunctionType>&) = delete;
	cScopedOperation (cScopedOperation<FunctionType>&& other) :
		function (std::move (other.function)),
		dismissed (std::exchange (other.dismissed, true))
	{
	}
	cScopedOperation<FunctionType>& operator= (cScopedOperation<FunctionType>&& other)
	{
		function = std::move (other.function);
		dismissed = std::exchange (other.dismissed, true);
		return *this;
	}

	~cScopedOperation()
	{
		if (!dismissed) function();
	}

	/**
	 * Dismisses the scoped operations so that it will not call the function
	 * when it is destroyed.
	 */
	void dismiss()
	{
		dismissed = true;
	}

private:
	FunctionType function;
	bool dismissed = false;
};

/**
 * Helper function to create scoped operation objects.
 *
 * @tparam FunctionType Type of the function to create the scoped operation for.
 * @param function The callable object to bind to the scoped operation.
 * @return The scoped operation that will call the passed function object on its destruction.
 */
template <typename FunctionType>
cScopedOperation<FunctionType> makeScopedOperation (const FunctionType& function)
{
	return cScopedOperation<FunctionType> (function);
}

#endif // utility_scopedoperationH
