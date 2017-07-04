#ifndef DB_PRINT_H
#define DB_PRINT_H

#include "Operator.h"
#include <iostream>
#include <vector>

class Print : public Operator
{
private:
    Operator* input_;
    std::ostream& output_stream_;

public:
    Print(Operator* input, std::ostream& output)
            : input_(input), output_stream_(output) { };

    void open();

    bool next();

    void close();

    std::vector<Register> getOutput() const;
};


#endif //DB_PRINT_H
