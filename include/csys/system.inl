// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef CSYS_HEADER_ONLY

#include "csys/system.h"

#endif

namespace csys
{
    ///////////////////////////////////////////////////////////////////////////
    // Public methods /////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    // Commands-Error-Warning strings.
    static const std::string_view s_Set = "set";
    static const std::string_view s_Get = "get";
    static const std::string_view s_Help = "help";
    static const std::string_view s_ErrorNoVar = "No variable provided";
    static const std::string_view s_ErrorSetGetNotFound = "Command doesn't exist and/or variable is not registered";

    CSYS_INLINE System::System()
    {
        // Register help command.
        RegisterCommand(s_Help.data(), "Display commands information", [this]()
        {
            // Custom command information display
            Log() << "help [command_name:String] (Optional)\n\t\t- Display command(s) information\n" << csys::endl;
            Log() << "set [variable_name:String] [data]\n\t\t- Assign data to given variable\n" << csys::endl;
            Log() << "get [variable_name:String]\n\t\t- Display data of given variable\n" << csys::endl;

            for (const auto &tuple : Commands())
            {
                // Filter set and get.
                if (tuple.first.size() >= 5 && (tuple.first[3] == ' ' || tuple.first[4] == ' '))
                    continue;

                // Skip help command.
                if (tuple.first.size() == 4 && (tuple.first == "help"))
                    continue;

                // Print the rest of commands
                Log() << tuple.second->Help();
            }
        });

        // Register pre-defined commands.
        m_CommandSuggestionTree.Insert(s_Set.data());
        m_CommandSuggestionTree.Insert(s_Get.data());
    }

    CSYS_INLINE System::System(const System &rhs) : m_CommandSuggestionTree(rhs.m_CommandSuggestionTree),
                                                    m_VariableSuggestionTree(rhs.m_VariableSuggestionTree),
                                                    m_CommandHistory(rhs.m_CommandHistory),
                                                    m_ItemLog(rhs.m_ItemLog),
                                                    m_RegisterCommandSuggestion(rhs.m_RegisterCommandSuggestion)
    {
        // Copy commands.
        for (const auto &pair : rhs.m_Commands)
        {
            m_Commands[pair.first] = std::unique_ptr<CommandBase>(pair.second->Clone());
        }

        // Copy scripts.
        for (const auto &pair: rhs.m_Scripts)
        {
            m_Scripts[pair.first] = std::make_unique<Script>(*pair.second);
        }
    }

    CSYS_INLINE System &System::operator=(const System &rhs)
    {
        if (this == &rhs)
            return *this;

        // Copy commands.
        for (const auto &pair : rhs.m_Commands)
        {
            m_Commands[pair.first] = std::unique_ptr<CommandBase>(pair.second->Clone());
        }

        // Other data.
        m_CommandSuggestionTree = rhs.m_CommandSuggestionTree;
        m_VariableSuggestionTree = rhs.m_VariableSuggestionTree;
        m_CommandHistory = rhs.m_CommandHistory;
        m_ItemLog = rhs.m_ItemLog;

        // Copy scripts.
        for (const auto &pair: rhs.m_Scripts)
        {
            m_Scripts[pair.first] = std::make_unique<Script>(*pair.second);
        }

        // Rest of data.
        m_RegisterCommandSuggestion = rhs.m_RegisterCommandSuggestion;

        return *this;
    }

    CSYS_INLINE void System::RunCommand(const std::string &line)
    {
        // Error checking.
        if (line.empty())
            return;

        // Log command.
        Log(csys::ItemType::COMMAND) << line << csys::endl;

        // Parse command line.
        ParseCommandLine(line);
    }

    CSYS_INLINE void System::RunScript(const std::string &script_name)
    {
        // Attempt to find script.
        auto script_pair = m_Scripts.find(script_name);

        // Exit if not found.
        if (script_pair == m_Scripts.end())
        {
            m_ItemLog.log(ERROR) << "Script \"" << script_name << "\" not found" << csys::endl;
            return;
        }

        // About to run script.
        m_ItemLog.log(INFO) << "Running \"" << script_name << "\"" << csys::endl;

        // Load if script is empty.
        if (script_pair->second->Data().empty())
        {
            try
            {
                script_pair->second->Load();
            }
            catch (csys::Exception &e)
            {
                Log(ERROR) << e.what() << csys::endl;
            }
        }

        // Run script.
        for (const auto &cmd : script_pair->second->Data())
        {
            // Parse command.
            RunCommand(cmd);
        }
    }

    CSYS_INLINE void System::RegisterScript(const std::string &name, const std::string &path)
    {
        // Attempt to find scripts.
        auto script = m_Scripts.find(name);

        // Don't register if script already exists.
        if (script == m_Scripts.end())
        {
            m_Scripts[name] = std::make_unique<Script>(path, true);
            m_VariableSuggestionTree.Insert(name);
        } else
            throw csys::Exception("ERROR: Script \'" + name + "\' already registered");
    }

    CSYS_INLINE void System::UnregisterCommand(const std::string &cmd_name)
    {
        // Exit if non existent.
        if (cmd_name.empty()) return;

        // Get command.
        auto command_it = m_Commands.find(cmd_name);
        auto help_command_it = m_Commands.find("help " + cmd_name);

        // Erase if found.
        if (command_it != m_Commands.end() && help_command_it != m_Commands.end())
        {
            m_CommandSuggestionTree.Remove(cmd_name);
            m_VariableSuggestionTree.Remove(cmd_name);

            m_Commands.erase(command_it);
            m_Commands.erase(help_command_it);
        }
    }

    CSYS_INLINE void System::UnregisterVariable(const std::string &var_name)
    {
        // Exit if non existent.
        if (var_name.empty()) return;

        // Get command.
        auto s_it = m_Commands.find("set " + var_name);
        auto g_it = m_Commands.find("get " + var_name);

        // Erase if found.
        if (s_it != m_Commands.end() && g_it != m_Commands.end())
        {
            m_VariableSuggestionTree.Remove(var_name);
            m_Commands.erase(s_it);
            m_Commands.erase(g_it);
        }
    }

    CSYS_INLINE void System::UnregisterScript(const std::string &script_name)
    {
        // Exit if non existent.
        if (script_name.empty()) return;

        // Get command.
        auto it = m_Scripts.find(script_name);

        // Erase if found.
        if (it != m_Scripts.end())
        {
            m_VariableSuggestionTree.Remove(script_name);
            m_Scripts.erase(it);
        }
    }

    // Getters ////////////////////////////////////////////////////////////////

    CSYS_INLINE AutoComplete &System::CmdAutocomplete() { return m_CommandSuggestionTree; }

    CSYS_INLINE AutoComplete &System::VarAutocomplete() { return m_VariableSuggestionTree; }

    CSYS_INLINE CommandHistory &System::History() { return m_CommandHistory; }

    CSYS_INLINE std::vector<Item> &System::Items() { return m_ItemLog.Items(); }

    CSYS_INLINE ItemLog &System::Log(ItemType type) { return m_ItemLog.log(type); }

    CSYS_INLINE std::unordered_map<std::string, std::unique_ptr<CommandBase>> &System::Commands() { return m_Commands; }

    CSYS_INLINE std::unordered_map<std::string, std::unique_ptr<Script>> &System::Scripts() { return m_Scripts; }

    ///////////////////////////////////////////////////////////////////////////
    // Private methods ////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    CSYS_INLINE void System::ParseCommandLine(const String &line)
    {
        // Get first non-whitespace char.
        size_t line_index = 0;
        std::pair<size_t, size_t> range = line.NextPoi(line_index);

        // Just whitespace was passed in. Don't log as command.
        if (range.first == line.End())
            return;

        // Push to history.
        m_CommandHistory.PushBack(line.m_String);

        // Get name of command.
        std::string command_name = line.m_String.substr(range.first, range.second - range.first);

        // Set or get
        bool is_cmd_set = command_name == s_Set;
        bool is_cmd_get = command_name == s_Get;
        bool is_cmd_help = !(is_cmd_set || is_cmd_get) ? command_name == s_Help : false;

        // Edge case for if user is just runs "help" command
        if (is_cmd_help)
        {
            range = line.NextPoi(line_index);
            if (range.first != line.End())
                command_name += " " + line.m_String.substr(range.first, range.second - range.first);
        }

            // Its a set or get command
        else if (is_cmd_set || is_cmd_get)
        {
            // Try to get variable name
            if ((range = line.NextPoi(line_index)).first == line.End())
            {
                Log(ERROR) << s_ErrorNoVar << endl;
                return;
            } else
                // Append variable name.
                command_name += " " + line.m_String.substr(range.first, range.second - range.first);
        }

        // Get runnable command
        auto command = m_Commands.find(command_name);
        if (command == m_Commands.end())
            Log(ERROR) << s_ErrorSetGetNotFound << endl;
            // Run the command
        else
        {
            // Get the arguments.
            String arguments = line.m_String.substr(range.second, line.m_String.size() - range.first);

            // Execute command.
            auto cmd_out = (*command->second)(arguments);

            // Log output.
            if (cmd_out.m_Type != NONE)
                m_ItemLog.Items().emplace_back(cmd_out);
        }
    }
}