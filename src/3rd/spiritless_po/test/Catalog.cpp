/*
  Copyright © 2022 OOTA, Masato
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "spiritless_po.h"

using namespace std;
using namespace spiritless_po;


bool equal(const Catalog::IndexDataT &a, const Catalog::IndexDataT &b)
{
    return a.stringTableIndex == b.stringTableIndex && a.totalPlurals == b.totalPlurals;
}

bool equal(const unordered_map<string, Catalog::IndexDataT>& a, const unordered_map<string, Catalog::IndexDataT>& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (auto elm : a) {
        auto it2 = b.find(elm.first);
        if (it2 == b.end() || !equal(elm.second, it2->second)) {
            return false;
        }
    }
    return true;
}

void dump_catalog(const Catalog &catalog)
{
    cout << "String Table:" << endl;
    size_t no = 0;
    for (auto s : catalog.GetStringTable()) {
        cout << no << ": \"" << s << "\"\n";
        ++no;
    }
    cout << "Index:" << endl;
    for (auto idx : catalog.GetIndex()) {
        auto base = idx.second.stringTableIndex;
        auto total = idx.second.totalPlurals;
        auto s = idx.first;
        auto pos = s.find("\04");
        cout << '"';
        if (pos == s.npos) {
            cout << s;
        } else {
            cout << s.substr(0, pos) << "\" / \"";
            cout << s.substr(pos + 1);
        }
        cout << "\": " << base;
        if (total == 1) {
            cout << endl;
        } else {
            cout << " - " << (base + total - 1) << endl;
        }
    }
    cout << "Metadata:" << endl;
    for (auto entry : catalog.GetMetadata()) {
        cout << entry.first << ": " << entry.second << endl;
    }
    cout << "Errors:" << endl;
    for (const auto &s : catalog.GetError()) {
        cerr << s << endl;
    }
}



TEST_CASE( "Default Constructor in Catalog", "[Catalog]" ) {
    Catalog catalog;
    string singular("apple");
    string plural("apples");

    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
}


const string test_data = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data\n"
"Plural-Forms: nplurals=5; plural=n;\n"

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

msgid "error"
)";

TEST_CASE( "Constructor(begin, end) in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    string singular("ant");
    string plural("ants");

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 5 );
    REQUIRE( catalog.GetStringTable().size() == 12 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
}


TEST_CASE( "Constructor(is) in Catalog", "[Catalog]" ) {
    istringstream is(test_data);
    Catalog catalog(is);
    string singular("ant");
    string plural("ants");

    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 5 );
    REQUIRE( catalog.GetStringTable().size() == 12 );
    REQUIRE( &catalog.gettext(singular) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 1) == &singular );
    REQUIRE( &catalog.ngettext(singular, plural, 2) == &plural );
    REQUIRE( &catalog.pgettext("context", singular) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 1) == &singular );
    REQUIRE( &catalog.npgettext("context", singular, plural, 2) == &plural );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
}


TEST_CASE( "Copy constructor in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);

    REQUIRE( catalog.GetError() == catalog2.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog2.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog2.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog2.GetStringTable() );
}


TEST_CASE( "Move constructor in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);
    Catalog catalog3(std::move(catalog2));

    REQUIRE( catalog.GetError() == catalog3.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog3.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog3.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog3.GetStringTable() );
}


TEST_CASE( "Assign operator in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2;
    catalog2 = catalog;

    REQUIRE( catalog.GetError() == catalog2.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog2.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog2.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog2.GetStringTable() );
}


TEST_CASE( "Move assign operator in Catalog", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(catalog);
    Catalog catalog3;
    catalog3 = std::move(catalog2);

    REQUIRE( catalog.GetError() == catalog3.GetError() );
    REQUIRE( catalog.GetMetadata() == catalog3.GetMetadata() );
    REQUIRE( equal(catalog.GetIndex(), catalog3.GetIndex()) );
    REQUIRE( catalog.GetStringTable() == catalog3.GetStringTable() );
}


TEST_CASE( "Catalog::Clear()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    REQUIRE( catalog.GetError().size() == 1 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 5 );
    REQUIRE( catalog.GetStringTable().size() == 12 );
    catalog.Clear();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 0 );
    REQUIRE( catalog.GetIndex().size() == 0 );
    REQUIRE( catalog.GetStringTable().size() == 0 );
}


const string test_data_2 = R"(# translator-comments
#. extracted-comment
#: references
#, flags
#| msgid previous-untranslated-string
msgid ""
msgstr "Project-Id-Version: test-data-2\n"
"MIME-Version: 1.0\n"
"Plural-Forms: nplurals=3; plural=(n+1) % 3;\n"

msgid "apples"
msgstr "Apples2"

msgid "bananas"
msgstr "Bananas"

msgid "corn"
msgid_plural "corns"
msgstr[0] "Corn#0"
msgstr[1] "Corn#1"
msgstr[2] "Corn#2"
msgstr[3] "Corn#3"

msgctxt "food"
msgid "eggs"
msgstr "Eggs"

msgctxt "food"
msgid "garlic"
msgid_plural "garlics"
msgstr[0] "Garlic#0"
msgstr[1] "Garlic#1"
msgstr[2] "Garlic#2"
msgstr[3] "Garlic#3"
msgstr[4] "Garlic#4"

msgid "error2"

msgid "mangoes"
msgstr "MANGOES"

msgid "oat"
msgid_plural "oats"
msgstr[0] "OAT#0"
msgstr[1] "OAT#1"
msgstr[2] "OAT#2"

msgctxt "food"
msgid "pickles"
msgstr "PICKLES"

msgid "error3"

msgctxt "food"
msgid "raisin"
msgid_plural "raisins"
msgstr[0] "RAISIN#0"
msgstr[1] "RAISIN#1"
msgstr[2] "RAISIN#2"
)";

TEST_CASE( "Catalog::Add(begin, end)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.Add(test_data_2.begin(), test_data_2.end());

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 9 );
    REQUIRE( catalog.GetStringTable().size() == 20 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::Add(is)", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    istringstream is(test_data_2);
    catalog.Add(is);

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 9 );
    REQUIRE( catalog.GetStringTable().size() == 20 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::Merge()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    Catalog catalog2(test_data_2.begin(), test_data_2.end());
    catalog.Merge(catalog2);

    REQUIRE( catalog.GetError().size() == 3 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 9 );
    REQUIRE( catalog.GetStringTable().size() == 20 );
    REQUIRE( catalog.gettext("apples") == "APPLES" );
    REQUIRE( catalog.ngettext("corn", "corns", 1) == "CORN#1" );
    REQUIRE( catalog.ngettext("corn", "corns", 2) == "CORN#2" );
    REQUIRE( catalog.ngettext("corn", "corns", 4) == "CORN#0" );
    REQUIRE( catalog.pgettext("food", "eggs") == "EGGS" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 1) == "GARLIC#1" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 2) == "GARLIC#2" );
    REQUIRE( catalog.npgettext("food", "garlic", "garlics", 4) == "GARLIC#4" );
    REQUIRE( catalog.gettext("mangoes") == "MANGOES" );
    REQUIRE( catalog.ngettext("oat", "oats", 1) == "OAT#1" );
    REQUIRE( catalog.ngettext("oat", "oats", 2) == "OAT#2" );
    REQUIRE( catalog.ngettext("oat", "oats", 4) == "OAT#0" );
    REQUIRE( catalog.pgettext("food", "pickles") == "PICKLES" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 1) == "RAISIN#1" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 2) == "RAISIN#2" );
    REQUIRE( catalog.npgettext("food", "raisin", "raisins", 4) == "RAISIN#0" );
}


TEST_CASE( "Catalog::ClearError()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.ClearError();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 5 );
    REQUIRE( catalog.GetStringTable().size() == 12 );

    catalog.Add(test_data_2.begin(), test_data_2.end());
    REQUIRE( catalog.GetError().size() == 2 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 9 );
    REQUIRE( catalog.GetStringTable().size() == 20 );

    catalog.ClearError();
    REQUIRE( catalog.GetError().size() == 0 );
    REQUIRE( catalog.GetMetadata().size() == 2 );
    REQUIRE( catalog.GetIndex().size() == 9 );
    REQUIRE( catalog.GetStringTable().size() == 20 );
}


TEST_CASE( "Catalog::GetError()", "[Catalog]" ) {
    Catalog catalog(test_data.begin(), test_data.end());
    catalog.Add(test_data_2.begin(), test_data_2.end());
    vector<string> expected_errors { "40,1: 'msgstr' is expected.", "38,1: 'msgstr' is expected.", "53,1: 'msgstr' is expected." };
    REQUIRE( catalog.GetError() ==  expected_errors );
}
