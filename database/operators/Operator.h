#ifndef DB_OPERATOR_H
#define DB_OPERATOR_H

#include "../relations/Type.h"
#include "Register.h"
#include <vector>

class Operator {
protected:
    std::vector<Record> records_;
    std::vector<Register> output_;

public:
    virtual void open() = 0;
    virtual bool next() = 0;
    virtual std::vector<Register> getOutput() const = 0;
    virtual void close() = 0;
};

#endif //DB_OPERATOR_H
