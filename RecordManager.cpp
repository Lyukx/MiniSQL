#include "RecordManager.h"

Data RecordManager::select(Table& tableinfo)
{
    Data result;
    string filename = tableinfo.name + ".tb";
    string stringrow;
    Row splitedRow;

    int length = tableinfo.totalLength + 1;//加多一位来判断这条记录是否被删除了
    const int recordNum = BLOCKSIZE / length;

    //i表示的是在buffer块中的offset
    for (int i = 0; i < tableinfo.blockNum; i++){
        //获取对应表的buffer块位置
        int bufferNum = buf.getIfIsInBuffer(filename, i);
//cout<<tableinfo.blockNum<<endl;
        if (bufferNum == -1){//表不在内存区的buffer中
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, i, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * length;
            stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);
            if (stringrow.c_str()[0] == EMPTY) continue;//inticate that this row of record have been deleted
            stringrow.erase(stringrow.begin());//把第一位去掉
            splitedRow = splitRow(tableinfo, stringrow);
            result.rows.push_back(splitedRow);
        }
    }
    return result;
}

Data RecordManager::select(Table tableinfo, vector<Condition> conditions)
{
    Data datas;
    if (conditions.size() == 0){
        datas = select(tableinfo);
        return datas;
    }
    string filename = tableinfo.name + ".tb";

    string stringrow;
    Row splitedRow;
    int length = tableinfo.totalLength + 1;//加多一位来判断这条记录是否被删除了
    const int recordNum = BLOCKSIZE / length;

    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
        int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * length;
            stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + length);
            if (stringrow.c_str()[0] != EMPTY){
                stringrow.erase(stringrow.begin());//把第一位去掉
                splitedRow = splitRow(tableinfo, stringrow);
                if (Check(tableinfo, splitedRow, conditions)){//如果满足条件，就把结果push到datas里面去
                    datas.rows.push_back(splitedRow);
                }
            }
        }
    }
    return datas;
}

void RecordManager::insertValue(Table& tableinfo, Row& splitedRow)
{
    string stringrow = connectRow(tableinfo, splitedRow);
    insertPos iPos = buf.getInsertPosition(tableinfo);
    buf.bufferBlock[iPos.bufferNUM].values[iPos.position] = NOTEMPTY;
    for (int i = 0; i < tableinfo.totalLength; i++){
        buf.bufferBlock[iPos.bufferNUM].values[iPos.position + i + 1] = stringrow.c_str()[i];
    }
    buf.bufferBlock[iPos.bufferNUM].isWritten = 1;
}

int RecordManager::deleteValue(Table tableinfo)
{
    string filename = tableinfo.name + ".tb";
    int count = 0;
    const int recordNum = BLOCKSIZE / (tableinfo.totalLength + 1);	//加多一位来判断这条记录是否被删除了
    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
        int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * (tableinfo.totalLength + 1);
            if (buf.bufferBlock[bufferNum].values[position] != EMPTY){
                buf.bufferBlock[bufferNum].values[position] = EMPTY;
                count++;
            }
        }
    }
    return count;
}

int RecordManager::deleteValue(Table tableinfo, vector<Condition> conditions)
{
    string filename = tableinfo.name + ".tb";
    string stringrow;
    Row splitedRow;
    int count = 0;
    const int recordNum = BLOCKSIZE / (tableinfo.totalLength + 1);
    for (int blockOffset = 0; blockOffset < tableinfo.blockNum; blockOffset++){
        int bufferNum = buf.getIfIsInBuffer(filename, blockOffset);
        if (bufferNum == -1){
            bufferNum = buf.getEmptyBuffer();
            buf.readBlock(filename, blockOffset, bufferNum);
        }
        for (int offset = 0; offset < recordNum; offset++){
            int position = offset * (tableinfo.totalLength + 1);
            stringrow = buf.bufferBlock[bufferNum].getvalues(position, position + tableinfo.totalLength + 1);
            if (stringrow.c_str()[0] != EMPTY){
                stringrow.erase(stringrow.begin());//删除一位
                splitedRow = splitRow(tableinfo, stringrow);
                //如果满足条件，就把记录delete掉
                if (Check(tableinfo, splitedRow, conditions)){
                    buf.bufferBlock[bufferNum].values[position] = DELETED;
                    count++;
                }
            }
        }
        buf.bufferBlock[bufferNum].isWritten = 1;
    }
    return count;
}
/*
void RecordManager::dropTable(Table tableinfo)
{
    string filename = tableinfo.name + ".tb";
    if (remove(filename.c_str()) != 0)
        perror("Error deleting file");
    else
        buf.setInvalid(filename);//when a file is deleted, a table or an index, all the value in buffer should be set invalid
}
*/
void RecordManager::dropTable(string tablename)
{
    string filename = tablename + ".tb";
    //删除文件并将对应的Buffer块置为可用
    remove(filename.c_str());
    buf.setInvalid(filename);
}

void RecordManager::createTable(Table tableinfo)
{
    string filename = tableinfo.name + ".tb";
    fstream  fout(filename.c_str(), ios::out);
    fout.close();
}

Row RecordManager::splitRow(Table tableinfo, string row)
{
    Row splitedRow;
    int s_pos = 0, f_pos = 0;
    for (int i = 0; i < tableinfo.attriNum; i++){
        s_pos = f_pos;
        f_pos += tableinfo.attributes[i].length;
        string col;
        for (int j = s_pos; j < f_pos; j++){
            col += row[j];
        }
        splitedRow.columns.push_back(col);
    }
    return splitedRow;
}

string RecordManager::connectRow(Table tableinfo, Row splitedRow)
{
    string tmptRow;
    string stringrow;
    for (int i = 0; i < splitedRow.columns.size(); i++){
        tmptRow = splitedRow.columns[i];
        for (; tmptRow.length() < tableinfo.attributes[i].length; tmptRow += EMPTY);
        stringrow += tmptRow;
    }
    return stringrow;
}

bool RecordManager::Check(Table tableinfo, Row row, vector<Condition> conditions)
{
    for (int i = 0; i < conditions.size(); i++){
        int colNum = conditions[i].columnNum;
        string value1 = "";
        string value2 = conditions[i].value;
        int length1 = 0;
        int length2 = value2.length();
        for (int k = 0; k < row.columns[colNum].length(); k++){
            if (row.columns[colNum].c_str()[k] == EMPTY){
                length1 = k;
                break;
            }
            value1 += row.columns[colNum].c_str()[k];
        }
        float fvalue1 = atof(value1.c_str());
        float fvalue2 = atof(value2.c_str());
        switch (tableinfo.attributes[colNum].type)
        {
        case CHAR:
            switch (conditions[i].op)
            {
            case Lt:
                if (value1 >= value2) return 0;
                break;
            case Le:
                if (value1 > value2) return 0;
                break;
            case Gt:
                if (value1 <= value2) return 0;
                break;
            case Ge:
                if (value1 < value2) return 0;
                break;
            case Eq:
                if (value1 != value2) return 0;
                break;
            case Ne:
                if (value1 == value2) return 0;
                break;
            }
            break;
        case INT: //整型比较，先比较长度，比如说123坑定就比99大，长度相同345与543的比较同字符串
            switch (conditions[i].op)
            {
            case Lt:
                if (length1 > length2) return 0;
                else if (length1 < length2) break;
                else if (value1 >= value2) return 0;
                break;
            case Le:
                if (length1 > length2) return 0;
                else if (length1 < length2) break;
                else if (value1 > value2) return 0;
                break;
            case Gt:
                if (length1 < length2) return 0;
                else if (length1 > length2) break;
                else if (value1 <= value2) return 0;
                break;
            case Ge:
                if (length1 < length2) return 0;
                else if (length1 > length2) break;
                else if (value1 < value2) return 0;
                break;
            case Eq:
                if (length1 != length2) return 0;
                else if (value1 != value2) return 0;
                break;
            case Ne:
                if (length1 != length2) break;
                else if (value1 == value2) return 0;
                break;
            }
            break;
        case FLOAT:// return 0;//浮点数比较大小，不知道怎么方便
            switch (conditions[i].op)
            {
            case Lt:
                if (fvalue1 >= fvalue2) return 0;
                break;
            case Le:
                if (fvalue1 > fvalue2) return 0;
                break;
            case Gt:
                if (fvalue1 <= fvalue2) return 0;
                break;
            case Ge:
                if (fvalue1 < fvalue2) return 0;
                break;
            case Eq:
                if (fvalue1 != fvalue2) return 0;
                break;
            case Ne:
                if (fvalue1 == fvalue2) return 0;
                break;
            }
        }
    }
    return 1;
}
