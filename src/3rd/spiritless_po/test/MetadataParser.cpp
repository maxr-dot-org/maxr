/*
  Copyright © 2022 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch.hpp>
#include <string>

#include "spiritless_po/MetadataParser.h"

using namespace std;
using namespace spiritless_po;


const string test_data = R"(Project-Id-Version: test-data
Report-Msgid-Bugs-To: https://github.com/oo13/spiritless_po/issues
POT-Creation-Date: 2022-12-11 12:34+0900
PO-Revision-Date: 2022-12-11 19:87+0000
Last-Translator: tester <tester@example.com>
Language-Team: Japanese
Language: ja
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=1; plural=0;
X-Revision: 1.1
)";

TEST_CASE( "Normal Metadata", "[MetadataParser]" ) {
    auto metadata = MetadataParser::Parse(test_data);
    REQUIRE( metadata.size() == 12 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data" );
    REQUIRE( metadata["Report-Msgid-Bugs-To"] ==  "https://github.com/oo13/spiritless_po/issues" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 12:34+0900" );
    REQUIRE( metadata["PO-Revision-Date"] ==  "2022-12-11 19:87+0000" );
    REQUIRE( metadata["Last-Translator"] ==  "tester <tester@example.com>" );
    REQUIRE( metadata["Language-Team"] ==  "Japanese" );
    REQUIRE( metadata["Language"] ==  "ja" );
    REQUIRE( metadata["MIME-Version"] ==  "1.0" );
    REQUIRE( metadata["Content-Type"] ==  "text/plain; charset=UTF-8" );
    REQUIRE( metadata["Content-Transfer-Encoding"] ==  "8bit" );
    REQUIRE( metadata["Plural-Forms"] ==  "nplurals=1; plural=0;" );
    REQUIRE( metadata["X-Revision"] ==  "1.1" );
}


const string test_data_no_NL = R"(Project-Id-Version: test-data no NL
Report-Msgid-Bugs-To: /dev/null)";

TEST_CASE( "No NL at the end in Metadata", "[MetadataParser]" ) {
    auto metadata = MetadataParser::Parse(test_data_no_NL);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data no NL" );
    REQUIRE( metadata["Report-Msgid-Bugs-To"] ==  "/dev/null" );
}


const string test_data_spaces = R"(Project-Id-Version: test-data spaces
POT-Creation-Date : 2022-12-11 12:34+0900
PO-Revision-Date:   2022-12-11 19:87+0000
Last-Translator: tester <tester@example.com>   
Language-Team: Japanese )";

TEST_CASE( "Spaces in Metadata", "[MetadataParser]" ) {
    auto metadata = MetadataParser::Parse(test_data_spaces);
    REQUIRE( metadata.size() == 5 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data spaces" );
    REQUIRE( metadata["POT-Creation-Date "] ==  "2022-12-11 12:34+0900" );
    REQUIRE( metadata["PO-Revision-Date"] ==  "2022-12-11 19:87+0000" );
    REQUIRE( metadata["Last-Translator"] ==  "tester <tester@example.com>   " );
    REQUIRE( metadata["Language-Team"] ==  "Japanese " );
}


const string test_data_duplicate = R"(Project-Id-Version: test-data duplicate
POT-Creation-Date: 2022-12-11 12:34+0900
POT-Creation-Date: 2022-12-11 19:87+0000
)";

TEST_CASE( "Duplicate keys in Metadata", "[MetadataParser]" ) {
    auto metadata = MetadataParser::Parse(test_data_duplicate);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data duplicate" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 12:34+0900" );
}


const string test_data_empty = R"(Project-Id-Version: test-data empty
: null value
POT-Creation-Date: 2022-12-11 19:87+0000
)";

TEST_CASE( "Empty keys in Metadata", "[MetadataParser]" ) {
    auto metadata = MetadataParser::Parse(test_data_empty);
    REQUIRE( metadata.size() == 2 );
    REQUIRE( metadata["Project-Id-Version"] ==  "test-data empty" );
    REQUIRE( metadata["POT-Creation-Date"] ==  "2022-12-11 19:87+0000" );
}
