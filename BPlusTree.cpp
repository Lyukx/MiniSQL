#include "BPlusTree.h"

int BPlusTree::getPtr(int pos)
{
    int ptr = 0;
    for (int i = pos; i < pos + POINTERLENGTH; i++){
        ptr = 10 * ptr + (buf.bufferBlock[bufferNum].values[i] - '0');
    }
    return ptr;
}
int BPlusTree::getRecordNum()
{
    int recordNum = 0;
    for (int i = 2; i < 6; i++){
        if (buf.bufferBlock[bufferNum].values[i] == EMPTY){
            break;
        }
        recordNum = 10 * recordNum + (buf.bufferBlock[bufferNum].values[i] - '0');
    }
    return recordNum;
}

Branch::Branch(int bufNum, const Index& indexinfo)
{
    bufferNum = bufNum;
    isRoot = (buf.bufferBlock[bufferNum].values[0] == 'R');
    int recordCount = getRecordNum();
    recordNum = 0;
    ptrFather = getPtr(6);
    columnLength = indexinfo.columnLength;
    int pos = 6 + POINTERLENGTH;
    for (int i = 0; i < recordCount; i++) {
        string key = "";
        for (int i = pos; i < pos + columnLength; i++){
            if (buf.bufferBlock[bufferNum].values[i] == EMPTY) break;
            else key += buf.bufferBlock[bufferNum].values[i];
        }
        pos += columnLength;
        int ptrChild = getPtr(pos);
        pos += POINTERLENGTH;
        IndexBranch node(key, ptrChild);
        insert(node);
    }
}
Branch::~Branch()
{
    //isRoot
    buf.bufferBlock[bufferNum].values[0] = isRoot ? 'R' : '_';
    //is not a Leaf
    buf.bufferBlock[bufferNum].values[1] = '_';
    //recordNum
    char tmpt[5];
    //itoa(recordNum, tmpt, 10);
    sprintf(tmpt,"%d",recordNum);
    string strRecordNum = tmpt;
    while (strRecordNum.length() < 4)
        strRecordNum = '0' + strRecordNum;
    strncpy(buf.bufferBlock[bufferNum].values + 2, strRecordNum.c_str(), 4);

    list<IndexBranch>::iterator i;
    int position = 6 + POINTERLENGTH;
    for (i = nodelist.begin(); i != nodelist.end(); i++)
    {
        string key = (*i).key;
        while (key.length() < columnLength)
            key += EMPTY;
        strncpy(buf.bufferBlock[bufferNum].values + position, key.c_str(), columnLength);
        position += columnLength;

        char tmpt[5];
        //itoa((*i).ptrChild, tmpt, 10);
        sprintf(tmpt,"%d",(*i).ptrChild);
        string ptrChild = tmpt;
        while (ptrChild.length() < POINTERLENGTH)
            ptrChild = '0' + ptrChild;
        strncpy(buf.bufferBlock[bufferNum].values + position, ptrChild.c_str(), POINTERLENGTH);
        position += POINTERLENGTH;
    }
}
void Branch::insert(IndexBranch node)
{
    recordNum++;
    list<IndexBranch>::iterator i = nodelist.begin();
    if (nodelist.size() == 0)
        nodelist.insert(i, node);
    else{
        for (i = nodelist.begin(); i != nodelist.end(); i++)
        if ((*i).key > node.key) break;
        nodelist.insert(i, node);
    }
}
IndexBranch Branch::pop()
{
    recordNum--;
    IndexBranch tmpt = nodelist.back();
    nodelist.pop_back();
    return tmpt;
}
IndexBranch Branch::getfront()
{
    return nodelist.front();
}


Leaf::Leaf(int bufNum){
    bufferNum = bufNum;
    recordNum = 0;
    nextSibling = lastSibling = 0;
}
Leaf::Leaf(int bufNum, const Index& indexinfo){
    bufferNum = bufNum;
    isRoot = (buf.bufferBlock[bufferNum].values[0] == 'R');
    int recordCount = getRecordNum();
    recordNum = 0;
    ptrFather = getPtr(6);
    lastSibling = getPtr(6 + POINTERLENGTH);
    nextSibling = getPtr(6 + POINTERLENGTH * 2);
    columnLength = indexinfo.columnLength;	//保存起来以后析构函数还要用到的

    int position = 6 + POINTERLENGTH * 3;	//前面的几位用来存储index的相关信息了
    for (int i = 0; i < recordCount; i++)
    {
        string key = "";
        for (int i = position; i < position + columnLength; i++){
            if (buf.bufferBlock[bufferNum].values[i] == EMPTY) break;
            else key += buf.bufferBlock[bufferNum].values[i];
        }
        position += columnLength;
        int offsetInFile = getPtr(position);
        position += POINTERLENGTH;
        int offsetInBlock = getPtr(position);
        position += POINTERLENGTH;
        IndexLeaf node(key, offsetInFile, offsetInBlock);
        insert(node);
    }
}
Leaf::~Leaf(){
    //isRoot
    buf.bufferBlock[bufferNum].values[0] = isRoot ? 'R' : '_';
    //isLeaf
    buf.bufferBlock[bufferNum].values[1] = 'L';
    //recordNum
    char tmpt[5];
    //itoa(recordNum, tmpt, 10);
    sprintf(tmpt,"%d",recordNum);
    string strRecordNum = tmpt;
    while (strRecordNum.length() < 4)
        strRecordNum = '0' + strRecordNum;
    int position = 2;
    strncpy(buf.bufferBlock[bufferNum].values + position, strRecordNum.c_str(), 4);
    position += 4;

    //itoa(ptrFather, tmpt, 10);
    sprintf(tmpt,"%d",ptrFather);
    string strptrFather = tmpt;
    while (strptrFather.length() < POINTERLENGTH)
        strptrFather = '0' + strptrFather;
    strncpy(buf.bufferBlock[bufferNum].values + position, strptrFather.c_str(), POINTERLENGTH);
    position += POINTERLENGTH;

    //itoa(lastSibling, tmpt, 10);
    sprintf(tmpt,"%d",lastSibling);
    string strLastSibling = tmpt;
    while (strLastSibling.length() < POINTERLENGTH)
        strLastSibling = '0' + strLastSibling;
    strncpy(buf.bufferBlock[bufferNum].values + position, strLastSibling.c_str(), POINTERLENGTH);
    position += POINTERLENGTH;

    //itoa(nextSibling, tmpt, 10);
    sprintf(tmpt,"%d",nextSibling);
    string strNextSibling = tmpt;
    while (strNextSibling.length() < POINTERLENGTH)
        strNextSibling = '0' + strNextSibling;
    strncpy(buf.bufferBlock[bufferNum].values + position, strNextSibling.c_str(), POINTERLENGTH);
    position += POINTERLENGTH;

    list<IndexLeaf>::iterator i;
    for (i = nodelist.begin(); i != nodelist.end(); i++)
    {
        string key = (*i).key;
        while (key.length() < columnLength)
            key += EMPTY;
        strncpy(buf.bufferBlock[bufferNum].values + position, key.c_str(), columnLength);
        position += columnLength;

        //itoa((*i).offsetInFile, tmpt, 10);
        sprintf(tmpt,"%d",(*i).offsetInFile);
        string offsetInFile = tmpt;
        while (offsetInFile.length() < POINTERLENGTH)
            offsetInFile = '0' + offsetInFile;
        strncpy(buf.bufferBlock[bufferNum].values + position, offsetInFile.c_str(), POINTERLENGTH);
        position += POINTERLENGTH;

        //itoa((*i).offsetInBlock, tmpt, 10);
        sprintf(tmpt,"%d",(*i).offsetInBlock);
        string offsetInBlock = tmpt;
        while (offsetInBlock.length() < POINTERLENGTH)
            offsetInBlock = '0' + offsetInBlock;
        strncpy(buf.bufferBlock[bufferNum].values + position, offsetInBlock.c_str(), POINTERLENGTH);
        position += POINTERLENGTH;
    }
}

void Leaf::insert(IndexLeaf node){
    recordNum++;
    list<IndexLeaf>::iterator i = nodelist.begin();
    if (nodelist.size() == 0){
        nodelist.insert(i, node);
        return;
    }
    else{
        for (i = nodelist.begin(); i != nodelist.end(); i++)
        if ((*i).key > node.key) break;
    }
    nodelist.insert(i, node);
}
IndexLeaf Leaf::pop(){
    recordNum--;
    IndexLeaf tmpt = nodelist.back();
    nodelist.pop_back();
    return tmpt;
}
IndexLeaf Leaf::getfront(){
    return nodelist.front();
}
