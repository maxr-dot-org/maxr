/** Plural forms information parser.
    \file PluralParser.h
    \author OOTA, Masato
    \copyright Copyright © 2019, 2022 OOTA, Masato
    \par License Boost
    \parblock
      This program is distributed under the Boost Software License Version 1.0.
      You can get the license file at “https://www.boost.org/LICENSE_1_0.txt”.
    \endparblock
*/

#ifndef SPIRITLESS_PO_PLURAL_PARSER_H_
#define SPIRITLESS_PO_PLURAL_PARSER_H_

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>

#if (defined(SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_EXECUTE) || defined(SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_COMPILE)) && !defined(SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT)
#define SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT
#endif
#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT
#include <iostream>
#endif

namespace spiritless_po {
#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE
namespace SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE {
#endif

    /** This class is a parser for the plural form information, including the expression. */
    class PluralParser {
    public:
        // The plural expression parser is an LL(1) parser.
        // The type of the function to execute a plural expression.
        class FunctionType;
        /** The integer type for the plural forms.
            \note The enough size of the immediate number in the plural expression is 32 bits regardless of the size of NumT.
        */
        typedef unsigned long int NumT;

        /** Parse a plural form information.
            \param [in] plural_form_info A plural form information.
            \return The pair of the number of the plurals and the function to calculate the plural expression.

            This function parses a plural form information, such as "Plural-Forms: nplurals=2; plural=n != 1;". The result is the pair of 2 and the equivalent to [](NumT n) -> NumT { return n != 1; }.
        */
        static std::pair<NumT, FunctionType> Parse(const std::string &plural_form_info);


        /** The iterator type for the plural form information. */
        typedef std::string::const_iterator InP;
        /** The type of exception when raised by a parse error. */
        class ExpressionError : public std::runtime_error {
        private:
            // Only PluralParser can create an instance of the class.
            friend class PluralParser;
            explicit ExpressionError(const std::string &whatArg, InP it);
            explicit ExpressionError(const char *whatArg, InP it);

        public:
            /** Get the position where the error occurs.
                \return The position where the error occurs.
                \note what() returns the error message.
                \note The return value is the iterator for the input parameter of PluralParser::Parse().
            */
            InP Where() const noexcept;

        private:
            InP pos;
        };


    private:
        // Opcode for plural function
        typedef unsigned char Opcode;
        // Another function type (Internal use)
        using CompiledPluralFunctionT = PluralParser::NumT (*)(PluralParser::NumT);
        // This needs a friend because it shares some private types.
        friend class PluralParser::FunctionType;

    public:
        /** The type of the function to execute a plural expression. */
        class FunctionType {
        private:
            // Only PluralParser can create an instance of this class except for default constructor.
            friend class PluralParser;

        public:
            /** Create the function that returns 0. */
            FunctionType();

            /** This class is copyable.
                \param [in] a The source.
            */
            FunctionType(const FunctionType &a) = default;

            /** This class is movable.
                \param [in] a The source.
            */
            FunctionType(FunctionType &&a) = default;

            /** This class is destructible. */
            ~FunctionType() = default;

            /** This class is assignable.
                \param [in] a The source.
            */
            FunctionType &operator=(const FunctionType &a) = default;

            /** This class is move assignable.
                \param [in] a The source.
            */
            FunctionType &operator=(FunctionType &&a) = default;

            // Users can execute the function.
            /** Calculate the plural expression.
                \param [in] n The value relating to the translating text.
                \return The plural index.
            */
            PluralParser::NumT operator()(PluralParser::NumT n) const;

        private:
            // Users cannot create a function.
            // program and max_data_size must be bug-free.
            FunctionType(const std::vector<PluralParser::Opcode> &program,
                         size_t max_data_size);
            explicit FunctionType(CompiledPluralFunctionT func);

            NumT Read32(size_t &i) const;

        private:
            CompiledPluralFunctionT compiled_func;
            std::vector<PluralParser::Opcode> code;
            mutable std::vector<PluralParser::NumT> data;
        };


    private:
        PluralParser();
        ~PluralParser() = default;

        static void SkipSpaces(InP &it, InP end);
        static NumT GetNumber(InP &it, InP end);
        static std::pair<InP, InP> GetExpression(InP it, InP end, const std::string &keyword);
        FunctionType CreatePluralFunction();
        static FunctionType ParseExpression(InP &it, InP end);
        void PushOpcode(Opcode op, InP it);
        size_t PushIForELSEandAddress(Opcode op, InP it);
        void InsertAddress32(size_t adrs_index, size_t jump_length);
        void AdjustJumpAddress(size_t if_adrs_index, size_t else_adrs_index, InP it);
        void PushImmediateNumber(NumT n, InP it);
        void ParseTerm7(InP &it, InP end);
        void ParseTerm71(InP &it, InP end);
        void ParseTerm6(InP &it, InP end);
        void ParseTerm61(InP &it, InP end);
        void ParseTerm5(InP &it, InP end);
        void ParseTerm51(InP &it, InP end);
        void ParseTerm4(InP &it, InP end);
        void ParseTerm41(InP &it, InP end);
        void ParseTerm3(InP &it, InP end);
        void ParseTerm31(InP &it, InP end);
        void ParseTerm2(InP &it, InP end);
        void ParseTerm21(InP &it, InP end);
        void ParseTerm1(InP &it, InP end);
        void ParseTerm11(InP &it, InP end);
        void ParseTerm0(InP &it, InP end);
        void ParseValue(InP &it, InP end);

        // for debug
        static void DebugPrintOpcode(Opcode op);
        static void DebugPrintCode(const std::vector<Opcode> &cd);
        void DebugPrintCode() const;


        /* for interpreter */
        enum : Opcode {
            NUM,
            NUM32,
            NOT,
            MULT,
            DIV,
            MOD,
            ADD,
            SUB,
            LE,
            LT,
            GT,
            GE,
            EQ,
            NE,
            AND,
            OR,
            IF,
            IF32,
            ELSE,
            ELSE32,
            VAR,
            END,
        };


        /* data members */
        std::vector<Opcode> code;
        size_t top_of_data;
        size_t max_data_size;
    };



    inline PluralParser::PluralParser()
        : code(), top_of_data(0), max_data_size(0)
    {
    }



#ifndef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT
    inline void PluralParser::DebugPrintOpcode(Opcode)
    {
    }
    inline void PluralParser::DebugPrintCode(const std::vector<Opcode> &)
    {
    }
    inline void PluralParser::DebugPrintCode() const
    {
    }
#else
    inline void PluralParser::DebugPrintOpcode(Opcode op)
    {
        switch (op) {
        case NUM:
            std::cout << "NUM";
            break;
        case NUM32:
            std::cout << "NUM32";
            break;
        case NOT:
            std::cout << "NOT";
            break;
        case MULT:
            std::cout << "MULT";
            break;
        case DIV:
            std::cout << "DIV";
            break;
        case MOD:
            std::cout << "MOD";
            break;
        case ADD:
            std::cout << "ADD";
            break;
        case SUB:
            std::cout << "SUB";
            break;
        case LE:
            std::cout << "LE";
            break;
        case LT:
            std::cout << "LT";
            break;
        case GT:
            std::cout << "GT";
            break;
        case GE:
            std::cout << "GE";
            break;
        case EQ:
            std::cout << "EQ";
            break;
        case NE:
            std::cout << "NE";
            break;
        case AND:
            std::cout << "AND";
            break;
        case OR:
            std::cout << "OR";
            break;
        case IF:
            std::cout << "IF";
            break;
        case IF32:
            std::cout << "IF32";
            break;
        case ELSE:
            std::cout << "ELSE";
            break;
        case ELSE32:
            std::cout << "ELSE32";
            break;
        case VAR:
            std::cout << "VAR";
            break;
        case END:
            std::cout << "END";
            break;
        default:
            std::cout << static_cast<unsigned int>(op);
        }
    }
    inline void PluralParser::DebugPrintCode(const std::vector<Opcode> &cd)
    {
        size_t n = 0;
        for (size_t i = 0; i < cd.size(); ++i) {
            std::cout << i << ": ";
            if (n == 0) {
                DebugPrintOpcode(cd[i]);
                switch (cd[i]) {
                case NUM:
                case IF:
                case ELSE:
                    n = 1;
                    break;
                case NUM32:
                case IF32:
                case ELSE32:
                    n = 4;
                    break;
                }
            } else {
                std::cout << static_cast<unsigned int>(cd[i]);
                --n;
            }
            std::cout << std::endl;
        }
    }
    inline void PluralParser::DebugPrintCode() const
    {
        DebugPrintCode(code);
    }
#endif // SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT



    inline PluralParser::FunctionType::FunctionType()
        : FunctionType([](NumT) -> NumT { return 0; })
    {
    }

    inline PluralParser::FunctionType::FunctionType(const std::vector<PluralParser::Opcode> &program,
                                                    size_t max_data_size)
        : compiled_func(nullptr),
          code(program),
          data(max_data_size)
    {
        // We can read an item at the current position + 4 without checking.
        code.push_back(END);
        code.push_back(END);
        code.push_back(END);
        code.push_back(END);
    }

    inline PluralParser::FunctionType::FunctionType(CompiledPluralFunctionT func)
        : compiled_func(func), code(0), data(0)
    {
    }

    inline PluralParser::NumT PluralParser::FunctionType::Read32(size_t &i) const
    {
        NumT n = code[i];
        n <<= 8;
        n |= code[++i];
        n <<= 8;
        n |= code[++i];
        n <<= 8;
        n |= code[++i];
        return n;
    }

    inline PluralParser::NumT PluralParser::FunctionType::operator()(const NumT n) const
    {
        if (compiled_func) {
            return compiled_func(n);
        }

#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_EXECUTE
        PluralParser::DebugPrintCode(code);
#endif
        size_t top = static_cast<size_t>(-1);
        for (size_t i = 0; i < code.size() && code[i] != END; ++i) {
#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_EXECUTE
            std::cout << i << ": ";
            DebugPrintOpcode(code[i]);
            std::cout << std::endl;
#endif
            switch (code[i]) {
            case NUM:
                ++i;
                ++top;
                data.at(top) = code[i];
                break;
            case NUM32:
                ++top;
                ++i;
                data.at(top) = Read32(i);
                break;
            case NOT:
                data[top] = !data.at(top);
                break;
            case MULT:
                data.at(top - 1) *= data.at(top);
                --top;
                break;
            case DIV:
                data.at(top-1) /= data.at(top);
                --top;
                break;
            case MOD:
                data.at(top-1) %= data.at(top);
                --top;
                break;
            case ADD:
                data.at(top-1) += data.at(top);
                --top;
                break;
            case SUB:
                data.at(top-1) -= data.at(top);
                --top;
                break;
            case LE:
                data[top-1] = data.at(top-1) <= data.at(top);
                --top;
                break;
            case LT:
                data[top-1] = data.at(top-1) < data.at(top);
                --top;
                break;
            case GT:
                data[top-1] = data.at(top-1) > data.at(top);
                --top;
                break;
            case GE:
                data[top-1] = data.at(top-1) >= data.at(top);
                --top;
                break;
            case EQ:
                data[top-1] = data.at(top-1) == data.at(top);
                --top;
                break;
            case NE:
                data[top-1] = data.at(top-1) != data.at(top);
                --top;
                break;
            case AND:
                data[top-1] = data.at(top-1) && data.at(top);
                --top;
                break;
            case OR:
                data[top-1] = data.at(top-1) || data.at(top);
                --top;
                break;
            case IF:
                ++i;
                if (!data.at(top)) {
                    i += code[i];
                }
                --top;
                break;
            case IF32:
                {
                    ++i;
                    const NumT r = Read32(i);
                    if (!data.at(top)) {
                        i += r;
                    }
                    --top;
                }
                break;
            case ELSE:
                ++i;
                i += code[i];
                break;
            case ELSE32:
                ++i;
                i += Read32(i);
                break;
            case VAR:
                ++top;
                data.at(top) = n;
                break;
            default:
                assert(false);
            }
            assert(i < code.size());
            assert(top == static_cast<size_t>(-1) || top < data.size());
        }
        assert(top == 0);
        return data.at(0);
    }



    inline PluralParser::ExpressionError::ExpressionError(const std::string &whatArg, const InP it)
        : std::runtime_error(whatArg), pos(it)
    {
    }

    inline PluralParser::ExpressionError::ExpressionError(const char *whatArg, const InP it)
        : std::runtime_error(whatArg), pos(it)
    {
    }

    inline PluralParser::InP PluralParser::ExpressionError::Where() const noexcept
    {
        return pos;
    }



    // Plural forms information parser.
    inline std::pair<PluralParser::NumT, PluralParser::FunctionType>
    PluralParser::Parse(const std::string &plural_exp)
    {
        const InP begin = plural_exp.cbegin();
        const InP end = plural_exp.cend();
        auto npluralsRange = GetExpression(begin, end, "nplurals");
        const NumT nplurals = GetNumber(npluralsRange.first, npluralsRange.second);

        auto pluralRange = GetExpression(begin, end, "plural");
        const auto f = ParseExpression(pluralRange.first, pluralRange.second);

        return std::make_pair(nplurals, f);
    }



    // Skip spaces (Utility function)
    inline void PluralParser::SkipSpaces(InP &it, const InP end)
    {
        while (it != end && std::isspace(static_cast<unsigned char>(*it))) {
            ++it;
        }
    }

    // get a number. (Utility function)
    inline PluralParser::NumT PluralParser::GetNumber(InP &it, const InP end)
    {
        std::string s;
        while (it != end && std::isdigit(static_cast<unsigned char>(*it))) {
            s += *it++;
        }
        if (!s.empty()) {
            return std::stoi(s);
        }
        throw ExpressionError("Parse error: '0'..'9' is expected.", it);
    }

    // get a expression. (Utility function)
    inline std::pair<PluralParser::InP, PluralParser::InP>
    PluralParser::GetExpression(const InP begin, const InP end, const std::string &keyword)
    {
        auto curIt = std::find_end(begin, end, keyword.cbegin(), keyword.cend());
        if (curIt == end) {
            throw ExpressionError("Parse error: '" + keyword + "' is not found.", begin);
        }
        std::advance(curIt, keyword.length());
        SkipSpaces(curIt, end);
        if (*curIt != '=') {
            throw ExpressionError("'=' is expected.", curIt);
        }
        ++curIt;
        SkipSpaces(curIt, end);
        const InP find_pos = curIt;
        while (curIt != end && *curIt != ';') {
            ++curIt;
        }
        if (*curIt != ';') {
            throw ExpressionError("';' is expected.", curIt);
        }
        return std::make_pair(find_pos, curIt);
    }

#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_INTERPRETER
    inline PluralParser::FunctionType PluralParser::CreatePluralFunction()
    {
        return FunctionType(code, max_data_size);
    }
#else
    /*
      These plural expressions is derived from lib/plurals.js in "https://github.com/alexanderwallin/node-gettext". It's published by a sort of MIT License but we have no need to show the copyright notice and the permission notice because the second sentence is removed.

      Copyright (c) 2011-2012 Andris Reinman

      Permission is hereby granted, free of charge, to any person obtaining a copy
      of this software and associated documentation files (the "Software"), to deal
      in the Software without restriction, including without limitation the rights
      to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
      copies of the Software, and to permit persons to whom the Software is
      furnished to do so, subject to the following conditions:

      THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
      IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
      FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
      AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
      LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
      OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
      SOFTWARE.
    */
    inline PluralParser::FunctionType PluralParser::CreatePluralFunction()
    {
        static std::map<decltype(code), CompiledPluralFunctionT> func_map {
            {
                { NUM, 0 },
                [](NumT) -> NumT { return 0; }
            },
            {
                { VAR, NUM, 1, EQ, VAR, NUM, 10, MOD, NUM, 1, EQ, OR, IF, 4, NUM, 0, ELSE, 2, NUM, 1 },
                [](NumT n) -> NumT { return n == 1 || n % 10 == 1 ? 0 : 1; }
            },
            {
                { VAR, NUM, 0, NE },
                [](NumT n) -> NumT { return n != 0; }
            },
            {
                { VAR, NUM, 1, NE },
                [](NumT n) -> NumT { return n != 1; }
            },
            {
                { VAR, NUM, 1, GT },
                [](NumT n) -> NumT { return n > 1; }
            },
            {
                { VAR, NUM, 10, MOD, NUM, 1, NE, VAR, NUM, 100, MOD, NUM, 11, EQ, OR },
                [](NumT n) -> NumT { return n % 10 != 1 || n % 100 == 11; }
            },
            {
                { VAR, NUM, 10, MOD, NUM, 1, EQ, VAR, NUM, 100, MOD, NUM, 11, NE, AND, IF, 4, NUM, 0, ELSE, 12, VAR, NUM, 0, NE, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n % 10 == 1 && n % 100 != 11 ? 0 : n != 0 ? 1 : 2; }
            },
            {
                { VAR, NUM, 10, MOD, NUM, 1, EQ, VAR, NUM, 100, MOD, NUM, 11, NE, AND, IF, 4, NUM, 0, ELSE, 31, VAR, NUM, 10, MOD, NUM, 2, GE, VAR, NUM, 100, MOD, NUM, 10, LT, VAR, NUM, 100, MOD, NUM, 20, GE, OR, AND, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n % 10 == 1 && n % 100 != 11 ? 0 : n % 10 >= 2 && (n % 100 < 10 || n % 100 >= 20) ? 1 : 2; }
            },
            {
                { VAR, NUM, 10, MOD, NUM, 1, EQ, VAR, NUM, 100, MOD, NUM, 11, NE, AND, IF, 4, NUM, 0, ELSE, 39, VAR, NUM, 10, MOD, NUM, 2, GE, VAR, NUM, 10, MOD, NUM, 4, LE, VAR, NUM, 100, MOD, NUM, 10, LT, VAR, NUM, 100, MOD, NUM, 20, GE, OR, AND, AND, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n % 10 == 1 && n % 100 != 11 ? 0 : n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20) ? 1 : 2; }
            },
            {
                { VAR, NUM, 0, EQ, IF, 4, NUM, 0, ELSE, 12, VAR, NUM, 1, EQ, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n == 0 ? 0 : n == 1 ? 1 : 2; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 28, VAR, NUM, 0, EQ, VAR, NUM, 100, MOD, NUM, 0, GT, VAR, NUM, 100, MOD, NUM, 20, LT, AND, OR, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n == 1 ? 0 : (n == 0 || (n % 100 > 0 && n % 100 < 20)) ? 1 : 2; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 39, VAR, NUM, 10, MOD, NUM, 2, GE, VAR, NUM, 10, MOD, NUM, 4, LE, VAR, NUM, 100, MOD, NUM, 10, LT, VAR, NUM, 100, MOD, NUM, 20, GE, OR, AND, AND, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n == 1 ? 0 : n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 || n % 100 >= 20) ? 1 : 2; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 17, VAR, NUM, 2, GE, VAR, NUM, 4, LE, AND, IF, 4, NUM, 1, ELSE, 2, NUM, 2 },
                [](NumT n) -> NumT { return n == 1 ? 0 : (n >= 2 && n <= 4) ? 1 : 2; }
            },
            {
                { VAR, NUM, 100, MOD, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 36, VAR, NUM, 100, MOD, NUM, 2, EQ, IF, 4, NUM, 1, ELSE, 23, VAR, NUM, 100, MOD, NUM, 3, EQ, VAR, NUM, 100, MOD, NUM, 4, EQ, OR, IF, 4, NUM, 2, ELSE, 2, NUM, 3 },
                [](NumT n) -> NumT { return n % 100 == 1 ? 0 : n % 100 == 2 ? 1 : n % 100 == 3 || n % 100 == 4 ? 2 : 3; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 49, VAR, NUM, 0, EQ, VAR, NUM, 100, MOD, NUM, 1, GT, VAR, NUM, 100, MOD, NUM, 11, LT, AND, OR, IF, 4, NUM, 1, ELSE, 23, VAR, NUM, 100, MOD, NUM, 10, GT, VAR, NUM, 100, MOD, NUM, 20, LT, AND, IF, 4, NUM, 2, ELSE, 2, NUM, 3 },
                [](NumT n) -> NumT { return n == 1 ? 0 : n == 0 || ( n % 100 > 1 && n % 100 < 11) ? 1 : (n % 100 > 10 && n % 100 < 20 ) ? 2 : 3; }
            },
            {
                { VAR, NUM, 1, EQ, VAR, NUM, 11, EQ, OR, IF, 4, NUM, 0, ELSE, 32, VAR, NUM, 2, EQ, VAR, NUM, 12, EQ, OR, IF, 4, NUM, 1, ELSE, 17, VAR, NUM, 2, GT, VAR, NUM, 20, LT, AND, IF, 4, NUM, 2, ELSE, 2, NUM, 3 },
                [](NumT n) -> NumT { return (n == 1 || n == 11) ? 0 : (n == 2 || n == 12) ? 1 : (n > 2 && n < 20) ? 2 : 3; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 27, VAR, NUM, 2, EQ, IF, 4, NUM, 1, ELSE, 17, VAR, NUM, 8, NE, VAR, NUM, 11, NE, AND, IF, 4, NUM, 2, ELSE, 2, NUM, 3 },
                [](NumT n) -> NumT { return n == 1 ? 0 : n == 2 ? 1 : (n != 8 && n != 11) ? 2 : 3; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 22, VAR, NUM, 2, EQ, IF, 4, NUM, 1, ELSE, 12, VAR, NUM, 3, EQ, IF, 4, NUM, 2, ELSE, 2, NUM, 3 },
                [](NumT n) -> NumT { return n == 1 ? 0 : n == 2 ? 1 : n == 3 ? 2 : 3; }
            },
            {
                { VAR, NUM, 1, EQ, IF, 4, NUM, 0, ELSE, 32, VAR, NUM, 2, EQ, IF, 4, NUM, 1, ELSE, 22, VAR, NUM, 7, LT, IF, 4, NUM, 2, ELSE, 12, VAR, NUM, 11, LT, IF, 4, NUM, 3, ELSE, 2, NUM, 4 },
                [](NumT n) -> NumT { return n == 1 ? 0 : n == 2 ? 1 : n < 7 ? 2 : n < 11 ? 3 : 4; }
            },
            {
                { VAR, NUM, 0, EQ, IF, 4, NUM, 0, ELSE, 56, VAR, NUM, 1, EQ, IF, 4, NUM, 1, ELSE, 46, VAR, NUM, 2, EQ, IF, 4, NUM, 2, ELSE, 36, VAR, NUM, 100, MOD, NUM, 3, GE, VAR, NUM, 100, MOD, NUM, 10, LE, AND, IF, 4, NUM, 3, ELSE, 15, VAR, NUM, 100, MOD, NUM, 11, GE, IF, 4, NUM, 4, ELSE, 2, NUM, 5 },
                [](NumT n) -> NumT { return n == 0 ? 0 : n == 1 ? 1 : n == 2 ? 2 : n % 100 >= 3 && n % 100 <= 10 ? 3 : n % 100 >= 11 ? 4 : 5; }
            },
        };
        auto it = func_map.find(code);
        if (it != func_map.end()) {
            return FunctionType(it->second);
        } else {
            return FunctionType(code, max_data_size);
        }
    }
    /* End of the derived work. */
#endif

    // This is a parser of the plural expression, and returns the decision function.
    // InP is an input iterator type.
    // start = term7;
    inline PluralParser::FunctionType PluralParser::ParseExpression(InP &it, const InP end)
    {
        PluralParser result;
        result.ParseTerm7(it, end);
        SkipSpaces(it, end);
#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_PRINT_COMPILE
        DebugPrintCode(result.code);
#endif
        if (it == end) {
            if (result.top_of_data == 1) {
                return result.CreatePluralFunction();
            } else {
                throw ExpressionError("Bug: Invalid data stack level.", it);
            }
        } else {
            throw ExpressionError("Parse error: Invalid character is detected.", it);
        }
    }



    // Push opcode to this->code and adjust top_of_data and max_data_size.
    inline void PluralParser::PushOpcode(const Opcode op, const InP it)
    {
        switch (op) {
        case NUM:
        case NUM32:
        case VAR:
            ++top_of_data;
            max_data_size = std::max(max_data_size, top_of_data);
            break;
        case MULT:
        case DIV:
        case MOD:
        case ADD:
        case SUB:
        case LE:
        case LT:
        case GT:
        case GE:
        case EQ:
        case NE:
        case AND:
        case OR:
        case IF:
        case ELSE:
            if (top_of_data == 0) {
                throw ExpressionError("Bug: Data stack underflow.", it);
            }
            --top_of_data;
            break;
        case IF32:
        case ELSE32:
            throw ExpressionError("Bug: IF32 and ELSE32 must not be pushed to code.", it);
            break;
        case NOT:
            break;
        case END:
            throw ExpressionError("Bug: END must not be pushed to code.", it);
            break;
        default:
            throw ExpressionError("Bug: Unknown code is pushed.", it);
        }
        code.push_back(op);
    }

    // Push IF or ELSE
    // return relative_address_index for IF and ELSE.
    inline size_t PluralParser::PushIForELSEandAddress(const Opcode op, const InP it)
    {
        PushOpcode(op, it);
        const size_t index = code.size();
        code.push_back(0);
        return index;
    }

    // 8 bit address replace 32 bit address.
    inline void PluralParser::InsertAddress32(size_t adrs_index, size_t jump_length)
    {
        code.resize(code.size() + 3);
        std::copy(code.begin() + adrs_index + 1, code.end() - 3,
                  code.begin() + adrs_index + 4);
        code[adrs_index + 0] = (jump_length >> 24) & 0xFF;
        code[adrs_index + 1] = (jump_length >> 16) & 0xFF;
        code[adrs_index + 2] = (jump_length >> 8) & 0xFF;
        code[adrs_index + 3] = jump_length & 0xFF;
    }

    // Adjust the jump addresses in an IF-ELSE block.
    // The location of END-IF is code.end().
    inline void PluralParser::AdjustJumpAddress(const size_t if_adrs_index, size_t else_adrs_index, const InP it)
    {
        if (if_adrs_index < 1) {
            throw ExpressionError("Bug: The index of an address must be more than 0.", it);
        }
        if (if_adrs_index >= else_adrs_index) {
            throw ExpressionError("Bug: The index of ELSE must be more than IF's.", it);
        }
        if (else_adrs_index >= code.size()) {
            throw ExpressionError("Bug: The size of code[] must be more than the index of ELSE.", it);
        }
        if (code[if_adrs_index - 1] != IF || code[else_adrs_index - 1] != ELSE) {
            throw ExpressionError("Bug: The opcodes must be IF and ELSE.", it);
        }

        const size_t endif_index = code.size();
        size_t if_length = else_adrs_index - if_adrs_index;
        const size_t else_length = endif_index - else_adrs_index - 1;
        // Practically, the relative address is always 8 bit.

#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_ELSE
        const bool else_length_is_32bit = true;
#else
        const bool else_length_is_32bit = else_length > 0xFF;
#endif
        if (else_length_is_32bit) {
            // if_length includes ELSE command and ELSE will replace ELSE32.
            if_length += 3;
        }
#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_IF
        const bool if_length_is_32bit = true;
#else
        const bool if_length_is_32bit = if_length > 0xFF;
#endif
        if (if_length_is_32bit) {
            InsertAddress32(if_adrs_index, if_length);
            code[if_adrs_index - 1] = IF32;
            else_adrs_index += 3;
        } else {
            code[if_adrs_index] = static_cast<Opcode>(if_length);
        }
        if (else_length_is_32bit) {
            InsertAddress32(else_adrs_index, else_length);
            code[else_adrs_index - 1] = ELSE32;
        } else {
            code[else_adrs_index] = static_cast<Opcode>(else_length);
        }
    }

    // Push an immediate number into code.
    inline void PluralParser::PushImmediateNumber(const NumT n, const InP it)
    {
#ifndef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_USE_32BIT_NUMBER
        if (n <= 0xFF) {
            // Practically, the immediate number is always 8 bit.
            PushOpcode(NUM, it);
            code.push_back(static_cast<Opcode>(n));
            return;
        }
#endif
        PushOpcode(NUM32, it);
        code.push_back((n >> 24) & 0xFF);
        code.push_back((n >> 16) & 0xFF);
        code.push_back((n >> 8) & 0xFF);
        code.push_back(n & 0xFF);
    }



    // Lower level parsers.
    // term7 = term6, term71;
    inline void PluralParser::ParseTerm7(InP &it, const InP end)
    {
        ParseTerm6(it, end);
        ParseTerm71(it, end);
    }

    // term71 = e | '?', term7, ':', term7;
    inline void PluralParser::ParseTerm71(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && *it == '?') {
            ++it;
            const size_t if_adrs = PushIForELSEandAddress(IF, it);
            ParseTerm7(it, end);
            const size_t else_adrs = PushIForELSEandAddress(ELSE, it);
            SkipSpaces(it, end);
            if (it != end && *it == ':') {
                ++it;
                ParseTerm7(it, end);
                AdjustJumpAddress(if_adrs, else_adrs, it);
            } else {
                throw ExpressionError("Parse error: ':' is expected.", it);
            }
        }
    }

    // term6 = term5, term61;
    inline void PluralParser::ParseTerm6(InP &it, const InP end)
    {
        ParseTerm5(it, end);
        ParseTerm61(it, end);
    }

    // term61 = e | '||', term6;
    inline void PluralParser::ParseTerm61(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && *it == '|') {
            ++it;
            if (it != end && *it == '|') {
                ++it;
                ParseTerm6(it, end);
                PushOpcode(OR, it);
            } else {
                throw ExpressionError("Parse error: '|' is expected.", it);
            }
        }
    }

    // term5 = term4, term51;
    inline void PluralParser::ParseTerm5(InP &it, const InP end)
    {
        ParseTerm4(it, end);
        ParseTerm51(it, end);
    }

    // term51 = e | '&&', term5;
    inline void PluralParser::ParseTerm51(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && *it == '&') {
            ++it;
            if (it != end && *it == '&') {
                ++it;
                ParseTerm5(it, end);
                PushOpcode(AND, it);
            } else {
                throw ExpressionError("Parse error: '&' is expected.", it);
            }
        }
    }

    // term4 = term3, term41;
    inline void PluralParser::ParseTerm4(InP &it, const InP end)
    {
        ParseTerm3(it, end);
        ParseTerm41(it, end);
    }

    // term41 = e | '==', term4 | '!=', term4;
    inline void PluralParser::ParseTerm41(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && (*it == '=' || *it == '!')) {
            const bool eq = *it == '=';
            ++it;
            if (it != end && *it == '=') {
                ++it;
                ParseTerm4(it, end);
                if (eq) {
                    PushOpcode(EQ, it);
                } else {
                    PushOpcode(NE, it);
                }
            } else {
                throw ExpressionError("Parse error: '=' is expected.", it);
            }
        }
    }

    // term3 = term2, term31;
    inline void PluralParser::ParseTerm3(InP &it, const InP end)
    {
        ParseTerm2(it, end);
        ParseTerm31(it, end);
    }

    // term31 = e | '<', term3 | '<=', term3 | '>', term3 | '<=', term3;
    inline void PluralParser::ParseTerm31(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && (*it == '<' || *it == '>')) {
            const bool lt = *it == '<';
            ++it;
            const bool eq = it != end && *it == '=';
            if (eq) {
                ++it;
            }
            ParseTerm3(it, end);
            if (lt) {
                if (eq) {
                    PushOpcode(LE, it);
                } else {
                    PushOpcode(LT, it);
                }
            } else {
                if (eq) {
                    PushOpcode(GE, it);
                } else {
                    PushOpcode(GT, it);
                }
            }
        }
    }

    // term2 = term1, term21;
    inline void PluralParser::ParseTerm2(InP &it, const InP end)
    {
        ParseTerm1(it, end);
        ParseTerm21(it, end);
    }

    // term21 = e | '+', term2 | '-', term2;
    inline void PluralParser::ParseTerm21(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && (*it == '+' || *it == '-')) {
            const bool plus = *it == '+';
            ++it;
            ParseTerm2(it, end);
            if (plus) {
                PushOpcode(ADD, it);
            } else {
                PushOpcode(SUB, it);
            }
        }
    }

    // term1 = term0, term11;
    inline void PluralParser::ParseTerm1(InP &it, const InP end)
    {
        ParseTerm0(it, end);
        ParseTerm11(it, end);
    }

    // term11 = e | '*', term1 | '/', term1 | '%', term1;
    inline void PluralParser::ParseTerm11(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end && (*it == '*' || *it == '/' || *it == '%')) {
            const char op = *it;
            ++it;
            ParseTerm1(it, end);
            if (op == '*') {
                PushOpcode(MULT, it);
            } else if (op == '/') {
                PushOpcode(DIV, it);
            } else {
                PushOpcode(MOD, it);
            }
        }
    }

    // term0 = {'!'} value;
    inline void PluralParser::ParseTerm0(InP &it, const InP end)
    {
        bool isNot = false;
        for (;;) {
            SkipSpaces(it, end);
            if (it != end && *it == '!') {
                ++it;
                isNot = !isNot;
            } else {
                break;
            }
        }
        ParseValue(it, end);
        if (isNot) {
            PushOpcode(NOT, it);
        }
    }

    // value = 'n' | digit, {digit} | '(', term7, ')';
    // digit = '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';
    inline void PluralParser::ParseValue(InP &it, const InP end)
    {
        SkipSpaces(it, end);
        if (it != end) {
            if (*it == 'n') {
                ++it;
                PushOpcode(VAR, it);
                return;
            } else if (*it == '(') {
                ++it;
                ParseTerm7(it, end);
                SkipSpaces(it, end);
                if (it != end && *it == ')') {
                    ++it;
                    return;
                } else {
                    throw ExpressionError("Parse error: ')' is expected.", it);
                }
            } else {
                try {
                    const NumT v = GetNumber(it, end);
                    PushImmediateNumber(v, it);
                    return;
                } catch (const ExpressionError &) {
                    // fall through
                }
            }
        }
        throw ExpressionError("Parse error: 'n' or '(', '0'..'9' is expected.", it);
    }

#ifdef SPIRITLESS_PO_DEBUG_PLURAL_PARSER_NAMESPACE
}
#endif
} // namespace spiritless_po

#endif // SPIRITLESS_PO_PLURAL_PARSER_H_
