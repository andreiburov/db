#ifndef DB_SCHEMA_H
#define DB_SCHEMA_H

#include "Type.h"
#include <string>
#include <vector>

struct Schema {
    struct Relation {
        struct Attribute {
            std::string name;
            Type type;
            unsigned length;
            bool notNull;

            Attribute(std::string name, Type type, unsigned length, bool notNull)
                    : name(name), type(type), length(length), notNull(notNull) { }

            Attribute(std::string name, Type type, unsigned length)
                    : Attribute(name, type, length, true) { }

            Attribute()
                    : Attribute(std::string(), Type::Misc, 0, true) { }
        };

        std::string name;
        std::vector<Attribute> attributes;
        std::vector<unsigned> primaryKey;

        Relation(const std::string &name) : name(name) { }

        inline void addAttribute(Attribute attribute) {
            attributes.push_back(attribute);
        }
    };

    std::vector<Schema::Relation> relations;
};

#endif //DB_SCHEMA_H
