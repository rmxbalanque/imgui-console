// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef CSYS_HEADER_ONLY

#include "csys/autocomplete.h"

#endif

namespace csys
{
    ///////////////////////////////////////////////////////////////////////////
    // Constructor/Destructors ////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    CSYS_INLINE AutoComplete::~AutoComplete()
    {
        delete m_Root;
    }

    CSYS_INLINE AutoComplete::AutoComplete(const AutoComplete &tree) : m_Size(tree.m_Size), m_Count(tree.m_Count)
    {
        DeepClone(tree.m_Root, m_Root);
    }

    CSYS_INLINE AutoComplete &AutoComplete::operator=(const AutoComplete &rhs)
    {
        // Prevent self assignment.
        if (&rhs == this) return *this;

        // Clean.
        delete m_Root;

        // Copy from source tree.
        DeepClone(rhs.m_Root, m_Root);
        m_Size = rhs.m_Size;
        m_Count = rhs.m_Count;

        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Public methods /////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    CSYS_INLINE size_t AutoComplete::Size() const
    {
        return m_Size;
    }

    CSYS_INLINE size_t AutoComplete::Count() const
    {
        return m_Count;
    }

    CSYS_INLINE bool AutoComplete::Search(const char *word)
    {
        ACNode *ptr = m_Root;

        // Traverse tree in look for the given string.
        while (ptr)
        {
            if (*word < ptr->m_Data)
            {
                ptr = ptr->m_Less;
            } else if (*word == ptr->m_Data)
            {
                // Word was found.
                if (*(word + 1) == '\0' && ptr->m_IsWord)
                    return true;

                ptr = ptr->m_Equal;
                ++word;
            } else
            {
                ptr = ptr->m_Greater;
            }
        }

        return false;
    }

    CSYS_INLINE void AutoComplete::Insert(const char *word)
    {
        ACNode **ptr = &m_Root;
        ++m_Count;

        while (*word != '\0')
        {
            // Insert char into tree.
            if (*ptr == nullptr)
            {
                *ptr = new ACNode(*word);
                ++m_Size;
            }

            // Traverse tree.
            if (*word < (*ptr)->m_Data)
            {
                ptr = &(*ptr)->m_Less;
            } else if (*word == (*ptr)->m_Data)
            {
                // String is already in tree, therefore only mark as word.
                if (*(word + 1) == '\0')
                {
                    if ((*ptr)->m_IsWord)
                        --m_Count;

                    (*ptr)->m_IsWord = true;
                }

                // Advance.
                ptr = &(*ptr)->m_Equal;
                ++word;
            } else
            {
                ptr = &(*ptr)->m_Greater;
            }
        }
    }

    CSYS_INLINE void AutoComplete::Insert(const std::string &word)
    {
        Insert(word.c_str());
    }

    CSYS_INLINE void AutoComplete::Remove(const std::string &word)
    {
        RemoveAux(m_Root, word.c_str());
    }

    CSYS_INLINE void AutoComplete::Suggestions(const char *prefix, std::vector<std::string> &ac_options)
    {
        ACNode *ptr = m_Root;
        auto temp = prefix;

        // Traverse tree and check if prefix exists.
        while (ptr)
        {
            if (*prefix < ptr->m_Data)
            {
                ptr = ptr->m_Less;
            } else if (*prefix == ptr->m_Data)
            {
                // Prefix exists in tree.
                if (*(prefix + 1) == '\0')
                    break;

                ptr = ptr->m_Equal;
                ++prefix;
            } else
            {
                ptr = ptr->m_Greater;
            }
        }

        // Already a word. (No need to auto complete).
        if (ptr && ptr->m_IsWord) return;

        // Prefix is not in tree.
        if (!ptr) return;

        // Retrieve auto complete options.
        SuggestionsAux(ptr->m_Equal, ac_options, temp);
    }

    CSYS_INLINE std::string AutoComplete::Suggestions(const std::string &prefix, r_sVector &ac_options)
    {
        std::string temp = prefix;
        Suggestions(temp, ac_options, true);
        return temp;
    }

    CSYS_INLINE void AutoComplete::Suggestions(std::string &prefix, r_sVector ac_options, bool partial_complete)
    {
        ACNode *ptr = m_Root;
        const char *temp = prefix.data();
        size_t prefix_end = prefix.size();

        // Traverse tree and check if prefix exists.
        while (ptr)
        {
            if (*temp < ptr->m_Data)
            {
                ptr = ptr->m_Less;
            } else if (*temp == ptr->m_Data)
            {
                // Prefix exists in tree.
                if (*(temp + 1) == '\0')
                {
                    if (partial_complete)
                    {
                        ACNode *pc_ptr = ptr->m_Equal;

                        // Get partially completed string.
                        while (pc_ptr)
                        {
                            if (pc_ptr->m_Equal && !pc_ptr->m_Less && !pc_ptr->m_Greater)
                                prefix.push_back(pc_ptr->m_Data);
                            else
                                break;

                            pc_ptr = pc_ptr->m_Equal;
                        }
                    }

                    break;
                }

                ptr = ptr->m_Equal;
                ++temp;
            } else
            {
                ptr = ptr->m_Greater;
            }
        }

        // Already a word. (No need to auto complete).
        if (ptr && ptr->m_IsWord) return;

        // Prefix is not in tree.
        if (!ptr) return;

        // Retrieve auto complete options.
        SuggestionsAux(ptr->m_Equal, ac_options, prefix.substr(0, prefix_end));
    }

    CSYS_INLINE std::unique_ptr<AutoComplete::sVector> AutoComplete::Suggestions(const char *prefix)
    {
        auto temp = std::make_unique<sVector>();
        Suggestions(prefix, *temp);
        return temp;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Private methods ////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    CSYS_INLINE void AutoComplete::SuggestionsAux(AutoComplete::ACNode *root, r_sVector ac_options, std::string buffer)
    {
        if (!root) return;

        // Continue looking in left branch.
        if (root->m_Less) SuggestionsAux(root->m_Less, ac_options, buffer);

        // Word was found, push into autocomplete options.
        if (root->m_IsWord)
        {
            ac_options.push_back(buffer.append(1, root->m_Data));
            buffer.pop_back();
        }

        // Continue in middle branch, and push character.
        if (root->m_Equal)
        {
            SuggestionsAux(root->m_Equal, ac_options, buffer.append(1, root->m_Data));
            buffer.pop_back();
        }

        // Continue looking in right branch.
        if (root->m_Greater)
        {
            SuggestionsAux(root->m_Greater, ac_options, buffer);
        }
    }

    CSYS_INLINE bool AutoComplete::RemoveAux(AutoComplete::ACNode *root, const char *word)
    {
        if (!root) return false;

        // String is in TST.
        if (*(word + 1) == '\0' && root->m_Data == *word)
        {
            // Un-mark word node.
            if (root->m_IsWord)
            {
                root->m_IsWord = false;
                return (!root->m_Equal && !root->m_Less && !root->m_Greater);
            }
                // String is a prefix.
            else
                return false;
        } else
        {
            // String is a prefix.
            if (*word < root->m_Data)
                RemoveAux(root->m_Less, word);
            else if (*word > root->m_Data)
                RemoveAux(root->m_Greater, word);

                // String is in TST.
            else if (*word == root->m_Data)
            {
                // Char is unique.
                if (RemoveAux(root->m_Equal, word + 1))
                {
                    delete root->m_Equal;
                    root->m_Equal = nullptr;
                    return !root->m_IsWord && (!root->m_Equal && !root->m_Less && !root->m_Greater);
                }
            }
        }

        return false;
    }

    CSYS_INLINE void AutoComplete::DeepClone(AutoComplete::ACNode *src, AutoComplete::ACNode *&dest)
    {
        if (!src) return;

        dest = new ACNode(*src);
        DeepClone(src->m_Less, dest->m_Less);
        DeepClone(src->m_Equal, dest->m_Equal);
        DeepClone(src->m_Greater, dest->m_Greater);
    }
}
