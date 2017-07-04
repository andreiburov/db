#ifndef DB_TABLESCAN_H
#define DB_TABLESCAN_H

#include "../relations/Schema.h"
#include "../slotted/SPSegment.h"
#include "Operator.h"

/*
 *  void open(): Open the operator
 *  bool next(): Produce the next tuple
 *  vector<Register*> getOutput(): Get all produced values
 *  void close(): Close the operator
 *
 */

class TableScan : public Operator
{
private:
    Schema::Relation& relation_;
    SPSegment& segment_;
    uint64_t current_page_;
    uint64_t registered_;

public:
    TableScan(Schema::Relation& relation, SPSegment& segment)
            : relation_(relation), segment_(segment),
              current_page_(GetFirstPage(segment.getSegmentId())),
              registered_(0UL)
    { }

    void open();

    bool next();

    void close();

    std::vector<Register> getOutput() const;

private:
    void produceRegisters(const Record& record);
};

#endif //DB_TABLESCAN_H