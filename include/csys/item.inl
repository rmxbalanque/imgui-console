// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef CSYS_HEADER_ONLY

#include "csys/item.h"

#endif

#include <chrono>

namespace csys
{

    ///////////////////////////////////////////////////////////////////////////
    // Console Item ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    CSYS_INLINE static const std::string_view s_Command = "> ";
    CSYS_INLINE static const std::string_view s_Warning = "\t[WARNING]: ";
    CSYS_INLINE static const std::string_view s_Error = "[ERROR]: ";
    CSYS_INLINE static const auto s_TimeBegin = std::chrono::steady_clock::now();

    CSYS_INLINE Item::Item(ItemType type) : m_Type(type)
    {
        auto timeNow = std::chrono::steady_clock::now();
        m_TimeStamp = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - s_TimeBegin).count());
    }

    CSYS_INLINE Item &Item::operator<<(const std::string_view str)
    {
        m_Data.append(str);
        return *this;
    }

    CSYS_INLINE std::string Item::Get() const
    {
        switch (m_Type)
        {
            case COMMAND:
                return s_Command.data() + m_Data;
            case LOG:
                return '\t' + m_Data;
            case WARNING:
                return s_Warning.data() + m_Data;
            case ERROR:
                return s_Error.data() + m_Data;
            case INFO:
                return m_Data;
            case NONE:
            default:
                return "";
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // Console Item Log ///////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

#define LOG_BASIC_TYPE_DEF(type)\
    CSYS_INLINE ItemLog& ItemLog::operator<<(type data)\
    {\
        m_Items.back() << std::to_string(data);\
        return *this;\
    }

    CSYS_INLINE ItemLog &ItemLog::log(ItemType type)
    {
        // New item.
        m_Items.emplace_back(type);
        return *this;
    }

    CSYS_INLINE std::vector<Item> &ItemLog::Items()
    {
        return m_Items;
    }

    CSYS_INLINE void ItemLog::Clear()
    {
        m_Items.clear();
    }

    CSYS_INLINE ItemLog &ItemLog::operator<<(const std::string_view data)
    {
        m_Items.back() << data;
        return *this;
    }

    CSYS_INLINE ItemLog &ItemLog::operator<<(const char data)
    {
        m_Items.back().m_Data.append(1, data);
        return *this;
    }

    // Basic type operator definitions.
    LOG_BASIC_TYPE_DEF(int)

    LOG_BASIC_TYPE_DEF(long)

    LOG_BASIC_TYPE_DEF(float)

    LOG_BASIC_TYPE_DEF(double)

    LOG_BASIC_TYPE_DEF(long long)

    LOG_BASIC_TYPE_DEF(long double)

    LOG_BASIC_TYPE_DEF(unsigned int)

    LOG_BASIC_TYPE_DEF(unsigned long)

    LOG_BASIC_TYPE_DEF(unsigned long long)
}