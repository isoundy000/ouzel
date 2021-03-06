// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <map>
#include <string>
#include <vector>

namespace ouzel
{
    namespace xml
    {
        class Node
        {
        public:
            enum class Type
            {
                NONE,
                COMMENT,
                CDATA,
                DOCTYPE_DEFINITION,
                PROCESSING_INSTRUCTION,
                TAG,
                TEXT
            };

            Node();

            const std::string& getValue() const { return value; }

        protected:
            bool parseNode(const std::vector<uint32_t>& utf32,
                            std::vector<uint32_t>::iterator& iterator);
            bool encodeNode(std::vector<uint8_t>& data) const;

            Type type = Type::NONE;

            std::string value;
            std::map<std::string, std::string> attributes;
            std::vector<Node> children;
        };

        class Data: public Node
        {
        public:
            Data();
            Data(const std::string& filename);
            Data(const std::vector<uint8_t>& data);

            bool init(const std::string& filename);
            bool init(const std::vector<uint8_t>& data);
            
            bool save(const std::string& filename) const;
            bool encode(std::vector<uint8_t>& data) const;

            bool hasBOM() const { return bom; }
            void setBOM(bool newBOM) { bom = newBOM; }

        protected:

            bool bom = false;
        };
    } // namespace xml
}
