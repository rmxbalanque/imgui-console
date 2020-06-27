// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_COMMAND_H
#define CSYS_COMMAND_H
#pragma once

#include <functional>
#include <type_traits>
#include <utility>
#include "csys/arguments.h"
#include "csys/exceptions.h"
#include "csys/item.h"

namespace csys
{
    /*!
     * \brief
     *      Non-templated class that allows for the storage of commands as well as accessing certain functionality of
     *      said commands.
     */
    struct CommandBase
    {
        /*!
         * \brief
         *      Default virtual destructor
         */
        virtual ~CommandBase() = default;

        /*!
         * \brief
         *      Parses and runs the function held within the child class
         * \param input
         *      String of arguments for the command to parse and pass to the function
         * \return
         *      Returns item error if the parsing in someway was messed up, and none if there was no issue
         */
        virtual Item operator()(String &input) = 0;

        /*!
         * \brief
         *      Gets info about the command and usage
         * \return
         *      String containing info about the command
         */
        [[nodiscard]] virtual std::string Help() = 0;

        /*!
         * \brief
         *      Getter for the number of arguments the command takes
         * \return
         *      Returns the number of arguments taken by the command
         */
        [[nodiscard]] virtual size_t ArgumentCount() const = 0;

        /*!
         * \brief
         *      Deep copies a command
         * \return
         *      Pointer to newly copied command
         */
        [[nodiscard]] virtual CommandBase* Clone() const = 0;
    };

    /*!
     * \brief
     *      Base template for a command that takes N amount of arguments
     * \tparam Fn
     *      Decltype of function to be called when command is parsed and ran
     * \tparam Args
     *      Argument type list that is proportional to the argument list of the function Fn
     */
    template<typename Fn, typename ...Args>
    class CSYS_API Command : public CommandBase
    {
    public:
        /*!
         * \brief
         *      Constructor that sets the name, description, function and arguments as well as add a null argument
         *      for parsing
         * \param name
         *      Name of the command to call by
         * \param description
         *      Info about the command
         * \param function
         *      Function to run when command is called
         * \param args
         *      Arguments to be used for parsing and passing into the function. Must be of type "Arg<T>"
         */
        Command(String name, String description, Fn function, Args... args) : m_Name(std::move(name)),
                                                                              m_Description(std::move(description)),
                                                                              m_Function(function),
                                                                              m_Arguments(args..., Arg<NULL_ARGUMENT>())
        {}

        /*!
         * \brief
         *      Parses and runs the function m_Function
         * \param input
         *      String of arguments for the command to parse and pass to the function
         * \return
         *      Returns item error if the parsing in someway was messed up, and none if there was no issue
         */
        Item operator()(String &input) final
        {
            try
            {
                // Try to parse and call the function
                constexpr int argumentSize = sizeof... (Args);
                Call(input, std::make_index_sequence<argumentSize + 1>{}, std::make_index_sequence<argumentSize>{});
            }
            catch (Exception &ae)
            {
                // Error happened with parsing
                return Item(ERROR) << (m_Name.m_String + ": " + ae.what());
            }
            return Item(NONE);
        }

        /*!
         * \brief
         *      Gets info about the command and usage
         * \return
         *      String containing info about the command
         */
        [[nodiscard]] std::string Help() final
        {
            return m_Name.m_String + DisplayArguments(std::make_index_sequence<sizeof ...(Args)>{}) + "\n\t\t- " +
                   m_Description.m_String + "\n\n";
        }

        /*!
         * \brief
         *      Getter for the number of arguments the command takes
         * \return
         *      Returns the number of arguments taken by the command
         */
        [[nodiscard]] size_t ArgumentCount() const final
        {
            return sizeof... (Args);
        }

        /*!
         * \brief
         *      Deep copies a command
         * \return
         *      Pointer to newly copied command
         */
        [[nodiscard]] CommandBase* Clone() const final
        {
            return new Command<Fn, Args...>(*this);
        }
    private:
        /*!
         * \brief
         *      Parses arguments and passes them into the command to be ran
         * \tparam Is_p
         *      Index sequence from 0 to Argument Count + 1, used for parsing
         * \tparam Is_c
         *      Index sequence from 0 to Argument Count, used for passing into the function
         * \param input
         *      String of arguments to be parsed
         */
        template<size_t... Is_p, size_t... Is_c>
        void Call(String &input, const std::index_sequence<Is_p...> &, const std::index_sequence<Is_c...> &)
        {
            size_t start = 0;

            // Parse arguments
            int _[]{0, (void(std::get<Is_p>(m_Arguments).Parse(input, start)), 0)...};
            (void) (_);

            // Call function with unpacked tuple
            m_Function((std::get<Is_c>(m_Arguments).m_Arg.m_Value)...);
        }

        /*!
         * \brief
         *      Displays the usage for running the command successfully
         * \tparam Is
         *      Index sequence from 0 to Argument CountH
         * \return
         *      Returns a string containing the usage of the command
         */
        template<size_t ...Is>
        std::string DisplayArguments(const std::index_sequence<Is...> &)
        {
            return (std::get<Is>(m_Arguments).Info() + ...);
        }

        const String m_Name;                                            //!< Name of command
        const String m_Description;                                     //!< Description of the command
        std::function<void(typename Args::ValueType...)> m_Function;    //!< Function to be invoked as command
        std::tuple<Args..., Arg<NULL_ARGUMENT>> m_Arguments;            //!< Arguments to be passed into m_Function
    };

    /*!
     * \brief
     *      Template specialization for a command that doesn't take any arguments
     * \tparam Fn
     *      Decltype of function to be called when the command is invoked
     */
    template<typename Fn>
    class CSYS_API Command<Fn> : public CommandBase
    {
    public:
        /*!
         * \brief
         *      Constructor that sets the name, description and function. A null argument will be added to the arguments
         *      for parsing
         * \param name
         *      Name of the command to call by
         * \param description
         *      Info about the command
         * \param function
         *      Function to run when command is called
         */
        Command(String name, String description, Fn function) : m_Name(std::move(name)),
                                                                m_Description(std::move(description)),
                                                                m_Function(function), m_Arguments(Arg<NULL_ARGUMENT>())
        {}

        /*!
         * \brief
         *      Parses and runs the function m_Function
         * \param input
         *      String of arguments for the command to parse and pass to the function. This should be empty
         * \return
         *      Returns item error if the parsing in someway was messed up, and none if there was no issue
         */
        Item operator()(String &input) final
        {
            // call the function
            size_t start = 0;
            try
            {
                // Check to see if input is all whitespace
                std::get<0>(m_Arguments).Parse(input, start);
            }
            catch (Exception &ae)
            {
                // Command had something passed into it
                return Item(ERROR) << (m_Name.m_String + ": " + ae.what());
            }

            // Call function
            m_Function();
            return Item(NONE);
        }

        /*!
         * \brief
         *      Gets info about the command and usage
         * \return
         *      String containing info about the command
         */
        [[nodiscard]] std::string Help() final
        {
            return m_Name.m_String + "\n\t\t- " + m_Description.m_String + "\n\n";
        }

        /*!
         * \brief
         *      Getter for the number of arguments the command takes
         * \return
         *      0
         */
        [[nodiscard]] size_t ArgumentCount() const final
        {
            return 0;
        }

        /*!
         * \brief
         *      Deep copies a command
         * \return
         *      Pointer to newly copied command
         */
        [[nodiscard]] CommandBase* Clone() const final
        {
            return new Command<Fn>(*this);
        }
    private:

        const String m_Name;                           //!< Name of command
        const String m_Description;                    //!< Description of the command
        std::function<void(void)> m_Function;          //!< Function to be invoked as command
        std::tuple<Arg<NULL_ARGUMENT>> m_Arguments;    //!< Arguments to be passed into m_Function
    };
}

#endif //CSYS_COMMAND_H
