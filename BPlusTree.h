#ifndef _BPLUSTREE_H_
#define _BPLUSTREE_H_
#include "BufferManager.h"
#include "Structures.h"
#include <list>
#include <iostream>
#include <stdlib.h>
using namespace std;

#define POINTERLENGTH 5
extern BufferManager buf;

//叶节点数据结构
class IndexLeaf{
public:
    string key;
    int offsetInFile;
    int offsetInBlock;
    IndexLeaf(string k, int oif, int oib) :key(k), offsetInFile(oif), offsetInBlock(oib){}
};

//中间结点数据结构
class IndexBranch{
public:
    string key;
    int ptrChild;
    IndexBranch() :key(""), ptrChild(0){}
    IndexBranch(string k, int ptrC) :key(k), ptrChild(ptrC){}
};

//b+树基类，用于叶子节点类和中间结点类的继承
class BPlusTree{
public:
    bool isRoot;
    int bufferNum;
    int ptrFather;
    int recordNum;
    int columnLength;
    BPlusTree(){}
    BPlusTree(int bufNum) : bufferNum(bufNum), recordNum(0){}
    int getPtr(int pos);
    int getRecordNum();
};

//中间节点类
class Branch : public BPlusTree
{
public:
    //新添加的Branch
    Branch(int bufNum) : BPlusTree(bufNum){}

    Branch(int bufNum, const Index& indexinfo);
    ~Branch();
    void insert(IndexBranch node);

    IndexBranch pop();
    IndexBranch getfront();

    list<IndexBranch> nodelist;
};

//叶子节点类
class Leaf : public BPlusTree
{
public:
    int nextSibling;
    int lastSibling;
    list<IndexLeaf> nodelist;
    Leaf(int bufNum);
    Leaf(int bufNum, const Index& indexinfo);
    ~Leaf();

    void insert(IndexLeaf node);
    IndexLeaf pop();
    IndexLeaf getfront();
};

#endif
