#ifndef _RECORD_H_
#define _RECORD_H_
#include "BufferManager.h"
#include "Structures.h"
#include <stdio.h>
#include <vector>
extern BufferManager buf;
class RecordManager
{
public:
    Data select(Table& tableinfo);
    Data select(Table tableinfo, vector<Condition> conditions);
    void insertValue(Table& tableinfo, Row& splitedRow);
    int deleteValue(Table tableinfo);
    int deleteValue(Table tableinfo, vector<Condition> conditions);
    //void dropTable(Table tableinfo);
    void dropTable(string tablename);
    void createTable(Table tableinfo);
private:
    //检查输入的查询语句条件，用于select与deleteValue
    bool Check(Table tableinfor, Row row, vector<Condition> conditions);
    Row splitRow(Table tableinfor, string row);
    string connectRow(Table tableinfor, Row splitedRow);
};
#endif
