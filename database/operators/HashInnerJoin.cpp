#include "HashInnerJoin.h"

void HashInnerJoin::open() {
    left_input_->open();
    while (left_input_->next()) {
        auto input_left = left_input_->getOutput();
        hash_[input_left[left_id_]] = input_left;
    }

    right_input_->open();
}

bool HashInnerJoin::next() {
    output_.clear();

    while (!right_input_->next()) {
        auto input_right = left_input_->getOutput();
        auto it = hash_.find(input_right[right_id_]);
        if (it == hash_.end()) continue;

        auto input_left = it->second;
        output_.insert(output_.end(), input_left.begin(), input_left.end());
        output_.insert(output_.end(), input_right.begin(), input_right.end());
    }

    return false;
}

void HashInnerJoin::close() {
    left_input_->close();
    right_input_->close();
}

std::vector<Register> HashInnerJoin::getOutput() {
    return output_;
}
