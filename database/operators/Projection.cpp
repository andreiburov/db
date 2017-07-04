#include <stdexcept>
#include "Projection.h"

void Projection::open() {
    input_->open();
    output_.reserve(indices_.size());
}

bool Projection::next() {
    output_.clear();
    while (input_->next()) {
        auto input = input_->getOutput();

        for (auto i : indices_) {
            if (i >= input.size()) {
                throw std::out_of_range("column id is too large");
            } else {
                output_.push_back(input[i]);
                return true;
            }
        }
    }

    return false;
}

void Projection::close() {
    input_->close();
}

std::vector<Register> Projection::getOutput() const {
    return output_;
}
