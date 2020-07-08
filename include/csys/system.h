// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_SYSTEM_H
#define CSYS_SYSTEM_H

#pragma once

#include "csys/command.h"
#include "csys/autocomplete.h"
#include "csys/history.h"
#include "csys/item.h"
#include "csys/script.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace csys
{
    class CSYS_API System
    {
    public:

        /*!
         * \brief Initialize system object
         */
        System();

        /*!
         * \brief
         *      Move constructor
         * \param rhs
         *      System to be copied.
         */
        System(System &&rhs) = default;

        /*!
         * \brief
         *      Copy constructor
         * \param rhs
         *      System to be copied.
         */
        System(const System &rhs);

        /*!
         * \brief
         *      Move assignment operator
         * \param rhs
         *      System to be copied.
         */
        System &operator=(System &&rhs) = default;

        /*!
         * \brief
         *      Copy assigment operator.
         * \param rhs
         *      System to be copied.
         */
        System &operator=(const System &rhs);
        
        /*!
         * \brief
         *      Parse given command line input and run it
         * \param line
         *      Command line string
         */
        void RunCommand(const std::string &line);

        /*!
         * \brief
         *      Get console registered command autocomplete tree
         * \return
         *      Autocomplete Ternary Search Tree
         */
        AutoComplete &CmdAutocomplete();

        /*!
         * \brief
         *      Get console registered variables autocomplete tree
         * \return
         *      Autocomplete Ternary Search Tree
         */
        AutoComplete &VarAutocomplete();

        /*!
         * \brief
         *      Get command history container
         * \return
         *      Command history vector
         */
        CommandHistory &History();

        /*!
         * \brief
         *      Get console items
         * \return
         *      Console items container
         */
        std::vector<Item> &Items();

        /*!
         * \brief
         *      Creates a new item entry to log information
         * \param type
         *      Log type (COMMAND, LOG, WARNING, ERROR)
         * \return
         *      Reference to console items obj
         */
        ItemLog &Log(ItemType type = ItemType::LOG);

        /*!
         * \brief
         *      Run the given script
         * \param script_name
         *      Script to be executed
         *
         *  \note
         *      If script exists but its not loaded, this methods will load the script and proceed to run it.
         */
        void RunScript(const std::string &script_name);

        /*!
         * \brief
         *      Get registered command container
         * \return
         *      Commands container
         */
        std::unordered_map<std::string, std::unique_ptr<CommandBase>> &Commands();

        /*!
         * \brief
         *      Get registered scripts container
         * \return
         *      Scripts container
         */
        std::unordered_map<std::string, std::unique_ptr<Script>> &Scripts();

        /*!
         * \brief
         *      Registers a command within the system to be invokable
         * \tparam Fn
         *      Decltype of the function to invoke when command is ran
         * \tparam Args
         *      List of arguments that match that of the argument list within the function Fn of type csys::Arg<T>
         * \param name
         *      Non-whitespace separating name of the command. Whitespace will be dropped
         * \param description
         *      Description describing what the command does
         * \param function
         *      A non-member function to run when command is called
         * \param args
         *      List of csys::Arg<T>s that matches that of the argument list of 'function'
         */
        template<typename Fn, typename ...Args>
        void RegisterCommand(const String &name, const String &description, Fn function, Args... args)
        {
            // Check if function can be called with the given arguments and is not part of a class
            static_assert(std::is_invocable_v<Fn, typename Args::ValueType...>, "Arguments specified do not match that of the function");
            static_assert(!std::is_member_function_pointer_v<Fn>, "Non-static member functions are not allowed");

            // Move to command
            size_t name_index = 0;
            auto range = name.NextPoi(name_index);

            // Command already registered
            if (m_Commands.find(name.m_String) != m_Commands.end())
                throw csys::Exception("ERROR: Command already exists");

            // Check if command has a name
            else if (range.first == name.End())
            {
                Log(ERROR) << "Empty command name given" << csys::endl;
                return;
            }

            // Get command name
            std::string command_name = name.m_String.substr(range.first, range.second - range.first);

            // Command contains more than one word
            if (name.NextPoi(name_index).first != name.End())
                throw csys::Exception("ERROR: Whitespace separated command names are forbidden");

            // Register for autocomplete.
            if (m_RegisterCommandSuggestion)
            {
                m_CommandSuggestionTree.Insert(command_name);
                m_VariableSuggestionTree.Insert(command_name);
            }

            // Add commands to system
            m_Commands[name.m_String] = std::make_unique<Command<Fn, Args...>>(name, description, function, args...);

            // Make help command for command just added
            auto help = [this, command_name]() {
                Log(LOG) << m_Commands[command_name]->Help() << csys::endl;
            };

            m_Commands["help " + command_name] = std::make_unique<Command<decltype(help)>>("help " + command_name,
                                                                                           "Displays help info about command " +
                                                                                           command_name, help);
        }

        /*!
         * \brief
         *      Register's a variable within the system
         * \tparam T
         *      Type of the variable
         * \tparam Types
         *      Type of arguments that type T can be constructed with
         * \param name
         *      Name of the variable
         * \param var
         *      The variable to register
         * \param args
         *      List of csys::Arg to be used for the construction of type T
         * \note
         *      Type T requires an assignment operator, and constructor that takes type 'Types...'
         *      Param 'var' is assumed to have a valid life-time up until it is unregistered or the program ends
         */
        template<typename T, typename ...Types>
        void RegisterVariable(const String &name, T &var, Arg<Types>... args)
        {
            static_assert(std::is_constructible_v<T, Types...>, "Type of var 'T' can not be constructed with types of 'Types'");
            static_assert(sizeof... (Types) != 0, "Empty variadic list");

            // Register get command
            auto var_name = RegisterVariableAux(name, var);

            // Register set command
            auto setter = [&var](Types... params){ var = T(params...); };
            m_Commands["set " + var_name] = std::make_unique<Command<decltype(setter), Arg<Types>...>>("set " + var_name,
                                                                                        "Sets the variable " + var_name,
                                                                                        setter, args...);
        }

        /*!
         * \brief
         *      Register's a variable within the system
         * \tparam T
         *      Type of the variable
         * \tparam Types
         *      Type of arguments that type T can be constructed with
         * \param name
         *      Name of the variable
         * \param var
         *      The variable to register
         * \param setter
         *      Custom setter that runs when command set 'name' is invoked
         * \note
         *      The setter must have dceltype of void(decltype(var)&, Types...)
         *      Param 'var' is assumed to have a valid life-time up until it is unregistered or the program ends
         */
        template<typename T, typename ...Types>
        void RegisterVariable(const String &name, T &var, void(*setter)(T&, Types...))
        {
            // Register get command
            auto var_name = RegisterVariableAux(name, var);

            // Register set command
            auto setter_l = [&var, setter](Types... args){ setter(var, args...); };
            m_Commands["set " + var_name] = std::make_unique<Command<decltype(setter_l), Arg<Types>...>>("set " + var_name,
                                                                                        "Sets the variable " + var_name,
                                                                                         setter_l, Arg<Types>("")...);
        }

        /*!
         * \brief
         *      Register script into console system
         * \param name
         *      Script name
         * \param path
         *      Scrip path
         */
        void RegisterScript(const std::string &name, const std::string &path);

        /*!
         * \brief
         *      Unregister command from console system
         * \param cmd_name
         *      Command to unregister
         */
        void UnregisterCommand(const std::string &cmd_name);

        /*!
         * \brief
         *      Unregister variable from console system
         * \param var_name
         *      Variable to unregister
         */
        void UnregisterVariable(const std::string &var_name);

        /*!
         * \brief
         *      Unregister script from console system
         * \param script_name
         *      Script to unregister
         */
        void UnregisterScript(const std::string &script_name);

    protected:
        template<typename T>
        std::string RegisterVariableAux(const String &name, T &var)
        {
            // Disable.
            m_RegisterCommandSuggestion = false;

            // Make sure only one word was passed in
            size_t name_index = 0;
            auto range = name.NextPoi(name_index);
            if (name.NextPoi(name_index).first != name.End())
                throw csys::Exception("ERROR: Whitespace separated variable names are forbidden");

            // Get variable name
            std::string var_name = name.m_String.substr(range.first, range.second - range.first);

            // Get Command
            const auto GetFunction = [this, &var]() {
                m_ItemLog.log(LOG) << var << endl;
            };

            // Register get command
            m_Commands["get " + var_name] = std::make_unique<Command<decltype(GetFunction)>>("get " + var_name,
                                                                                             "Gets the variable " +
                                                                                             var_name, GetFunction);

            // Enable again.
            m_RegisterCommandSuggestion = true;

            // Register variable
            m_VariableSuggestionTree.Insert(var_name);

            return var_name;
        }

        void ParseCommandLine(const String &line);                                   //!< Parse command line and execute command

        std::unordered_map<std::string, std::unique_ptr<CommandBase>> m_Commands;    //!< Registered command container
        AutoComplete m_CommandSuggestionTree;                                        //!< Autocomplete Ternary Search Tree for commands
        AutoComplete m_VariableSuggestionTree;                                       //!< Autocomplete Ternary Search Tree for registered variables
        CommandHistory m_CommandHistory;                                             //!< History of executed commands
        ItemLog m_ItemLog;                                                           //!< Console Items (Logging)
        std::unordered_map<std::string, std::unique_ptr<Script>> m_Scripts;          //!< Scripts
        bool m_RegisterCommandSuggestion = true;                                     //!< Flag that determines if commands will be registered for autocomplete.
    };
}

#ifdef CSYS_HEADER_ONLY
#include "csys/system.inl"
#endif

#endif

// void (T&, int, float...);

// registaerVariable("name", var, Arg<int>...); //c
// registaerVariable("name", var, setter);      //d
