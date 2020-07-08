// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_ARGUMENT_PARSER_H
#define CSYS_ARGUMENT_PARSER_H
#pragma once

#include "csys/api.h"
#include "csys/string.h"
#include "csys/exceptions.h"
#include <vector>
#include <stdexcept>
#include <string_view>

namespace csys
{
    namespace
    {
        inline const std::string_view s_Reserved("\\[]\"");                                             //!< All the reserved chars
        inline constexpr char s_ErrMsgReserved[] = "Reserved chars '\\, [, ], \"' must be escaped with \\";    //!< Error message for reserved chars
    }

    /*!
     * \brief
     *      Helper class to check for parsing arguments that involve checking for reserved chars
     */
    struct CSYS_API Reserved
    {
        /*!
         * \brief
         *      Checks if the current char 'c' is the escaping char
         * \param c
         *      Char to compare against with the escaping char
         * \return
         *      Returns true if the char 'c' is the escaping char
         */
        static inline bool IsEscapeChar(char c)
        { return c == '\\'; }

        /*!
         * \brief
         *      Checks if the current char is reserved
         * \param c
         *      Char to check
         * \return
         *      Returns true if the current char 'c' is reserved
         */
        static inline bool IsReservedChar(char c)
        {
            for (auto rc : s_Reserved) if (c == rc) return true;
            return false;
        }

        /*!
         * \brief
         *      Checks if a char is escaping another
         * \param input
         *      Chars to check for it escaping
         * \param pos
         *      Char that is escaping
         * \return
         *      Returns true if the current char is the escaping char and is escaping
         */
        static inline bool IsEscaping(std::string &input, size_t pos)
        {
            return pos < input.size() - 1 && IsEscapeChar(input[pos]) && IsReservedChar(input[pos + 1]);
        }

        /*!
         * \brief
         *      Checks if a certain char is being escaped
         * \param input
         *      Chars to check
         * \param pos
         *      The current char to check if its being escaped
         * \return
         *      Returns true if the current char at 'pos' is being escaped
         */
        static inline bool IsEscaped(std::string &input, size_t pos)
        {
            bool result = false;

            // Go through checking if the prev char is getting escaped and toggle between true and false
            // Edge case is escaping the escaping char before another
            for (size_t i = pos; i > 0; --i)
                if (IsReservedChar(input[i]) && IsEscapeChar(input[i - 1]))
                    result = !result;
                else
                    break;
            return result;
        }

        // Delete unwanted operations
        Reserved() = delete;
        ~Reserved() = delete;
        Reserved(Reserved&&) = delete;
        Reserved(const Reserved&) = delete;
        Reserved& operator=(Reserved&&) = delete;
        Reserved& operator=(const Reserved&) = delete;
    };

    /*!
     * \brief
     *      Used for parsing arguments of different types
     * \tparam T
     *      Argument type
     */
    template<typename T>
    struct CSYS_API ArgumentParser
    {
        /*!
         * \brief
         *      Constructor that parses the argument and sets value T
         * \param input
         *      Command line set of arguments to be parsed
         * \param start
         *      Start in 'input' to where this argument should be parsed from
         */
        inline ArgumentParser(String &input, size_t &start);

        T m_Value; //!< Value of parsed argument
    };

    /*!
     * \brief
     *      Macro for template specialization, used for cleaner code reuse
     */
#define ARG_PARSE_BASE_SPEC(TYPE) \
  template<> \
  struct CSYS_API ArgumentParser<TYPE> \
  { \
    inline ArgumentParser(String &input, size_t &start); \
    TYPE m_Value = 0; \
  }; \
  inline ArgumentParser<TYPE>::ArgumentParser(String &input, size_t &start)

    /*!
     * \brief
     *      Macro for getting the sub-string within a range, used for readability
     */
#define ARG_PARSE_SUBSTR(range) input.m_String.substr(range.first, range.second - range.first)

    /*!
     * \brief
     *      Macro for build-int types that already have functions in the stl for parsing them from a string
     */
#define ARG_PARSE_GENERAL_SPEC(TYPE, TYPE_NAME, FUNCTION) \
  ARG_PARSE_BASE_SPEC(TYPE) \
  { \
    auto range = input.NextPoi(start); \
    try \
    { \
      m_Value = (TYPE)FUNCTION(ARG_PARSE_SUBSTR(range), &range.first); \
    } \
    catch (const std::out_of_range&) \
    { \
      throw Exception(std::string("Argument too large for ") + TYPE_NAME, \
                              input.m_String.substr(range.first, range.second));  \
    } \
    catch (const std::invalid_argument&) \
    { \
      throw Exception(std::string("Missing or invalid ") + TYPE_NAME + " argument", \
                              input.m_String.substr(range.first, range.second)); } \
  }

    /*!
     * \brief
     *      Template specialization for string argument parsing
     */
    ARG_PARSE_BASE_SPEC(csys::String)
    {
        m_Value.m_String.clear(); // Empty string before using

        // Lambda for getting a word from the string and checking for reserved chars
        static auto GetWord = [](std::string &str, size_t start, size_t end)
        {
            // For issues with reserved chars
            static std::string invalid_chars;
            invalid_chars.clear();

            std::string result;

            // Go through the str from start to end
            for (size_t i = start; i < end; ++i)
                // general case, not reserved char
                if (!Reserved::IsReservedChar(str[i]))
                    result.push_back(str[i]);
                // is a reserved char
                else
                {
                    // check for \ char and if its escaping
                    if (Reserved::IsEscapeChar(str[i]) && Reserved::IsEscaping(str, i))
                        result.push_back(str[++i]);
                    // reserved char but not being escaped
                    else
                        throw Exception(s_ErrMsgReserved, str.substr(start, end - start));
                }

            return result;
        };

        // Go to the start of the string argument
        auto range = input.NextPoi(start);

        // If its a single string
        if (input.m_String[range.first] != '"')
            m_Value = GetWord(input.m_String, range.first, range.second);
        // Multi word string
        else
        {
            ++range.first; // move past the first "
            while (true)
            {
                // Get the next non-escaped "
                range.second = input.m_String.find('"', range.first);
                while (range.second != std::string::npos && Reserved::IsEscaped(input.m_String, range.second))
                    range.second = input.m_String.find('"', range.second + 1);

                // Check for closing "
                if (range.second == std::string::npos)
                {
                    range.second = input.m_String.size();
                    throw Exception("Could not find closing '\"'", ARG_PARSE_SUBSTR(range));
                }

                // Add word to already existing string
                m_Value.m_String += GetWord(input.m_String, range.first, range.second);

                // Go to next word
                range.first = range.second + 1;

                // End of string check
                if (range.first < input.m_String.size() && !std::isspace(input.m_String[range.first]))
                {
                    // joining two strings together
                    if (input.m_String[range.first] == '"')
                        ++range.first;
                }
                else
                    // End of input
                    break;
            }
        }

        // Finished parsing
        start = range.second + 1;
    }

    /*!
     * \brief
     *      Template specialization for boolean argument parsing
     */
    ARG_PARSE_BASE_SPEC(bool)
    {
        // Error messages
        static const char *s_err_msg = "Missing or invalid boolean argument";
        static const char *s_false = "false";
        static const char *s_true = "true";

        // Get argument
        auto range = input.NextPoi(start);

        // check if the length is between the len of "true" and "false"
        input.m_String[range.first] = char(std::tolower(input.m_String[range.first]));

        // true branch
        if (range.second - range.first == 4 && input.m_String[range.first] == 't')
        {
            // Go through comparing grabbed arg to "true" char by char, bail if not the same
            for (size_t i = range.first + 1; i < range.second; ++i)
                if ((input.m_String[i] = char(std::tolower(input.m_String[i]))) != s_true[i - range.first])
                    throw Exception(s_err_msg + std::string(", expected true"), ARG_PARSE_SUBSTR(range));
            m_Value = true;
        }
        // false branch
        else if (range.second - range.first == 5 && input.m_String[range.first] == 'f')
        {
            // Go through comparing grabbed arg to "false" char by char, bail if not the same
            for (size_t i = range.first + 1; i < range.second; ++i)
                if ((input.m_String[i] = char(std::tolower(input.m_String[i]))) != s_false[i - range.first])
                    throw Exception(s_err_msg + std::string(", expected false"), ARG_PARSE_SUBSTR(range));
            m_Value = false;
        }
        // anything else, not true or false
        else
            throw Exception(s_err_msg, ARG_PARSE_SUBSTR(range));
    }

    /*!
     * \brief
     *      Template specialization for char argument parsing
     */
    ARG_PARSE_BASE_SPEC(char)
    {
        // Grab the argument
        auto range = input.NextPoi(start);
        size_t len = range.second - range.first;

        // Check if its 3 or more letters
        if (len > 2 || len <= 0)
            throw Exception("Too many or no chars were given", ARG_PARSE_SUBSTR(range));
        // potential reserved char
        else if (len == 2)
        {
            // Check if the first char is \ and the second is a reserved char
            if (!Reserved::IsEscaping(input.m_String, range.first))
                throw Exception("Too many chars were given", ARG_PARSE_SUBSTR(range));

            // is correct
            m_Value = input.m_String[range.first + 1];
        }
        // if its one char and reserved
        else if (Reserved::IsReservedChar(input.m_String[range.first]))
            throw Exception(s_ErrMsgReserved, ARG_PARSE_SUBSTR(range));
        // one char, not reserved
        else
            m_Value = input.m_String[range.first];
    }

    /*!
     * \brief
     *      Template specialization for unsigned char argument parsing
     */
    ARG_PARSE_BASE_SPEC(unsigned char)
    {
        // Grab the argument
        auto range = input.NextPoi(start);
        size_t len = range.second - range.first;

        // Check if its 3 or more letters
        if (len > 2 || len <= 0)
            throw Exception("Too many or no chars were given", ARG_PARSE_SUBSTR(range));
        // potential reserved char
        else if (len == 2)
        {
            // Check if the first char is \ and the second is a reserved char
            if (!Reserved::IsEscaping(input.m_String, range.first))
                throw Exception("Too many chars were given", ARG_PARSE_SUBSTR(range));

            // is correct
            m_Value = static_cast<unsigned char>(input.m_String[range.first + 1]);
        }
        // if its one char and reserved
        else if (Reserved::IsReservedChar(input.m_String[range.first]))
            throw Exception(s_ErrMsgReserved, ARG_PARSE_SUBSTR(range));
        // one char, not reserved
        else
            m_Value = static_cast<unsigned char>(input.m_String[range.first]);
    }

    /*!
     * \brief
     *      Template specialization for short argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(short, "signed short", std::stoi)

    /*!
     * \brief
     *      Template specialization for unsigned short argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(unsigned short, "unsigned short", std::stoul)

    /*!
     * \brief
     *      Template specialization for int argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(int, "signed int", std::stoi)

    /*!
     * \brief
     *      Template specialization for unsigned int argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(unsigned int, "unsigned int", std::stoul)

    /*!
     * \brief
     *      Template specialization for long argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(long, "long", std::stol)

    /*!
     * \brief
     *      Template specialization for unsigned long argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(unsigned long, "unsigned long", std::stoul)

    /*!
     * \brief
     *      Template specialization for long long argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(long long, "long long", std::stoll)

    /*!
     * \brief
     *      Template specialization for unsigned long long argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(unsigned long long, "unsigned long long", std::stoull)

    /*!
     * \brief
     *      Template specialization for float argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(float, "float", std::stof)

    /*!
     * \brief
     *      Template specialization for double argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(double, "double", std::stod)

    /*!
     * \brief
     *      Template specialization for long double argument parsing
     */
    ARG_PARSE_GENERAL_SPEC(long double, "long double", std::stold)

    /*!
     * \brief
     *      Template specialization for vector argument parsing
     * \tparam T
     *      Type that the vector holds
     *
     */
    template<typename T>
    struct CSYS_API ArgumentParser<std::vector<T>>
    {
        /*!
         * \brief
         *      Grabs a vector argument of type T from 'input' starting from 'start'
         * \param input
         *      Input to the command for this class to parse its argument
         * \param start
         *      Start of this argument
         */
        ArgumentParser(String &input, size_t &start);

        std::vector<T> m_Value;    //!< Vector of data parsed
    };

    /*!
     * \brief
     *      Grabs a vector argument of type T from 'input' starting from 'start'
     * \tparam T
     *      Type of vector
     * \param input
     *      Input to the command for this class to parse its argument
     * \param start
     *      Start of this argument
     */
    template<typename T>
    ArgumentParser<std::vector<T>>::ArgumentParser(String &input, size_t &start)
    {
        // Clean out vector before use
        m_Value.clear();

        // Grab the start of the vector argument
        auto range = input.NextPoi(start);

        // Empty
        if (range.first == input.End()) return;

        // Not starting with [
        if (input.m_String[range.first] != '[')
            throw Exception("Invalid vector argument missing opening [", ARG_PARSE_SUBSTR(range));

        // Erase [
        input.m_String[range.first] = ' ';
        while (true)
        {
            // Get next argument in vector
            range = input.NextPoi(range.first);

            // No more, empty vector
            if (range.first == input.End()) return;

            // Is a nested vector, go deeper
            else if (input.m_String[range.first] == '[')
                m_Value.push_back(ArgumentParser<T>(input, range.first).m_Value);
            else
            {
                // Find first non-escaped ]
                range.second = input.m_String.find(']', range.first);
                while (range.second != std::string::npos && Reserved::IsEscaped(input.m_String, range.second))
                    range.second = input.m_String.find(']', range.second + 1);

                // Check for closing ]
                if (range.second == std::string::npos)
                {
                    range.second = input.m_String.size();
                    throw Exception("Invalid vector argument missing closing ]", ARG_PARSE_SUBSTR(range));
                }

                // Erase ]
                input.m_String[range.second] = ' ';
                start = range.first;

                // Parse all arguments contained within the vector
                while (true)
                {
                    // If end of parsing, get out
                    if ((range.first = input.NextPoi(range.first).first) >= range.second)
                    {
                        start = range.first;
                        return;
                    }

                    // Parse argument and go to next
                    m_Value.push_back(ArgumentParser<T>(input, start).m_Value);
                    range.first = start;
                }
            }
        }
    }
}

#endif //CSYS_ARGUMENT_PARSER_H
