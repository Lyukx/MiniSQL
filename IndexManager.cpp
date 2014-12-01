
#include "IndexManager.h"

void IndexManager::createIndex(const Table& tableinfo, Index& indexinfo){
    //创建保存index信息的idx文件
    string filename = indexinfo.index_name + ".idx";
    fstream  fout(filename.c_str(), ios::out);
    fout.close();

    //创建根
    int blockNum = buf.getEmptyBuffer();
    buf.bufferBlock[blockNum].filename = filename;
    buf.bufferBlock[blockNum].blockOffset = 0;
    buf.bufferBlock[blockNum].isWritten = 1;
    buf.bufferBlock[blockNum].isValid = 1;

    //刚刚创建时的根节点既是根节点也是叶节点
    buf.bufferBlock[blockNum].values[0] = 'R';
    buf.bufferBlock[blockNum].values[1] = 'L';

    //添加有多少记录
    memset(buf.bufferBlock[blockNum].values + 2, '0', 4);//现在记录条数为零

    //存放一个父指针，两个兄弟指针
    for (int i = 0; i < 3; i++)
        memset(buf.bufferBlock[blockNum].values + 6 + POINTERLENGTH*i, '0', POINTERLENGTH);
    indexinfo.blockNum++;

    //retrieve datas of the table and form a B+ Tree
    filename = tableinfo.name + ".tb";
    string stringrow;
    string key;

    int length = tableinfo.totalLength + 1;//加多一位来判断这条记录是否被删除了
    const int recordNum = BLOCKSIZE / length;

    //read datas from the record and sort it into a B+ Tree and store it
    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
        int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * length;
            stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);
            if (stringrow.c_str()[0] == EMPTY) continue;//inticate that this row of record have been deleted
            stringrow.erase(stringrow.begin());	//把第一位去掉
            key = getColumnValue(tableinfo, indexinfo, stringrow);
            IndexLeaf node(key, blockOffset, offset);
            insertValue(indexinfo, node);
        }
    }
}

void IndexManager::insertKey(string key, Index& indexinfo, Table& tableinfo)
{
    string filename = tableinfo.name + ".tb";
    string stringrow;

    int length = tableinfo.totalLength + 1;//加多一位来判断这条记录是否被删除了
    const int recordNum = BLOCKSIZE / length;

    //read datas from the record and sort it into a B+ Tree and store it
    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
        int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * length;
            stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);
            if (stringrow.c_str()[0] == EMPTY) continue;//inticate that this row of record have been deleted
            stringrow.erase(stringrow.begin());	//把第一位去掉
            if (getColumnValue(tableinfo, indexinfo, stringrow) == key){
                IndexLeaf node(key, blockOffset, offset);
                insertValue(indexinfo, node);
            }
        }
    }
}

IndexBranch IndexManager::insertValue(Index& indexinfo, IndexLeaf node, int blockOffset){
    IndexBranch reBranch;//for return, intial to be empty
    string filename = indexinfo.index_name + ".idx";
    int bufferNum = buf.getbufferNum(filename, blockOffset);
    if (bufferNum == -1){
        bufferNum = buf.getEmptyBuffer();
        buf.readBlock(filename, blockOffset, bufferNum);
    }
    buf.bufferBlock[bufferNum].isWritten = 1;
    bool isLeaf = (buf.bufferBlock[bufferNum].values[1] == 'L');// L for leaf
    if (isLeaf){
        Leaf leaf(bufferNum, indexinfo);
        leaf.insert(node);

        //判断是否要分裂
        const int RecordLength = indexinfo.columnLength + POINTERLENGTH * 2;
        const int MaxrecordNum = (BLOCKSIZE - 6 - POINTERLENGTH * 3) / RecordLength;
        if (leaf.recordNum > MaxrecordNum){//record number is too great, need to split
            if (leaf.isRoot){//this leaf is a root
                int rbufferNum = leaf.bufferNum;	// buffer number for new root
                leaf.bufferNum = buf.AddBuffer(indexinfo);	//find a new place for old leaf
                int sbufferNum = buf.AddBuffer(indexinfo);	// buffer number for sibling
                Branch branchRoot(rbufferNum);	//new root, which is branch indeed
                Leaf leafadd(sbufferNum);	//sibling

                //is root
                branchRoot.isRoot = 1;
                leafadd.isRoot = 0;
                leaf.isRoot = 0;

                branchRoot.ptrFather = leafadd.ptrFather = leaf.ptrFather = 0;
                branchRoot.columnLength = leafadd.columnLength = leaf.columnLength;
                //link the newly added leaf block in the link list of leaf
                leafadd.lastSibling = buf.bufferBlock[leaf.bufferNum].blockOffset;
                leaf.nextSibling = buf.bufferBlock[leafadd.bufferNum].blockOffset;
                while (leafadd.nodelist.size() < leaf.nodelist.size()){
                    IndexLeaf tnode = leaf.pop();
                    leafadd.insert(tnode);
                }

                IndexBranch tmptNode;
                tmptNode.key = leafadd.getfront().key;
                tmptNode.ptrChild = buf.bufferBlock[leafadd.bufferNum].blockOffset;
                branchRoot.insert(tmptNode);
                tmptNode.key = leaf.getfront().key;
                tmptNode.ptrChild = buf.bufferBlock[leaf.bufferNum].blockOffset;
                branchRoot.insert(tmptNode);
                return reBranch;
            }
            else{//this leaf is not a root, we have to cascade up
                int bufferNum = buf.AddBuffer(indexinfo);
                Leaf leafadd(bufferNum);
                leafadd.isRoot = 0;
                leafadd.ptrFather = leaf.ptrFather;
                leafadd.columnLength = leaf.columnLength;

                //link the newly added leaf block in the link list of leaf
                leafadd.nextSibling = leaf.nextSibling;
                leafadd.lastSibling = buf.bufferBlock[leaf.bufferNum].blockOffset;
                leaf.nextSibling = buf.bufferBlock[leafadd.bufferNum].blockOffset;
                if (leafadd.nextSibling != 0){
                    int bufferNum = buf.getbufferNum(filename, leafadd.nextSibling);
                    Leaf leafnext(bufferNum, indexinfo);
                    leafnext.lastSibling = buf.bufferBlock[leafadd.bufferNum].blockOffset;
                }
                while (leafadd.nodelist.size() < leaf.nodelist.size()){
                    IndexLeaf tnode = leaf.pop();
                    leafadd.insert(tnode);
                }
                reBranch.key = leafadd.getfront().key;
                reBranch.ptrChild = leaf.nextSibling;
                return reBranch;
            }
        }
        else{//not need to split,just return
            return reBranch;
        }

    }
    else{//not a leaf
        Branch branch(bufferNum, indexinfo);
        list<IndexBranch>::iterator i = branch.nodelist.begin();
        if ((*i).key > node.key){	//如果新插入的值比最左边的还要小
            (*i).key = node.key;	//就跟新最左边的值
        }
        else{
            for (i = branch.nodelist.begin(); i != branch.nodelist.end(); i++)
            if ((*i).key > node.key) break;
            i--;//得到(*i) 左边的指针的位置
        }
        IndexBranch bnode = insertValue(indexinfo, node, (*i).ptrChild);//go down

        if (bnode.key == ""){
            return reBranch;
        }
        else{//bnode.key != "", 说明底层的B树快发生了split（分裂），要作出相应的操作
            branch.insert(bnode);
            const int RecordLength = indexinfo.columnLength + POINTERLENGTH;
            const int MaxrecordNum = (BLOCKSIZE - 6 - POINTERLENGTH) / RecordLength;
            if (branch.recordNum > MaxrecordNum){//need to split up
                if (branch.isRoot){
                    int rbufferNum = branch.bufferNum;	// buffer number for new root
                    branch.bufferNum = buf.AddBuffer(indexinfo);	//find a new place for old branch
                    int sbufferNum = buf.AddBuffer(indexinfo);	// buffer number for sibling
                    Branch branchRoot(rbufferNum);	//new root
                    Branch branchadd(sbufferNum);	//sibling

                    //is root
                    branchRoot.isRoot = 1;
                    branchadd.isRoot = 0;
                    branch.isRoot = 0;

                    branchRoot.ptrFather = branchadd.ptrFather = branch.ptrFather = 0;
                    branchRoot.columnLength = branchadd.columnLength = branch.columnLength;

                    while (branchadd.nodelist.size() < branch.nodelist.size()){
                        IndexBranch tnode = branch.pop();
                        branchadd.insert(tnode);
                    }

                    IndexBranch tmptNode;
                    tmptNode.key = branchadd.getfront().key;
                    tmptNode.ptrChild = buf.bufferBlock[branchadd.bufferNum].blockOffset;
                    branchRoot.insert(tmptNode);
                    tmptNode.key = branch.getfront().key;
                    tmptNode.ptrChild = buf.bufferBlock[branch.bufferNum].blockOffset;
                    branchRoot.insert(tmptNode);
                    return reBranch;//here the function must have already returned to the top lay
                }
                else{//branch is not a root
                    int bufferNum = buf.AddBuffer(indexinfo);
                    Branch branchadd(bufferNum);
                    branchadd.isRoot = 0;
                    branchadd.ptrFather = branch.ptrFather;
                    branchadd.columnLength = branch.columnLength;

                    while (branchadd.nodelist.size() < branch.nodelist.size()){
                        IndexBranch tnode = branch.pop();
                        branchadd.insert(tnode);
                    }
                    reBranch.key = branchadd.getfront().key;
                    reBranch.ptrChild = buf.bufferBlock[bufferNum].blockOffset;
                    return reBranch;
                }
            }
            else{//not need to split,just return
                return reBranch;
            }
        }
    }
    return reBranch;//here is just for safe
}

Data IndexManager::selectEqual(const Table& tableinfo, const Index& indexinfo, string key, int blockOffset)
{
    //start from the root and look down
    Data datas;
    string filename = indexinfo.index_name + ".idx";
    int bufferNum = buf.getbufferNum(filename, blockOffset);
    bool isLeaf = (buf.bufferBlock[bufferNum].values[1] == 'L');// L for leaf
    if (isLeaf){
        Leaf leaf(bufferNum, indexinfo);
        list<IndexLeaf>::iterator i = leaf.nodelist.begin();
        for (i = leaf.nodelist.begin(); i != leaf.nodelist.end(); i++)
        if ((*i).key == key){
            filename = indexinfo.table_name + ".tb";
            int recordBufferNum = buf.getbufferNum(filename, (*i).offsetInFile);//把记录读进buffer
            int position = (tableinfo.totalLength + 1)* ((*i).offsetInBlock);
            string stringrow = buf.bufferBlock[recordBufferNum].getvalues(position, position + tableinfo.totalLength);
            if (stringrow.c_str()[0] != EMPTY){
                stringrow.erase(stringrow.begin());//把第一位去掉
                Row splitedRow = splitRow(tableinfo, stringrow);
                datas.rows.push_back(splitedRow);
                return datas;
            }
        }
    }
    else{	//it is not a leaf
        Branch branch(bufferNum, indexinfo);
        list<IndexBranch>::iterator i = branch.nodelist.begin();
        for (i = branch.nodelist.begin(); i != branch.nodelist.end(); i++){
            if ((*i).key > key){
                i--;//得到(*i) 左边的指针的位置
                break;
            }
        }
        if (i == branch.nodelist.end()) i--;
        datas = selectEqual(tableinfo, indexinfo, key, (*i).ptrChild);
    }
    return datas;
}
Data IndexManager::selectBetween(const Table& tableinfo, const Index& indexinfo, string keyFrom, string keyTo, int blockOffset)
{
    Data datas;
    string filename = indexinfo.index_name + ".idx";
    int bufferNum = buf.getbufferNum(filename, blockOffset);
    bool isLeaf = (buf.bufferBlock[bufferNum].values[1] == 'L');// L for leaf
    if (isLeaf){
        do{
            Leaf leaf(bufferNum, indexinfo);
            list<IndexLeaf>::iterator i;
            for (i = leaf.nodelist.begin(); i != leaf.nodelist.end(); i++){
                if ((*i).key >= keyFrom){
                    if ((*i).key > keyTo){
                        return datas;
                    }
                    filename = indexinfo.table_name + ".tb";
                    int recordBufferNum = buf.getbufferNum(filename, (*i).offsetInFile);//把记录读进buffer
                    int position = (tableinfo.totalLength + 1)* ((*i).offsetInBlock);
                    string stringrow = buf.bufferBlock[recordBufferNum].getvalues(position, position + tableinfo.totalLength);
                    if (stringrow.c_str()[0] != EMPTY){
                        stringrow.erase(stringrow.begin());//把第一位去掉
                        Row splitedRow = splitRow(tableinfo, stringrow);
                        datas.rows.push_back(splitedRow);
                    }
                }
            }
            if (leaf.nextSibling != 0){
                filename = indexinfo.index_name + ".idx";
                bufferNum = buf.getbufferNum(filename, leaf.nextSibling);
            }
            else return datas;
        } while (1);
    }
    else{//not leaf, go down to the leaf
        Branch branch(bufferNum, indexinfo);
        list<IndexBranch>::iterator i = branch.nodelist.begin();
        if ((*i).key > keyFrom){//如果keyFrom 比最小的键值还要小，就在最左边开始找下去
            datas = selectBetween(tableinfo, indexinfo, keyFrom, keyTo, (*i).ptrChild);
            return datas;
        }
        else{//否则就进入循环，找到入口
            for (i = branch.nodelist.begin(); i != branch.nodelist.end(); i++){
                if ((*i).key > keyFrom){
                    i--;//得到(*i) 左边的指针的位置
                    break;
                }
            }
            datas = selectBetween(tableinfo, indexinfo, keyFrom, keyTo, (*i).ptrChild);
            return datas;
        }
    }
    return datas;
}
void IndexManager::dropIndex(Index& indexinfo)
{
    string filename = indexinfo.index_name + ".idx";
    if (remove(filename.c_str()) != 0)
        perror("Error deleting file");
    else
        buf.setInvalid(filename);
}
Row IndexManager::splitRow(Table tableinfo, string row)
{
    Row splitedRow;
    int s_pos = 0, f_pos = 0;
    for (int i = 0; i < tableinfo.attriNum; i++){
        s_pos = f_pos;
        f_pos += tableinfo.attributes[i].length;
        string col;
        for (int j = s_pos; j < f_pos; j++){
            if (row[j] == EMPTY) break;
            col += row[j];
        }
        splitedRow.columns.push_back(col);
    }
    return splitedRow;
}

string IndexManager::getColumnValue(const Table& tableinfo, const Index& indexinfo, string row)
{
    string colValue;
    int s_pos = 0, f_pos = 0;
    for (int i = 0; i <= indexinfo.column; i++){
        s_pos = f_pos;
        f_pos += tableinfo.attributes[i].length;
    }
    for (int j = s_pos; j < f_pos && row[j] != EMPTY; j++)
        colValue += row[j];
    return colValue;
}
