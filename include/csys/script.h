// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_SCRIPT_H
#define CSYS_SCRIPT_H
#pragma once

#include <string>
#include <vector>
#include "csys/api.h"

namespace csys
{
    class CSYS_API Script
    {
    public:

        /*!
         * \brief
         *      Create script object from file
         * \param path
         *      Path of script file
         * \param load_on_init
         *      Load script when object is created
         */
        explicit Script(std::string path, bool load_on_init = true);

        /*!
         * \brief
         *      Create script object from file
         * \param path
         *      Path of script file
         * \param load_on_init
         *      Load script when object is created
         */
        explicit Script(const char *path, bool load_on_init = true);

        /*!
         * \brief
         *      Create script object from file already in memory
         * \param data
         *      Script file memory
         */
        explicit Script(std::vector<std::string> data);

        /*!
         * \brief
         *      Move constructor
         * \param rhs
         *      Script to be copied.
         */
        Script(Script &&rhs) = default;

        /*!
         * \brief
         *      Copy constructor
         * \param rhs
         *      Script to be copied.
         */
        Script(const Script &rhs) = default;

        /*!
         * \brief
         *      Move assignment operator
         * \param rhs
         *      Script to be copied.
         */
        Script &operator=(Script &&rhs) = default;

        /*!
         * \brief
         *      Copy assigment operator.
         * \param rhs
         *      Script to be copied.
         */
        Script &operator=(const Script &rhs) = default;
        
        /*!
         * \brief Load script file
         */
        void Load();

        /*!
         * \brief Reload script file (Unload & Load)
         */
        void Reload();

        /*!
         * /brief Clear script data
         */
        void Unload();

        /*!
         * \brief
         *      Set script file path (Will be used when laoding)
         * \param path
         *      Script file path
         */
        void SetPath(std::string path);

        /*!
         * \brief
         *      Retrieve script data (Commands)
         * \return
         *      List of commands in script
         */
        const std::vector<std::string> &Data();

    protected:
        std::vector<std::string> m_Data;    //!< Commands in script
        std::string m_Path;                 //!< Path of script file
        bool m_FromMemory;                  //!< Flag to specify if script was loaded from file or memory
    };
}

#ifdef CSYS_HEADER_ONLY
#include "csys/script.inl"
#endif

#endif
