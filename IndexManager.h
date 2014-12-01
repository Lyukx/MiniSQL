#ifndef _INDEX_H_
#define _INDEX_H_
#include "BufferManager.h"
#include "Structures.h"
#include "BPlusTree.h"

#define POINTERLENGTH 5
extern BufferManager buf;

//真正提供外部接口的类
class IndexManager{
public:
    void createIndex(const Table& tableinfo, Index& indexinfo);
    void insertKey(string key, Index& indexinfo, Table& tableinfo);
    void deleteKey(string key, Index& indexinfo);
    Data selectEqual(const Table& tableinfo, const Index& indexinfo, string key, int blockOffset = 0);
    Data selectBetween(const Table& tableinfo, const Index& indexinfo, string keyFrom, string keyTo, int blockOffset = 0);
    void dropIndex(Index& indexinfo);
    void deleteValue(){}
private:
    IndexBranch insertValue(Index& indexinfo, IndexLeaf node, int blockOffset = 0);
    Row splitRow(Table tableinfo, string row);
    string getColumnValue(const Table& tableinfo, const Index& indexinfo, string row);
};

#endif
