#ifndef DB_HASHINNERJOIN_H
#define DB_HASHINNERJOIN_H

#include <unordered_map>
#include "Operator.h"

class HashInnerJoin : public Operator
{
private:
    Operator* left_input_;
    Operator* right_input_;
    unsigned left_id_;
    unsigned right_id_;
    std::unordered_map<Register, std::vector<Register> > hash_;

public:
    HashInnerJoin(Operator* left_input, Operator* right_input, unsigned left_id, unsigned right_id)
    : left_input_(left_input), right_input_(right_input), left_id_(left_id), right_id_(right_id), hash_() { }

    void open();

    bool next();

    void close();

    std::vector<Register> getOutput();
};

#endif //DB_HASHINNERJOIN_H
