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

#include <UnitTest++/UnitTest++.h>

#include "ui/graphical/game/control/chatcommand/chatcommand.h"
#include "ui/graphical/game/control/chatcommand/chatcommandarguments.h"
#include "ui/graphical/game/control/chatcommand/chatcommandexecutor.h"

#include <map>
#include <string>
#include <vector>

namespace
{
	const std::string name = "test";
	const std::string desc= "dummy desc";

	TEST(ChatCommandFixtureNoArgs)
	{
		std::size_t test_counter = 0;

		auto test = cChatCommand(name, desc).setAction([&]() {++test_counter;});
		REQUIRE CHECK_EQUAL(0, test_counter);

		CHECK_EQUAL(name, test->getCommand().getName());
		CHECK_EQUAL(desc, test->getCommand().getDescription());

		CHECK(!test->tryExecute("/" + name + "2"));
		CHECK_EQUAL(0, test_counter);
		CHECK_THROW(!test->tryExecute("/" + name + " a"), std::runtime_error);
		CHECK_EQUAL(0, test_counter);

		CHECK(test->tryExecute("/" + name));
		CHECK_EQUAL(1, test_counter);
	}

	TEST(ChatCommand_Bool)
	{
		std::map<std::string, std::size_t> map_counter;
		auto action = [&](bool b) {++map_counter[b ? "on" : "off"];};

		auto test = cChatCommand(name, desc).addArgument<cChatCommandArgumentBool>().setAction(action);
		REQUIRE CHECK(map_counter.empty());

		CHECK(!test->tryExecute("/" + name + "2"));
		CHECK(map_counter.empty());
		CHECK_THROW(!test->tryExecute("/" + name), std::runtime_error);
		CHECK(map_counter.empty());

		for (const auto& s : {"off", "on"}) {
			CHECK(test->tryExecute("/" + name + " " + s));
			CHECK_EQUAL(1, map_counter[s]);
		}
	}

	TEST(ChatCommand_Choice)
	{
		const std::string name = "test";
		const std::string desc= "dummy desc";
		const std::vector<std::string> choices{"a", "b", "c"};
		std::map<std::string, std::size_t> map_counter;
		auto action = [&](const std::string& s) {++map_counter[s];};

		auto test = cChatCommand(name, desc).addArgument<cChatCommandArgumentChoice>(choices).setAction(action);
		REQUIRE CHECK(map_counter.empty());

		CHECK(!test->tryExecute("/" + name + "2"));
		CHECK(map_counter.empty());
		CHECK_THROW(!test->tryExecute("/" + name), std::runtime_error);
		CHECK(map_counter.empty());

		for (const auto& s : choices) {
			CHECK(test->tryExecute("/" + name + " " + s));
			CHECK_EQUAL(1, map_counter[s]);
		}
	}

}
