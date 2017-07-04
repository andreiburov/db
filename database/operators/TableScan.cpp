#include "TableScan.h"
#include "../dbms.h"

void TableScan::open() {
    this->output_.reserve(relation_.attributes.size()*segment_.getNumberOfRecords());
}

void TableScan::close() {
    this->records_.clear();
    this->output_.clear();
};

bool TableScan::next() {

    if (GetPageOffset(current_page_) > segment_.getMaxPageId()) { return false; }

    if (records_.size() <= registered_) {
        if (GetPageOffset(current_page_) >= segment_.getMaxPageId()) { return false; }
        records_ = segment_.lookupRecordsInPage(current_page_);
        registered_ = 0;
        produceRegisters(records_[registered_++]);
        current_page_++;
    } else {
        produceRegisters(records_[registered_++]);
    }

    return true;
}

std::vector<Register> TableScan::getOutput() const {
    return this->output_;
}

void TableScan::produceRegisters(const Record& record) {
    output_.clear();
    char* record_data = const_cast<char*>(record.getData());
    for (auto& attribute : relation_.attributes) {
        Register reg;
        switch (attribute.type) {
            case Type::Integer:
                reg.setInteger(*reinterpret_cast<DB_INT*>(record_data));
                break;
            default:
                reg.setString(DB_STR(record_data, attribute.length));
                break;
        }
        output_.push_back(reg);
        record_data += attribute.length;
        assert(record_data <= record.getData() + record.getLen());
    }
};