// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_ITEM_H
#define CSYS_ITEM_H
#pragma once

#include <vector>
#include <string>
#include "csys/api.h"

namespace csys
{
    static const char endl = '\n';

    /*!
     * \brief
     *      Console item type:
     *          - Command: Only used for commands.
     *          - Log: Used to log information inside a command.
     *          - Warning: Warn client through console.
     *          - Error: Display error to client through console.
     *          - Info: Any information wished to display through console.
     *          - None: Empty console item.
     */
    enum ItemType
    {
        COMMAND = 0,
        LOG,
        WARNING,
        ERROR,
        INFO,
        NONE
    };

    struct CSYS_API Item
    {
        /*!
         * \brief
         *      Create console item type
         * \param type
         *      Item type to be stored
         */
        explicit Item(ItemType type = ItemType::LOG);

        /*!
         * \brief
         *      Move constructor
         * \param rhs
         *      Item to be copied.
         */
        Item(Item &&rhs) = default;

        /*!
         * \brief
         *      Copy constructor
         * \param rhs
         *      Item to be copied.
         */
        Item(const Item &rhs) = default;

        /*!
         * \brief
         *      Move assignment operator
         * \param rhs
         *      Item to be copied.
         */
        Item &operator=(Item &&rhs) = default;

        /*!
         * \brief
         *      Copy assigment operator.
         * \param rhs
         *      Item to be copied.
         */
        Item &operator=(const Item &rhs) = default;

        /*!
         * \brief
         *      Log data to console item
         * \param str
         *      Append string to item data
         * \return
         *      Self (To allow for fluent logging)
         */
        Item &operator<<(std::string_view str);

        /*!
         * \brief
         *      Get final/styled string of the item
         * \return
         *      Stylized item string
         */
        [[nodiscard]] std::string Get() const;

        ItemType m_Type;             //!< Console item type
        std::string m_Data;          //!< Item string data
        unsigned int m_TimeStamp;    //!< Record timestamp
    };

#define LOG_BASIC_TYPE_DECL(type) ItemLog& operator<<(type data)

    class CSYS_API ItemLog
    {
    public:

        /*!
         * \brief
         *      Log console item
         * \param type
         *      Type of item to log
         * \return
         *      Self (To allow for fluent logging)
         */
        ItemLog &log(ItemType type);

        ItemLog() = default;

        /*!
         * \brief
         *      Move constructor
         * \param rhs
         *      ItemLog to be copied.
         */
        ItemLog(ItemLog &&rhs) = default;

        /*!
         * \brief
         *      Copy constructor
         * \param rhs
         *      ItemLog to be copied.
         */
        ItemLog(const ItemLog &rhs) = default;

        /*!
         * \brief
         *      Move assignment operator
         * \param rhs
         *      ItemLog to be copied.
         */
        ItemLog &operator=(ItemLog &&rhs) = default;

        /*!
         * \brief
         *      Copy assigment operator.
         * \param rhs
         *      ItemLog to be copied.
         */
        ItemLog &operator=(const ItemLog &rhs) = default;

        /*!
         * \brief
         *      Get logged console items
         * \return
         *      Console log
         */
        std::vector<Item> &Items();

        /*!
         * \brief Delete console item log history
         */
        void Clear();

        LOG_BASIC_TYPE_DECL(int);

        LOG_BASIC_TYPE_DECL(long);

        LOG_BASIC_TYPE_DECL(float);

        LOG_BASIC_TYPE_DECL(double);

        LOG_BASIC_TYPE_DECL(long long);

        LOG_BASIC_TYPE_DECL(long double);

        LOG_BASIC_TYPE_DECL(unsigned int);

        LOG_BASIC_TYPE_DECL(unsigned long);

        LOG_BASIC_TYPE_DECL(unsigned long long);

        LOG_BASIC_TYPE_DECL(std::string_view);

        LOG_BASIC_TYPE_DECL(char);

    protected:
        std::vector<Item> m_Items;
    };
}

#ifdef CSYS_HEADER_ONLY

#include "csys/item.inl"

#endif

#endif
