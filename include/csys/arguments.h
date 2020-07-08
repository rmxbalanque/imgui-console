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
     *      Macro for supporting trivial types
     */
#define SUPPORT_TYPE(TYPE, TYPE_NAME)\
    template<> struct is_supported_type<TYPE> { static constexpr bool value = true; }; \
    template<> \
    struct CSYS_API ArgData<TYPE> \
    { \
      explicit ArgData(String name) : m_Name(std::move(name)), m_Value() {} \
      const String m_Name; \
      String m_TypeName = TYPE_NAME; \
      TYPE m_Value; \
    };

    using NULL_ARGUMENT = void (*)();    //!< Null argument typedef

    /*!
     * \brief
     *      Base case struct where a type is not supported
     */
    template<typename> struct is_supported_type { static constexpr bool value = false; };

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

    //! Supported types
    SUPPORT_TYPE(String, "String")

    SUPPORT_TYPE(bool, "Boolean")

    SUPPORT_TYPE(char, "Char")

    SUPPORT_TYPE(unsigned char, "Unsigned_Char")

    SUPPORT_TYPE(short, "Signed_Short")

    SUPPORT_TYPE(unsigned short, "Unsigned_Short")

    SUPPORT_TYPE(int, "Signed_Int")

    SUPPORT_TYPE(unsigned int, "Unsigned_Int")

    SUPPORT_TYPE(long, "Signed_Long")

    SUPPORT_TYPE(unsigned long, "Unsigned_Long")

    SUPPORT_TYPE(long long, "Signed_Long_Long")

    SUPPORT_TYPE(unsigned long long, "Unsigned_Long_Long")

    SUPPORT_TYPE(float, "Float")

    SUPPORT_TYPE(double, "Double")

    SUPPORT_TYPE(long double, "Long_Double")

    //! Supported containers
    template<typename U> struct is_supported_type<std::vector<U>> { static constexpr bool value = is_supported_type<U>::value; };
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

    /*!
     * \brief
     *      Wrapper around an argument for use of parsing a command line
     * \tparam T
     *      Data type
     */
    template<typename T>
    struct CSYS_API Arg
    {
        /*!
         * \brief
         *      Is true if type of U is a supported type
         * \tparam U
         *      Type to check if it is supported
         */
        template<typename U>
        static constexpr bool is_supported_type_v = is_supported_type<U>::value;
    public:

        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;    //!< Type of this argument

        /*!
         * \brief
         *      Constructor for an argument for naming
         * \param name
         *      Name of the argument
         */
        explicit Arg(const String &name) : m_Arg(name)
        {
            static_assert(is_supported_type_v<ValueType>,
                    "ValueType 'T' is not supported, see 'Supported types' for more help");
        }

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
}

#endif //CSYS_ARGUMENTS_H
