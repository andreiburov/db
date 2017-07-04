#include <stdexcept>
#include "Selection.h"

void Selection::open() {
    input_->open();
}

bool Selection::next() {
    while (!input_->next()) {
        auto input = input_->getOutput();

        if (id_ < input.size()) {
            if (input[id_] == constant_) {
                return true;
            }
        } else {
            throw std::out_of_range("column id is too large");
        }

        continue;
    }

    return false;
}

std::vector<Register> Selection::getOutput() {
    return output_;
}

void Selection::close() {
    input_->close();
}
