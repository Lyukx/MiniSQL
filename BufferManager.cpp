#include "BufferManager.h"

LRU_List::LRU_List()
{
    head = new LRU_Node;
    end = new LRU_Node;
    head->blockNum = -1;
    end->blockNum = -1;
    head->front = NULL;
    head->next = end;
    end->front = head;
    end->next = NULL;
}

LRU_List::~LRU_List()
{
    LRU_Node *temp = head;
    LRU_Node *temp_;
    while (temp->next != NULL){
        temp_ = temp;
        temp = temp->next;
        delete temp_;
    }
    delete temp;
}

int LRU_List::getLRUblock(){
    int LRUblockNum;
    LRUblockNum = end->blockNum;
    if (end->front != head){
        end = end->front;
        delete end->next;
    }
    else
        end->blockNum = -1;
    return LRUblockNum;
}

void LRU_List::useBlock(int blockNum){
    LRU_Node* temp = head;
    while (temp->next != NULL){
        if (temp->blockNum == blockNum){
            if (temp == head){
                //不用修改
                return;
            }
            else if (temp == end){
                end = temp->front;
                temp->front = NULL;
                temp->next = head;
                head->front = temp;
                head = temp;
            }
            else{
                temp->front->next = temp->next;
                temp->next->front = temp->front;
                temp->front = NULL;
                temp->next = head;
                head->front = temp;
                head = temp;
            }
            return;
        }
        temp = temp->next;
    }
    //使用的是之前没有使用过的的Buffer块
    if (head->blockNum == -1)
        head->blockNum = blockNum;
    else if (end->blockNum == -1){
        end->blockNum = head->blockNum;
        head->blockNum = blockNum;
    }
    else{
        temp = new LRU_Node;
        temp->blockNum = blockNum;
        temp->front = NULL;
        temp->next = head;
        head->front = temp;
        head = temp;
    }
    return;
}

buffer::buffer()
{
    initialize();
}

void buffer::initialize()
{
    isWritten = 0;
    isValid = 0;
    filename = "NULL";
    blockOffset = 0;
    for (int i = 0; i < BLOCKSIZE; i++)
        values[i] = EMPTY;
    values[BLOCKSIZE] = '\0';
}
string buffer::getvalues(int startpos, int endpos)
{
    string tmpt = "";
    if (startpos >= 0 && startpos <= endpos && endpos <= BLOCKSIZE){
        for (int i = startpos; i < endpos; i++)
            tmpt += values[i];
    }
    return tmpt;
}

BufferManager::BufferManager()
{
    Full = false;
    for (int i = 0; i < MAXBLOCKNUMBER; i++)
        bufferBlock[i].initialize();
}

BufferManager::~BufferManager()
{
    for (int i = 0; i < MAXBLOCKNUMBER; i++)
        WriteDisk(i);
}

void BufferManager::WriteDisk(int bufferNum)
{
    if (!bufferBlock[bufferNum].isWritten)
        return;
    string filename = bufferBlock[bufferNum].filename;
    fstream fout(filename.c_str(), ios::in | ios::out);
    fout.seekp(BLOCKSIZE*bufferBlock[bufferNum].blockOffset, fout.beg);
    fout.write(bufferBlock[bufferNum].values, BLOCKSIZE);
    bufferBlock[bufferNum].initialize();
    fout.close();
}

int BufferManager::getbufferNum(string filename, int blockOffset)
{
    int bufferNum = getIfIsInBuffer(filename, blockOffset);
    if (bufferNum == -1){
        bufferNum = getEmptyBufferExcept(filename);
        readBlock(filename, blockOffset, bufferNum);
    }
    //lru.useBlock(bufferNum);
    return bufferNum;
}

void BufferManager::readBlock(string filename, int blockOffset, int bufferNum)
{
    bufferBlock[bufferNum].isValid = 1;
    bufferBlock[bufferNum].isWritten = 0;
    bufferBlock[bufferNum].filename = filename;
    bufferBlock[bufferNum].blockOffset = blockOffset;
    fstream  fin(filename.c_str(), ios::in);
    fin.seekp(BLOCKSIZE*blockOffset, fin.beg);
    fin.read(bufferBlock[bufferNum].values, BLOCKSIZE);
    fin.close();
}

//申请一个新的Buffer块，
int BufferManager::getEmptyBuffer()
{
    int bufferNum = 0;
    if (!Full){
        for (int i = 0; i < MAXBLOCKNUMBER; i++)
        {
            if (!bufferBlock[i].isValid){
                bufferBlock[i].initialize();
                bufferBlock[i].isValid = 1;
                return i;
            }
        }
        Full = true;
    }
    //If is full
    bufferNum = lru.getLRUblock();
    WriteDisk(bufferNum);
    bufferBlock[bufferNum].isValid = 1;
    return bufferNum;
}

int BufferManager::getEmptyBufferExcept(string filename)
{
    int bufferNum = 0;
    if (!Full){
        for (int i = 0; i < MAXBLOCKNUMBER; i++)
        {
            if (!bufferBlock[i].isValid){
                bufferBlock[i].initialize();
                bufferBlock[i].isValid = 1;
                return i;
            }
        }
        Full = true;
    }
    //If is full
    bufferNum = lru.getLRUblock();
    while (bufferBlock[bufferNum].filename == filename){
        bufferNum = lru.getLRUblock();
    }
    WriteDisk(bufferNum);
    bufferBlock[bufferNum].isValid = 1;
    return bufferNum;
}

insertPos BufferManager::getInsertPosition(Table& tableinfo)
{
    insertPos ipos;
    if (tableinfo.blockNum == 0){//the *.table file is empty and the data is firstly inserted
        ipos.bufferNUM = AddBuffer(tableinfo);
        ipos.position = 0;
        return ipos;
    }
    string filename = tableinfo.name + ".tb";
    int length = tableinfo.totalLength + 1;
    int blockOffset = tableinfo.blockNum - 1;//get the block offset in file of the last block
    int bufferNum = getIfIsInBuffer(filename, blockOffset);
    if (bufferNum == -1){//indicate that the data is not read in buffer yet
        bufferNum = getEmptyBuffer();
        readBlock(filename, blockOffset, bufferNum);
    }
    const int recordNum = BLOCKSIZE / length;//加多一位来判断这条记录是否被删除了
    for (int offset = 0; offset < recordNum; offset++){
        int position = offset * length;
        char isEmpty = bufferBlock[bufferNum].values[position];
        if (isEmpty == EMPTY){//find an empty space
            ipos.bufferNUM = bufferNum;
            ipos.position = position;
            return ipos;
        }
    }
    //if the program run till here, the last block is full, therefor one more block is added
    ipos.bufferNUM = AddBuffer(tableinfo);
    ipos.position = 0;
    return ipos;
}

int BufferManager::AddBuffer(Table& tableinfo)
{
    int bufferNum = getEmptyBuffer();
    bufferBlock[bufferNum].initialize();
    bufferBlock[bufferNum].isValid = 1;
    bufferBlock[bufferNum].isWritten = 1;
    bufferBlock[bufferNum].filename = tableinfo.name + ".tb";
    bufferBlock[bufferNum].blockOffset = tableinfo.blockNum++;
    return bufferNum;
}

int BufferManager::AddBuffer(Index& indexinfo)
{//add one more block in file for the index
    string filename = indexinfo.index_name + ".idx";
    int bufferNum = getEmptyBufferExcept(filename);
    bufferBlock[bufferNum].initialize();
    bufferBlock[bufferNum].isValid = 1;
    bufferBlock[bufferNum].isWritten = 1;
    bufferBlock[bufferNum].filename = filename;
    bufferBlock[bufferNum].blockOffset = indexinfo.blockNum++;
    return bufferNum;
}

int BufferManager::getIfIsInBuffer(string filename, int blockOffset)
{
    for (int bufferNum = 0; bufferNum < MAXBLOCKNUMBER; bufferNum++)
    if (bufferBlock[bufferNum].filename == filename && bufferBlock[bufferNum].blockOffset == blockOffset)	return bufferNum;
    return -1;	//indicate that the data is not read in buffer yet
}

void BufferManager::setInvalid(string filename)
{//when a file is deleted, a table or an index, all the value in buffer should be set invalid
    for (int i = 0; i < MAXBLOCKNUMBER; i++)
    {
        if (bufferBlock[i].filename == filename){
            bufferBlock[i].isValid = 0;
            bufferBlock[i].isWritten = 0;
        }
    }
}
