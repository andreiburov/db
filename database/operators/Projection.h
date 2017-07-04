#ifndef DB_PROJECTION_H
#define DB_PROJECTION_H

#include "Operator.h"

class Projection : public Operator
{
private:
    Operator* input_;
    std::vector<unsigned> indices_; // which columns to eliminate

public:
    Projection(Operator* input, std::vector<unsigned> indices)
        : input_(input), indices_(indices) { }

    void open();

    bool next();

    void close();

    std::vector<Register> getOutput() const;
};

#endif //DB_PROJECTION_H
