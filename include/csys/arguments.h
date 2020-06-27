// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_ARGUMENTS_H
#define CSYS_ARGUMENTS_H
#pragma once

#include "csys/api.h"
#include "csys/string.h"
#include "csys/exceptions.h"
#include "csys/argument_parser.h"
#include <vector>

namespace csys
{
    /*!
     * \brief
     *      Wrapper around a given data type to name it
     * \tparam T
     *      Type of data that must have a default constructor
     */
    template<typename T>
    struct CSYS_API ArgData
    {
        /*!
         * \brief
         *      Non-default constructor
         * \param name
         *      Name of the argument
         */
        explicit ArgData(String name) : m_Name(std::move(name)), m_Value()
        { }

        const String m_Name = "";                  //!< Name of argument
        String m_TypeName = "Unsupported Type";    //!< Name of type
        T m_Value;                                 //!< Actual value
    };

    /*!
     * \brief
     *      Wrapper around an argument for use of parsing a command line
     * \tparam T
     *      Data type
     */
    template<typename T>
    struct CSYS_API Arg
    {
        using ValueType = T;    //!< Type of this argument

        /*!
         * \brief
         *      Constructor for an argument for naming
         * \param name
         *      Name of the argument
         */
        explicit Arg(const String &name) : m_Arg(name)
        {}

        /*!
         * \brief
         *      Grabs its own argument from the command line and sets its value
         * \param input
         *      Command line argument list
         * \param start
         *      Start of its argument
         * \return
         *      Returns this
         */
        Arg<T> &Parse(String &input, size_t &start)
        {
            size_t index = start;

            // Check if there are more arguments to be read in
            if (input.NextPoi(index).first == input.End())
                throw Exception("Not enough arguments were given", input.m_String);
            // Set value grabbed from input aka command line argument
            m_Arg.m_Value = ArgumentParser<ValueType>(input, start).m_Value;
            return *this;
        }

        /*!
         * \brief
         *      Gets the info of the argument in the form of [name:type]
         * \return
         *      Returns a string containing the arugment's info
         */
        std::string Info()
        {
            return std::string(" [") + m_Arg.m_Name.m_String + ":" + m_Arg.m_TypeName.m_String + "]";
        }

        ArgData<ValueType> m_Arg;    //!< Data relating to this argument
    };

    using NULL_ARGUMENT = void (*)();    //!< Null argument typedef

    /*!
     * \brief
     *      Template specialization for a null argument that gets appended to a command's argument list to check if more
     *      than the required number of arguments
     */
    template<>
    struct CSYS_API Arg<NULL_ARGUMENT>
    {
        /*!
         * \brief
         *      Checks if the input starting from param 'start' is all whitespace or not
         * \param input
         *      Command line argument list
         * \param start
         *      Start of its argument
         * \return
         *      Returns this
         */
        Arg<NULL_ARGUMENT> &Parse(String &input, size_t &start)
        {
            if (input.NextPoi(start).first != input.End())
                throw Exception("Too many arguments were given", input.m_String);
            return *this;
        }
    };

    /*!
     * \brief
     *      Generalized macro for specialization's of built-in types
     */
#define ARG_BASE_SPEC(TYPE, TYPE_NAME) \
  template<>\
  struct CSYS_API ArgData<TYPE> \
  { \
    explicit ArgData(String name) : m_Name(std::move(name)), m_Value() {} \
    const String m_Name; \
    String m_TypeName = TYPE_NAME; \
    TYPE m_Value; \
  };

    /*!
     * \brief
     *      Template specialization for string argument
     */
    ARG_BASE_SPEC(String, "String")

    /*!
     * \brief
     *      Template specialization for bool argument
     */
    ARG_BASE_SPEC(bool, "Boolean")

    /*!
     * \brief
     *      Template specialization for char argument
     */
    ARG_BASE_SPEC(char, "Char")

    /*!
     * \brief
     *      Template specialization for unsigned char argument
     */
    ARG_BASE_SPEC(unsigned char, "Unsigned_Char")

    /*!
     * \brief
     *      Template specialization for short argument
     */
    ARG_BASE_SPEC(short, "Signed_Short")

    /*!
     * \brief
     *      Template specialization for unsigned short argument
     */
    ARG_BASE_SPEC(unsigned short, "Unsigned_Short")

    /*!
     * \brief
     *      Template specialization for int argument
     */
    ARG_BASE_SPEC(int, "Signed_Int")

    /*!
     * \brief
     *      Template specialization for unsigned int argument
     */
    ARG_BASE_SPEC(unsigned int, "Unsigned_Int")

    /*!
     * \brief
     *      Template specialization for unsigned long argument
     */
    ARG_BASE_SPEC(long, "Signed_Long")

    /*!
     * \brief
     *      Template specialization for unsigned long argument
     */
    ARG_BASE_SPEC(unsigned long, "Unsigned_Long")

    /*!
     * \brief
     *      Template specialization for long long argument
     */
    ARG_BASE_SPEC(long long, "Signed_Long_Long")

    /*!
     * \brief
     *      Template specialization for unsigned long long argument
     */
    ARG_BASE_SPEC(unsigned long long, "Unsigned_Long_Long")

    /*!
     * \brief
     *      Template specialization for float argument
     */
    ARG_BASE_SPEC(float, "Float")

    /*!
     * \brief
     *      Template specialization for double argument
     */
    ARG_BASE_SPEC(double, "Double")

    /*!
     * \brief
     *      Template specialization for long double argument
     */
    ARG_BASE_SPEC(long double, "Long_Double")

    /*!
     * \brief
     *      Template specialization for a vector of N vector of types argument
     * \tparam T
     *      Type of vector
     */
    template<typename T>
    struct CSYS_API ArgData<std::vector<T>>
    {
        /*!
         * \brief
         *      Constructor for a vector argument
         * \param name
         *      Name for argument
         */
        explicit ArgData(String name) : m_Name(std::move(name))
        {}

        const String m_Name;                                                                   //!< Name of argument
        String m_TypeName = std::string("Vector_Of_") + ArgData<T>("").m_TypeName.m_String;    //!< Type name
        std::vector<T> m_Value;                                                                //!< Vector of data
    };
}

#endif //CSYS_ARGUMENTS_H
