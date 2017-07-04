#include "Print.h"

void Print::open() {
    input_->open();
}

void Print::close() {
    input_->close();
}

bool Print::next() {
    if(!input_->next()) { return false; }
    output_ = input_->getOutput();
    for (auto reg : output_)
    {
        switch (reg.getType()) {
            Integer:
                output_stream_ << reg.getInteger();
                break;
            default:
                output_stream_ << reg.getString();
                break;
        }
    }

    return true;
}

std::vector<Register> Print::getOutput() const {
    return output_;
}
