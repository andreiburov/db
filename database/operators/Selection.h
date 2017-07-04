#ifndef DB_SELECTION_H
#define DB_SELECTION_H

#include "Operator.h"

class Selection : public Operator
{
private:
    Operator* input_;
    const Register constant_;
    unsigned id_;


public:
    Selection(Operator* input, unsigned id, const Register& constant)
            : input_(input), constant_(constant), id_(id) { }

    void open();

    bool next();

    std::vector<Register> getOutput() const;

    void close();
};

#endif //DB_SELECTION_H
