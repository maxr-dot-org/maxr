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

#include "chatcommand/chatcommand.h"
#include "chatcommand/chatcommandarguments.h"
#include "chatcommand/chatcommandexecutor.h"

#include <3rd/doctest/doctest.h>
#include <map>
#include <string>
#include <vector>

namespace
{
	const std::string name = "test";
	const std::string desc = "dummy desc";

	TEST_CASE ("ChatCommandFixtureNoArgs")
	{
		std::size_t test_counter = 0;

		auto test = cChatCommand (name, desc).setAction ([&]() { ++test_counter; });
		REQUIRE (0 == test_counter);

		CHECK (name == test->getCommand().getName());
		CHECK (desc == test->getCommand().getDescription());

		CHECK (!test->tryExecute ("/" + name + "2"));
		CHECK (0 == test_counter);
		CHECK_THROWS_AS (test->tryExecute ("/" + name + " a"), std::runtime_error);
		CHECK (0 == test_counter);

		CHECK (test->tryExecute ("/" + name));
		CHECK (1 == test_counter);
	}

	TEST_CASE ("ChatCommand_Bool")
	{
		std::map<std::string, std::size_t> map_counter;
		auto action = [&] (bool b) { ++map_counter[b ? "on" : "off"]; };

		auto test = cChatCommand (name, desc).addArgument<cChatCommandArgumentBool>().setAction (action);
		REQUIRE (map_counter.empty());

		CHECK (!test->tryExecute ("/" + name + "2"));
		CHECK (map_counter.empty());
		CHECK_THROWS_AS (test->tryExecute ("/" + name), std::runtime_error);
		CHECK (map_counter.empty());

		for (const auto& s : {"off", "on"})
		{
			CHECK (test->tryExecute ("/" + name + " " + s));
			CHECK (1 == map_counter[s]);
		}
	}

	TEST_CASE ("ChatCommand_Choice")
	{
		const std::vector<std::string> choices{"a", "b", "c"};
		std::map<std::string, std::size_t> map_counter;
		auto action = [&] (const std::string& s) { ++map_counter[s]; };

		auto test = cChatCommand (name, desc).addArgument<cChatCommandArgumentChoice> (choices).setAction (action);
		REQUIRE (map_counter.empty());

		CHECK (!test->tryExecute ("/" + name + "2"));
		CHECK (map_counter.empty());
		CHECK_THROWS_AS (test->tryExecute ("/" + name), std::runtime_error);
		CHECK (map_counter.empty());

		for (const auto& s : choices)
		{
			CHECK (test->tryExecute ("/" + name + " " + s));
			CHECK (1 == map_counter[s]);
		}
	}

} // namespace
