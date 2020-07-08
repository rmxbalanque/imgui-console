// Copyright (c) 2020-present, Roland Munguia & Tristan Florian Bouchard.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#ifndef CSYS_AUTOCOMPLETE_H
#define CSYS_AUTOCOMPLETE_H

#pragma once

#include "csys/api.h"
#include <vector>
#include <string>
#include <memory>

namespace csys
{
    // TODO: Check how to add support for UTF Encoding.
    // TODO: Todo add max word suggestion depth.
    // TODO: Only use "const char *" or "std::string" in csys. (On stl containers use iterators - SLOW). (Need to add std::string version)

    //!< Auto complete ternary search tree.
    class CSYS_API AutoComplete
    {
    public:

        // Type definitions.
        using r_sVector = std::vector<std::string> &;
        using sVector = std::vector<std::string>;

        //!< Autocomplete node.
        struct ACNode
        {
            explicit ACNode(const char data, bool isWord = false) : m_Data(data), m_IsWord(isWord), m_Less(nullptr), m_Equal(nullptr), m_Greater(nullptr)
            {};

            explicit ACNode(const char &&data, bool isWord = false) : m_Data(data), m_IsWord(isWord), m_Less(nullptr), m_Equal(nullptr), m_Greater(nullptr)
            {};

            ~ACNode()
            {
                delete m_Less;
                delete m_Equal;
                delete m_Greater;
            };

            char m_Data;          //!< Node data.
            bool m_IsWord;        //!< Flag to determine if node is the end of a word.
            ACNode *m_Less;       //!< Left pointer.
            ACNode *m_Equal;      //!< Middle pointer.
            ACNode *m_Greater;    //!< Right pointer.
        };

        /*!
         * \brief Default Constructor
         */
        AutoComplete() = default;

        /*!
         * \brief
         *      Copy constructor
         * \param tree
         *      Tree to be copied
         */
        AutoComplete(const AutoComplete &tree);

        /*!
         * \brief
         *      Move constructor
         * \param rhs
         *      Tree to be copied
         */
        AutoComplete(AutoComplete &&rhs) = default;

        /*!
         * \brief
         *      Assignment operator
         * \param rhs
         *      Source tree
         * \return
         *      Self
         */
        AutoComplete &operator=(const AutoComplete &rhs);

        /*!
         * \brief
         *      Move assignment operator
         * \param rhs
         *      Source tree
         * \return
         *      Self
         */
        AutoComplete& operator=(AutoComplete&& rhs) = default;

        /*!
         *
         * \tparam inputType
         *      String input type
         * \param[in] il
         *      List of string from which TST will be constructed
         */
        template<typename inputType>
        AutoComplete(std::initializer_list<inputType> il)
        {
            for (const auto &item : il)
            {
                Insert(item);
            }
        }

        /*!
         *
         * \tparam T
         *      Container type
         * \param[in] items
         *      Arbitrary container of strings
         */
        template<typename T>
        explicit AutoComplete(const T &items)
        {
            for (const auto &item : items)
            {
                Insert(item);
            }
        }

        /*!
         * /brief
         *      Destructor
         */
        ~AutoComplete();

        /*!
         * \brief
         *      Get tree node count
         * \return
         *      Tree node count
         */
        [[nodiscard]] size_t Size() const;

        /*!
         * \brief
         *      Get tree word count
         * \return
         *      Word count
         */
        [[nodiscard]] size_t Count() const;

        /*!
         * \brief
         *      Search if the given word is in the tree
         * \param[in] word
         *      Word to search
         * \return
         *      Found word
         */
        bool Search(const char *word);

        /*!
         * \brief
         *      Insert word into tree
         * \param[in] word
         *      Word to be inserted
         */
        void Insert(const char *word);

        /*!
         * \brief
         *      Insert word into tree
         * \param[in] word
         *      Word to be inserted
         */
        void Insert(const std::string &word);

        /*!
         * \brief
         *      Insert word into tree
         * \tparam strType
         *      String type to be inserted
         * \param[in] word
         *      Word to be inserted
         */
        template<typename strType>
        void Insert(const strType &word)
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
                }
                else if (*word == (*ptr)->m_Data)
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
                }
                else
                {
                    ptr = &(*ptr)->m_Greater;
                }
            }
        }

        /*!
         * \brief
         *      Removes a word from the tree if found
         * \param[in] word
         *      String to be removed
         */
        void Remove(const std::string &word);

        /*!
         * \brief
         *      Retrieve suggestions that match the given prefix
         * \tparam strType
         *      Prefix string type
         * \param[in] prefix
         *      Prefix to use for suggestion lookup
         * \param[out] ac_options
         *      Vector of found suggestions
         */
        template<typename strType>
        void Suggestions(const strType &prefix, r_sVector ac_options)
        {
            ACNode *ptr = m_Root;
            auto temp = prefix;

            // Traverse tree and check if prefix exists.
            while (ptr)
            {
                if (*prefix < ptr->m_Data)
                {
                    ptr = ptr->m_Less;
                }
                else if (*prefix == ptr->m_Data)
                {
                    // Prefix exists in tree.
                    if (*(prefix + 1) == '\0')
                        break;

                    ptr = ptr->m_Equal;
                    ++prefix;
                }
                else
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


        /*!
         * \brief
         *      Retrieve suggestions that match the given prefix
         * \param[in] prefix
         *      Prefix to use for suggestion lookup
         * \param[out] ac_options
         *      Vector of found suggestions
         */
        void Suggestions(const char *prefix, r_sVector ac_options);

        /*!
         * \brief
         *      Store suggestions that match prefix in ac_options and return partially completed prefix if possible.
         * \param[in] prefix
         *      Prefix to use for suggestion lookup
         * \param[out] ac_options
         *      Vector of found suggestions
         * \return
         *      Partially completed prefix
         */
        std::string Suggestions(const std::string &prefix, r_sVector ac_options);

        /*!
         * \brief
         *      Retrieve suggestions that match the given prefix
         * \param[in/out] prefix
         *      Prefix to use for suggestion lookup, will be partially completed if flag partial_complete is on
         * \param[out] ac_options
         *      Vector of found suggestions
         * \param[in] partial_complete
         *      Flag to determine if prefix string will be partially completed
         */
        void Suggestions(std::string &prefix, r_sVector ac_options, bool partial_complete);

        /*!
         * \brief
         *      Retrieve suggestions that match the given prefix
         * \tparam strType
         *      Prefix string type
         * \param[in] prefix
         *      Prefix to use for suggestion lookup
         * \return
         *      Vector of found suggestions
         */
        template<typename strType>
        std::unique_ptr<sVector> Suggestions(const strType &prefix)
        {
            auto temp = std::make_unique<sVector>();
            Suggestions(prefix, *temp);
            return temp;
        }

        /*!
         * \brief
         *      Retrieve suggestions that match the given prefix
         * \param[in] prefix
         *      Prefix to use for suggestion lookup
         * \return
         *      Vector of found suggestions
         */
        std::unique_ptr<sVector> Suggestions(const char *prefix);

    protected:

        /*!
         * \param[in] root
         *      Permutation root
         * \param[out] ac_options
         *      Vector of found suggestions
         * \param[in] buffer
         *      Prefix buffer
         */
        void SuggestionsAux(ACNode *root, r_sVector ac_options, std::string buffer);

        /*!
         * \brief
         *      Remove word auxiliary function
         * \param[in] root
         *      Current node to process
         * \param[in] word
         *      String to look for and remove
         * \return
         *      If node is word
         */
        bool RemoveAux(ACNode *root, const char *word);

        /*!
         * \brief
         *      Clone acNode and all its children
         * \param src
         *      Node to copy from
         * \param dest
         *      Where new node will be stored
         */
        void DeepClone(ACNode *src, ACNode *&dest);

        ACNode *m_Root = nullptr;    //!< Ternary Search Tree Root node
        size_t m_Size = 0;           //!< Node count
        size_t m_Count = 0;          //!< Word count
    };
}

#ifdef CSYS_HEADER_ONLY
#include "csys/autocomplete.inl"
#endif

#endif //CSYS_AUTOCOMPLETE_H
