#include "Print.h"

void Print::open() {
    input_->open();
}

void Print::close() {
    input_->close();
}

bool Print::next() {
    output_.clear();

    if(!input_->next()) { return false; }
    output_ = input_->getOutput();
    for (auto reg : output_)
    {
        switch (reg.getType()) {
            case Type::Integer:
                output_stream_ << reg.getInteger();
                break;
            default:
                output_stream_ << reg.getString();
                break;
        }
    }

    output_stream_ << std::endl;

    return true;
}

std::vector<Register> Print::getOutput() const {
    return output_;
}
