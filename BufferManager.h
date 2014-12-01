#ifndef BUFFER_H_
#define BUFFER_H_
#include "Structures.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

//#define BLOCKSIZE 4096	//一个4K块
//#define MAXBLOCKNUMBER 1024	//最多拥有1024个buffer块

using namespace std;

//实际存储buffer的数据结构，buffermanager管理的是一个buffer数组
class buffer{
public:
    bool isWritten;
    bool isValid;
    bool Lock(){ return isLock = 1; }
    bool Unlock(){ return isLock = 0; }
    string filename;
    int blockOffset;
    char values[BLOCKSIZE + 1];
    buffer();
    void initialize();
    string getvalues(int startpos, int endpos);
private:
    bool isLock;
};

//使用一个双向链表维护LRU算法，链表中的元素为Buffer块的index。
struct LRU_Node
{
    int blockNum;
    LRU_Node *front;
    LRU_Node *next;
};
class LRU_List
{
public:
    LRU_List();
    ~LRU_List();
    int getLRUblock();
    void useBlock(int blockNum);
    //void print();
private:
    LRU_Node *head;
    LRU_Node *end;
};

class BufferManager
{
public:
    buffer bufferBlock[MAXBLOCKNUMBER];
    BufferManager();
    ~BufferManager();
    //得到对应的buffer块offset
    int getbufferNum(string filename, int blockOffset);
    //读取buffer内容
    void readBlock(string filename, int blockOffset, int bufferNum);
    //申请一个新的buffer，返回值为Buffer的index
    int getEmptyBuffer();
    int getEmptyBufferExcept(string filename);
    //记录Buffer位置和文件中的offset
    insertPos getInsertPosition(Table& tableinfo);

    //添加table或者index到Buffer中去，供TableManager, IndexManager调用
    int AddBuffer(Table& tableinfo);
    int AddBuffer(Index& indexinfo);
    int getIfIsInBuffer(string filename, int blockOffset);
    void setInvalid(string filename);
private:
    void WriteDisk(int bufferNum);
    LRU_List lru;
    //记录buffer是否已满
    bool Full;
};

#endif
