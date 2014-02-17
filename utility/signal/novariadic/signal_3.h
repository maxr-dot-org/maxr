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

template<typename R, typename Arg1, typename Arg2, typename Arg3>
class cSignal<R(Arg1, Arg2, Arg3)> : public cSignalBase
{
	static_assert(std::is_same<R, void>::value, "Only functions returning void are allowed!");

	typedef std::function<R(Arg1, Arg2, Arg3)> StoredFunctionType;
public:
	cSignal() :
		nextIdentifer(0)
	{}

	template<typename F>
	cSignalConnection connect(F&& f)
	{
		cSignalConnection connection(nextIdentifer++, *this);

		auto func = StoredFunctionType(std::forward<F>(f));
		slots.push_back(std::make_pair(connection, func));

		return connection;
	}

	template<typename F>
	void disconnect(const F& f)
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

		for(auto slot = slots.begin(); slot < slots.end();)
		{
			bool erase = false;

			test_type* target = slot->second.target<test_type>();
			if(target != nullptr)
			{
				auto& t1 = conditionalDeref(target, should_deref());
				auto& t2 = conditionalDeref(&f, should_deref());
				erase = (*t1 == *t2);
			}

			if(erase)
			{
				slot = slots.erase(slot);
			}
			else
			{
				++slot;
			}
		}
	}

	virtual void disconnect(const cSignalConnection& connection) MAXR_OVERRIDE_FUNCTION
	{
		for(auto slot = slots.begin(); slot < slots.end();)
		{
			if(slot->first == connection)
			{
				slot = slots.erase(slot);
			}
			else
			{
				++slot;
			}
		}
	}

	void operator()(Arg1 arg1, Arg2 arg2, Arg3 arg3)
	{
		for(size_t i = 0; i < slots.size(); ++i)
		{
			slots[i].second(arg1, arg2, arg3);
		}
	}
private:
	std::vector<std::pair<cSignalConnection, StoredFunctionType>> slots;

	unsigned int nextIdentifer;
};

#endif // utility_signal_novariadic_signal_3H
