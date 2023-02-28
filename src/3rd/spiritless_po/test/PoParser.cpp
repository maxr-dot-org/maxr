/*
  Copyright © 2022 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch.hpp>
#include <iostream>
#include <string>

#include "spiritless_po/PoParser.h"

using namespace std;
using namespace spiritless_po;


bool equal(const PoParser::PoEntryT &a, const PoParser::PoEntryT &b)
{
    return a.msgid == b.msgid && a.msgstr == b.msgstr && a.error == b.error;
}

PoParser::PoEntryT create(const string &msgid, const vector<string> &msgstr, const string &error)
{
    PoParser::PoEntryT a;
    a.msgid = msgid;
    a.msgstr = msgstr;
    a.error = error;
    return a;
}

void dump_PO_entry(const vector<PoParser::PoEntryT> &entries)
{
    size_t n = 0;
    for (auto &ent : entries) {
        cout << n << ":\n";
        auto pos = ent.msgid.find("\04");
        if (pos == ent.msgid.npos) {
            cout << "  msgid: \"" << ent.msgid << "\"\n";
        } else {
            cout << "  msgctxt: \"" << ent.msgid.substr(0, pos) << "\"\n";
            cout << "  msgid: \"" << ent.msgid.substr(pos + 1) << "\"\n";
        }
        size_t pn = 0;
        for (auto &s : ent.msgstr) {
            cout << "  msgstr[" << pn << "]: \"" << s << "\"\n";
            ++pn;
        }
        cout << "  error: \"" << ent.error << "\"\n";
        ++n;
    }
}


const string test_data = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data\n"

msgid "apples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"

msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"

msgid "apples"
msgstr "Apples"
)";

TEST_CASE( "Normal PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data.begin(), test_data.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 7 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("", { "Project-Id-Version: test-data\n" }, "")) );
        REQUIRE( equal(entries[1], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[2], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[3], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[4], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[5], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
        REQUIRE( equal(entries[6], create("apples", { "Apples" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
        REQUIRE( entries[5].error.size() == 0 );
        REQUIRE( entries[6].error.size() == 0 );
    }
}


const string test_data_fuzzy = R"(
# fuzzy
msgid "apples"
msgstr "APPLES"

# comment
#, fuzzy
# comment
msgid "bananas"
msgstr "BANANAS"

# comment
#, fuzzy, c-format
# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

# comment
#, c-format, fuzzy
# comment
msgctxt "food"
msgid "eggs"
msgstr "EGGS"
)";

TEST_CASE( "Fuzzy PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_fuzzy.begin(), test_data_fuzzy.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 4 );
    }
    SECTION( "fuzzy msgstr is empty" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
    }
}


const string test_data_comment = R"(
# comment

msgid "apples"
msgstr "APPLES"

# comment

#, c-format
msgid "bananas"
msgstr "BANANAS"

# comment
msgid "corn"
msgid_plural "corns"
msgstr[0] "CORN#0"
msgstr[1] "CORN#1"
msgstr[2] "CORN#2"
msgstr[3] "CORN#3"

#, fuzzy


msgctxt "food"
msgid "eggs"
msgstr "EGGS"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "GARLIC#0"
msgstr[1] "GARLIC#1"
msgstr[2] "GARLIC#2"
msgstr[3] "GARLIC#3"
msgstr[4] "GARLIC#4"
# comment

#~ msgid "aaa"
)";

TEST_CASE( "Comment in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_comment.begin(), test_data_comment.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 5 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
        REQUIRE( equal(entries[2], create("corn", { "CORN#0", "CORN#1", "CORN#2", "CORN#3" }, "")) );
        REQUIRE( equal(entries[3], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[4], create("food\04garlic", { "GARLIC#0", "GARLIC#1", "GARLIC#2", "GARLIC#3", "GARLIC#4" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() == 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
    }
}


const string test_data_eof_wo_NL_1 = R"(
msgid "apples"
msgstr "APPLES")";

TEST_CASE( "EOF without NL in PO Entries (1/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_wo_NL_1.begin(), test_data_eof_wo_NL_1.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 1 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
    }
}


const string test_data_eof_wo_NL_2 = R"(
msgid "apples"
msgstr "APPLES"

msgid "bananas"
msgstr "BANANAS"
#~ comment)";

TEST_CASE( "EOF without NL in PO Entries (2/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_eof_wo_NL_2.begin(), test_data_eof_wo_NL_2.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 2 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS" }, "")) );
    }
    SECTION( "no errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
    }
}


const string test_data_empty = R"(
# empty
#, empty
# empty
)";

TEST_CASE( "Empty stream in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_empty.begin(), test_data_empty.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 0 );
    }
}


const string test_data_errors_1 = R"(
msgstr "APPLES"

msgid_plural "corns"

msgstr[0] "CORNS#0"
)";

TEST_CASE( "Errors in PO Entries (1/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_errors_1.begin(), test_data_errors_1.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 3 );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() > 0 );
        REQUIRE( entries[2].error.size() > 0 );
    }
}


const string test_data_errors_2 = R"(
msgid "apples"
msgctxt "food"
msgstr "APPLES"

msgid "bananas"
msgstr[0] "BANANAS"

msgid_plural "corns"
msgid "corn"
msgstr[0] "CORNS#0"

msgid "hops"

msgctxt "food"
msgstr "Apples"

msgid "garlic"
msgid_plural "garlics"
msgstr "GARLIC#0"

msgid "eggs")";

TEST_CASE( "Errors in PO Entries (2/2)", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_errors_2.begin(), test_data_errors_2.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 8 );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() > 0 );
        REQUIRE( entries[1].error.size() > 0 );
        REQUIRE( entries[2].error.size() > 0 );
        REQUIRE( entries[3].error.size() > 0 );
        REQUIRE( entries[4].error.size() > 0 );
        REQUIRE( entries[5].error.size() > 0 );
        REQUIRE( entries[6].error.size() > 0 );
        REQUIRE( entries[7].error.size() > 0 );
    }
}


const string test_data_multi_line_string = R"(
msgid "apples"
msgstr ""
"APPLES"

msgid "bananas"
msgstr "BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"
"BANANAS\n"

"BANANAS"

msgid ""
"corn"
msgid_plural "corns\n"
"corns"
msgstr[0] ""
"CORNS#0"
msgstr[1] "CORNS#1\n"
"CORNS#1"

msgid ""
"garlics\n"
"garlics\n"
"garlics"
msgstr ""
"GARLICS\n"
"GARLICS"

msgctxt ""
"food"
msgid "eggs"
msgstr "EGGS"

msgctxt ""
"food\n"
"food"
msgid "hops"
msgstr "HOPS"
)";

TEST_CASE( "Multi line strings in PO Entries", "[PoParser]" ) {
    auto entries = PoParser::GetEntries(test_data_multi_line_string.begin(), test_data_multi_line_string.end());
    SECTION( "size" ) {
        REQUIRE( entries.size() == 7 );
    }
    SECTION( "entries" ) {
        REQUIRE( equal(entries[0], create("apples", { "APPLES" }, "")) );
        REQUIRE( equal(entries[1], create("bananas", { "BANANAS\nBANANAS\nBANANAS\nBANANAS\nBANANAS\n" }, "")) );
        REQUIRE( equal(entries[3], create("corn", { "CORNS#0", "CORNS#1\nCORNS#1" }, "")) );
        REQUIRE( equal(entries[4], create("garlics\ngarlics\ngarlics", { "GARLICS\nGARLICS" }, "")) );
        REQUIRE( equal(entries[5], create("food\04eggs", { "EGGS" }, "")) );
        REQUIRE( equal(entries[6], create("food\nfood\04hops", { "HOPS" }, "")) );
    }
    SECTION( "errors" ) {
        REQUIRE( entries[0].error.size() == 0 );
        REQUIRE( entries[1].error.size() == 0 );
        REQUIRE( entries[2].error.size() > 0 );
        REQUIRE( entries[3].error.size() == 0 );
        REQUIRE( entries[4].error.size() == 0 );
        REQUIRE( entries[5].error.size() == 0 );
        REQUIRE( entries[6].error.size() == 0 );
    }
}
