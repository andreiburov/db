#ifndef DB_REGISTER_H
#define DB_REGISTER_H

#include "../relations/Type.h"
#include <string>

class Register
{
private:
    Type type_;
    int int_;
    std::string string_;

public:
    Register() : type_(Type::Misc) {}
    Register(int integer) : type_(Type::Integer), int_(integer) {}
    Register(const std::string& string) : type_(Type::String), string_(string) {}

    inline void setInteger(int integer) {
        type_ = Type::Integer;
        int_ = integer;
    }

    inline void setString(const std::string& string) {
        type_ = Type::String;
        string_ = string;
    }

    inline int getInteger() const {
        return int_;
    }

    inline std::string getString() const {
        return string_;
    }

    inline Type getType() const {
        return type_;
    }

    bool operator==(const Register& other) const {
        if (type_ != other.type_) {
            return false;
        }

        switch (type_) {
            case Type::Integer:
                return getInteger() == other.getInteger();
            case Type::String:
                return getString().compare(other.getString()) == 0;
            default:
                return false;
        }
    }

    inline size_t hash() const {
        switch(type_) {
            case Type::Integer:
                return std::hash<int>()(int_);
            case Type::String:
                return std::hash<std::string>()(string_);
            default:
                return 0UL;
        }
    }
};

namespace std {
    template<>
    struct hash<Register> {
        size_t operator()(const Register& reg) const
        {
            return reg.hash();
        }
    };
}

#endif //DB_REGISTER_H
