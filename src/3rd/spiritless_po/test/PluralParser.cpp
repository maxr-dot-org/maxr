/*
  Copyright © 2022 OOTA, Masato
            © 2013 Translate.
  License: CC-BY-SA-3.0
  See https://creativecommons.org/licenses/by-sa/3.0/legalcode for license details.
*/
#include <catch2/catch.hpp>
#include <random>
#include <string>
#include <vector>

#ifndef NDEBUG
#define NDEBUG
#endif

#include "spiritless_po/PluralParser.h"


// Debug Version
#ifdef NDEBUG
#undef NDEBUG
#endif
//#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_COMPILE
//#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_EXECUTE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE ENABLE_ASSERT
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE INTERPRETER
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_INTERPRETER
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_INTERPRETER
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE DEBUG_32BIT_NUM
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_NUMBER
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_NUMBER
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE DEBUG_32BIT_IF
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE DEBUG_32BIT_ELSE
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE DEBUG_32BIT_IF_ELSE
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE DEBUG_32BIT_ALL
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_NUMBER
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_PLURAL_PARSER_H_
#include "spiritless_po/PluralParser.h"
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_NUMBER
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
#undef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE

using namespace std;
using namespace spiritless_po;
using NumT = PluralParser::NumT;


/*
  This data table is derived from https://docs.translatehouse.org/projects/localization-guide/en/latest/l10n/pluralforms.html?id=l10n/pluralforms
  CC-BY-SA-3.0
  © 2013 Translate.
*/
#define PE00 \
    0
#define PE01 \
    n==1 || n%10==1 ? 0 : 1
#define PE02 \
    (n != 0)
#define PE03 \
    (n != 1)
#define PE04 \
    (n > 1)
#define PE05 \
    (n%10!=1 || n%100==11)
#define PE06 \
    (n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2)
#define PE07 \
    (n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2)
#define PE08 \
    (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)
#define PE09 \
    (n==0 ? 0 : n==1 ? 1 : 2)
#define PE10 \
    (n==1 ? 0 : (n==0 || (n%100 > 0 && n%100 < 20)) ? 1 : 2)
#define PE11 \
    (n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)
#define PE12 \
    (n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2
#define PE13 \
    (n==1) ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2
#define PE14 \
    n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2
#define PE15 \
    (n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3)
#define PE16 \
    (n==1 ? 0 : n==0 || ( n%100>1 && n%100<11) ? 1 : (n%100>10 && n%100<20 ) ? 2 : 3)
#define PE17 \
    (n==1 || n==11) ? 0 : (n==2 || n==12) ? 1 : (n > 2 && n < 20) ? 2 : 3
#define PE18 \
    (n==1) ? 0 : (n==2) ? 1 : (n != 8 && n != 11) ? 2 : 3
#define PE19 \
    (n==1) ? 0 : (n==2) ? 1 : (n == 3) ? 2 : 3
#define PE20 \
    n==1 ? 0 : n==2 ? 1 : (n>2 && n<7) ? 2 :(n>6 && n<11) ? 3 : 4
#define PE21 \
    (n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5)
/* End of Table */


#define STR_IMPL(X) #X
#define STR(X) STR_IMPL(X)
#define PLURAL_FORMS(X) "Plural-Forms: nplurals=1; plural=" STR(X) ";\n"

namespace {
    const size_t TOTAL_PLURAL_EXPRESSION = 22;
    const string plural_forms[] = {
        PLURAL_FORMS(PE00),
        PLURAL_FORMS(PE01),
        PLURAL_FORMS(PE02),
        PLURAL_FORMS(PE03),
        PLURAL_FORMS(PE04),
        PLURAL_FORMS(PE05),
        PLURAL_FORMS(PE06),
        PLURAL_FORMS(PE07),
        PLURAL_FORMS(PE08),
        PLURAL_FORMS(PE09),
        PLURAL_FORMS(PE10),
        PLURAL_FORMS(PE11),
        PLURAL_FORMS(PE12),
        PLURAL_FORMS(PE13),
        PLURAL_FORMS(PE14),
        PLURAL_FORMS(PE15),
        PLURAL_FORMS(PE16),
        PLURAL_FORMS(PE17),
        PLURAL_FORMS(PE18),
        PLURAL_FORMS(PE19),
        PLURAL_FORMS(PE20),
        PLURAL_FORMS(PE21),
    };

    NumT compiled_plural_00(NumT) { return PE00; }
    NumT compiled_plural_01(NumT n) { return PE01; }
    NumT compiled_plural_02(NumT n) { return PE02; }
    NumT compiled_plural_03(NumT n) { return PE03; }
    NumT compiled_plural_04(NumT n) { return PE04; }
    NumT compiled_plural_05(NumT n) { return PE05; }
    NumT compiled_plural_06(NumT n) { return PE06; }
    NumT compiled_plural_07(NumT n) { return PE07; }
    NumT compiled_plural_08(NumT n) { return PE08; }
    NumT compiled_plural_09(NumT n) { return PE09; }
    NumT compiled_plural_10(NumT n) { return PE10; }
    NumT compiled_plural_11(NumT n) { return PE11; }
    NumT compiled_plural_12(NumT n) { return PE12; }
    NumT compiled_plural_13(NumT n) { return PE13; }
    NumT compiled_plural_14(NumT n) { return PE14; }
    NumT compiled_plural_15(NumT n) { return PE15; }
    NumT compiled_plural_16(NumT n) { return PE16; }
    NumT compiled_plural_17(NumT n) { return PE17; }
    NumT compiled_plural_18(NumT n) { return PE18; }
    NumT compiled_plural_19(NumT n) { return PE19; }
    NumT compiled_plural_20(NumT n) { return PE20; }
    NumT compiled_plural_21(NumT n) { return PE21; }

    NumT (*compiled_plural_funcs[])(NumT) = {
        compiled_plural_00,
        compiled_plural_01,
        compiled_plural_02,
        compiled_plural_03,
        compiled_plural_04,
        compiled_plural_05,
        compiled_plural_06,
        compiled_plural_07,
        compiled_plural_08,
        compiled_plural_09,
        compiled_plural_10,
        compiled_plural_11,
        compiled_plural_12,
        compiled_plural_13,
        compiled_plural_14,
        compiled_plural_15,
        compiled_plural_16,
        compiled_plural_17,
        compiled_plural_18,
        compiled_plural_19,
        compiled_plural_20,
        compiled_plural_21,
    };


    // This function returns a random integer vector.
    vector<unsigned long int> gen_int_vector(const size_t s)
    {
        mt19937_64 random_engine;
        exponential_distribution<double> dist1(0.01);
        vector<unsigned long int> v(s);
        for (auto &elm : v) {
            elm = static_cast<unsigned long>(dist1(random_engine));
        }
        return v;
    }
}

TEMPLATE_TEST_CASE( "Default Constructor of PluralFunction", "[PluralFunction]", PluralParser, ENABLE_ASSERT::PluralParser, INTERPRETER::PluralParser ) {
    PluralParser::FunctionType plural_function;
    REQUIRE( plural_function(0) == 0 );
    REQUIRE( plural_function(1) == 0 );
    REQUIRE( plural_function(99) == 0 );
}

TEMPLATE_TEST_CASE( "Equality in PluralFunction", "[PluralFunction]",  PluralParser, ENABLE_ASSERT::PluralParser, INTERPRETER::PluralParser, DEBUG_32BIT_NUM::PluralParser, DEBUG_32BIT_IF::PluralParser, DEBUG_32BIT_ELSE::PluralParser, DEBUG_32BIT_IF_ELSE::PluralParser, DEBUG_32BIT_ALL::PluralParser ) {
    vector<typename TestType::FunctionType> test_funcs;
    for (auto &info : plural_forms) {
        auto it = TestType::Parse(info);
        test_funcs.push_back(it.second);
    }

    auto i = GENERATE(range(0, 1000));
    SECTION( STR(PE00) ) {
        auto test_f = test_funcs[0];
        auto expected_f = compiled_plural_funcs[0];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE01) ) {
        auto test_f = test_funcs[1];
        auto expected_f = compiled_plural_funcs[1];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE02) ) {
        auto test_f = test_funcs[2];
        auto expected_f = compiled_plural_funcs[2];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE03) ) {
        auto test_f = test_funcs[3];
        auto expected_f = compiled_plural_funcs[3];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE04) ) {
        auto test_f = test_funcs[4];
        auto expected_f = compiled_plural_funcs[4];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE05) ) {
        auto test_f = test_funcs[5];
        auto expected_f = compiled_plural_funcs[5];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE06) ) {
        auto test_f = test_funcs[6];
        auto expected_f = compiled_plural_funcs[6];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE07) ) {
        auto test_f = test_funcs[7];
        auto expected_f = compiled_plural_funcs[7];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE08) ) {
        auto test_f = test_funcs[8];
        auto expected_f = compiled_plural_funcs[8];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE09) ) {
        auto test_f = test_funcs[9];
        auto expected_f = compiled_plural_funcs[9];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE10) ) {
        auto test_f = test_funcs[10];
        auto expected_f = compiled_plural_funcs[10];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE11) ) {
        auto test_f = test_funcs[11];
        auto expected_f = compiled_plural_funcs[11];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE12) ) {
        auto test_f = test_funcs[12];
        auto expected_f = compiled_plural_funcs[12];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE13) ) {
        auto test_f = test_funcs[13];
        auto expected_f = compiled_plural_funcs[13];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE14) ) {
        auto test_f = test_funcs[14];
        auto expected_f = compiled_plural_funcs[14];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE15) ) {
        auto test_f = test_funcs[15];
        auto expected_f = compiled_plural_funcs[15];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE16) ) {
        auto test_f = test_funcs[16];
        auto expected_f = compiled_plural_funcs[16];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE17) ) {
        auto test_f = test_funcs[17];
        auto expected_f = compiled_plural_funcs[17];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE18) ) {
        auto test_f = test_funcs[18];
        auto expected_f = compiled_plural_funcs[18];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE19) ) {
        auto test_f = test_funcs[19];
        auto expected_f = compiled_plural_funcs[19];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE20) ) {
        auto test_f = test_funcs[20];
        auto expected_f = compiled_plural_funcs[20];
        REQUIRE(test_f(i) == expected_f(i));
    };
    SECTION( STR(PE21) ) {
        auto test_f = test_funcs[21];
        auto expected_f = compiled_plural_funcs[21];
        REQUIRE(test_f(i) == expected_f(i));
    };
}



TEST_CASE( "Plural Function Benchmark", "[!benchmark]" ) {
    const auto numbers = gen_int_vector(10000);
    vector<PluralParser::FunctionType> test_funcs;
    for (auto &info : plural_forms) {
        auto it = PluralParser::Parse(info);
        test_funcs.push_back(it.second);
    }

    BENCHMARK( STR(PE00) ) {
        auto test_f = test_funcs[0];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE01) ) {
        auto test_f = test_funcs[1];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE02) ) {
        auto test_f = test_funcs[2];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE03) ) {
        auto test_f = test_funcs[3];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE04) ) {
        auto test_f = test_funcs[4];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE05) ) {
        auto test_f = test_funcs[5];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE06) ) {
        auto test_f = test_funcs[6];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE07) ) {
        auto test_f = test_funcs[7];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE08) ) {
        auto test_f = test_funcs[8];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE09) ) {
        auto test_f = test_funcs[9];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE10) ) {
        auto test_f = test_funcs[10];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE11) ) {
        auto test_f = test_funcs[11];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE12) ) {
        auto test_f = test_funcs[12];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE13) ) {
        auto test_f = test_funcs[13];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE14) ) {
        auto test_f = test_funcs[14];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE15) ) {
        auto test_f = test_funcs[15];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE16) ) {
        auto test_f = test_funcs[16];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE17) ) {
        auto test_f = test_funcs[17];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE18) ) {
        auto test_f = test_funcs[18];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE19) ) {
        auto test_f = test_funcs[19];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE20) ) {
        auto test_f = test_funcs[20];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
    BENCHMARK( STR(PE21) ) {
        auto test_f = test_funcs[21];
        for (auto number : numbers) {
            volatile auto dummy = test_f(number);
            (void)dummy;
        }
    };
}
