// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_STRING_H
#define CSYS_STRING_H
#pragma once

#include <string>
#include <cctype>
#include "csys/api.h"

namespace csys
{
    /*!
     * \brief
     *      Wrapper around std::string to be used with parsing a string argument
     */
    struct CSYS_API String
    {
        /*!
         * \brief
         *      Default constructor
         */
        String() = default;

        /*!
         * \brief
         *      Conversion constructor from c-style string. A deep copy is made
         * \param str
         *      String to be converted
         */
        String(const char *str [[maybe_unused]]) : m_String(str ? str : "")
        {}

        /*!
         * \brief
         *      Conversion constructor from an std::string to csys::String. A deep copy is made
         * \param str
         *      String to be converted
         */
        String(std::string str) : m_String(std::move(str))
        {}

        /*!
         * \brief
         *      Conversion constructor from csys::String to a c-style string
         * \return
         *      Returns a pointer to the string contained within this string
         */
        operator const char *()
        { return m_String.c_str(); }

        /*!
         * \brief
         *      Conversion constructor from csys::String to an std::string
         * \return
         *      Returns a copy of this string
         */
        operator std::string()
        { return m_String; }

        /*!
         * \brief
         *      Moves until first non-whitespace char, and continues until the end of the string or a whitespace has is
         *      found
         * \param start
         *      Where to start scanning from. Will be set to pair.second
         * \return
         *      Returns the first element and one passed the end of non-whitespace. In other words [first, second)
         */
        std::pair<size_t, size_t> NextPoi(size_t &start) const
        {
            size_t end = m_String.size();
            std::pair<size_t, size_t> range(end + 1, end);
            size_t pos = start;

            // Go to the first non-whitespace char
            for (; pos < end; ++pos)
                if (!std::isspace(m_String[pos]))
                {
                    range.first = pos;
                    break;
                }

            // Go to the first whitespace char
            for (; pos < end; ++pos)
                if (std::isspace(m_String[pos]))
                {
                    range.second = pos;
                    break;
                }

            start = range.second;
            return range;
        }

        /*!
         * \brief
         *      Returns a value to be compared with the .first of the pair returned from NextPoi
         * \return
         *      Returns size of string + 1
         */
        [[nodiscard]] size_t End() const
        { return m_String.size() + 1; }

        std::string m_String;    //!< String data member
    };
}

#endif //CSYS_CSYS_STRING_H
