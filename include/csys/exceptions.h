// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_EXCEPTIONS_H
#define CSYS_EXCEPTIONS_H
#pragma once

#include <string>
#include <exception>
#include <utility>
#include "csys/api.h"

namespace csys
{
    /*!
     * \brief
     *      Csys exception extended from the stl for any issues
     */
    class CSYS_API Exception : public std::exception
    {
    public:
        /*!
         * \brief
         *      Basic constructor that makes a message with an arg
         * \param message
         *      Message describing what went wrong
         * \param arg
         *      What went wrong (will get put between ' ')
         */
        explicit Exception(const std::string &message, const std::string &arg) : m_Msg(message + ": '" + arg + "'")
        {}

        /*!
         * \brief
         *      Constructor for one string
         * \param message
         *      Message describing what went wrong
         */
        explicit Exception(std::string message) : m_Msg(std::move(message))
        {}

        /*!
         * \brief
         *      Default destructor
         */
        ~Exception() override = default;

        /*!
         * \brief
         *      Gets what happened
         * \return
         *      Returns a c-style string of what happened
         */
        [[nodiscard]] const char *what() const noexcept override
        {
            return m_Msg.c_str();
        }

    protected:
        std::string m_Msg;    //!< Message of what happened
    };
}

#endif //CSYS_CSYS_EXCEPTIONS_H
