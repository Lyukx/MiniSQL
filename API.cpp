#ifndef _API_H_
#define _API_H_

#include "Structures.h"
#include "BufferManager.h"
#include "CatalogManager.cpp"
#include "RecordManager.h"
#include "IndexManager.h"
#include "Interpret.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

extern CatalogManager catalog;
extern IndexManager indexm;
extern RecordManager record;

class API{	//上层模块只负责字符转换,语法的判断在上层进行,而catalog的判断在此处进行
public:
    //CatalogManager catalog;
    //IndexManager im;
    //RecordManager rm;
public:
    void createTable(Table aTable)
    {
        Catalog aCatalog;
        if (catalog.existCatalog(aTable.name))	//如果table已经存在
        {
            cout << "Error in API::createTable. Table already exist!" << endl;
            return;
        }
        //Transform Table to Catalog
        aCatalog.tableName = aTable.name;
        aCatalog.attributeNum = aTable.attriNum;
        aCatalog.indexNum = 0;
        aCatalog.recordNum = 0;
        aCatalog.havePrimaryKey = false;
        aCatalog.blockNum = 0;
        for (int i = 0; i < aTable.attributes.size(); i++)
        {
            if (aTable.attributes[i].isPrimeryKey)
                aCatalog.havePrimaryKey = true;
        }
        aCatalog.attributes = aTable.attributes;
        aCatalog.indexs.clear();

        catalog.addCatalog(aCatalog);	//加到catalog里
        record.createTable(aTable);		//加到table里
        return;
    }
    void dropTable(string tableName)
    {
        Catalog aCatalog;
        if (!catalog.existCatalog(tableName))	//如果table不存在
        {
            cout << "Error in API::dropTable. Table do not exist!" << endl;
            return;
        }
        aCatalog = catalog.getCatalog(tableName);
        for (int i = 0; i < aCatalog.indexs.size(); i++)	//删除table中存在的index
        {
            indexm.dropIndex(aCatalog.indexs[i]);	//index 中的删除
        }
        record.dropTable(tableName);	//recorder 中的删除
        catalog.deleteCatalog(tableName);	//Catalog 中的删除
        return;
    }
    void createIndex(Index aIndex)
    {
        Catalog aCatalog;
        Table aTable;
        if (catalog.existIndex(aIndex.index_name))
        {
            cout << "Error in API::createIndex. Index already exist!" << endl;
            return;
        }
        //得到相应的Table信息
        aCatalog = catalog.getCatalog(aIndex.table_name);
        aTable.name = aCatalog.tableName;
        aTable.blockNum = 0;
        aTable.attriNum = aCatalog.attributes.size();
        aTable.attributes = aCatalog.attributes;
        aTable.totalLength = 0;
        for (int i = 0; i < aTable.attriNum; i++)
        {
            aTable.totalLength += aCatalog.attributes[i].length;
        }
        catalog.addIndex(aIndex);	//加到catalog
        indexm.createIndex(aTable,aIndex);	//加到index
        return;
    }
    void dropIndex(string indexName)
    {
//        Index aIndex;
        if (!catalog.existIndex(indexName))	//如果不存在index
        {
            cout << "Error in API::dropIndex. Index not exist" << endl;
            return;
        }
        catalog.deleteIndex(indexName);
//        aIndex = catalog.getIndex(indexName);
//        indexm.dropIndex(aIndex);
        return;
    }
    Data select(Table& aTable,vector<Condition> conditions)
    {
        Data result;
        if (!catalog.existCatalog(aTable.name))	//如果不存在该表，返回空数据
        {
            cout << "Error in API::select. Table do not exist" << endl;
            return result;
        }
        //如果条件为空
        if (conditions.size() == 0)
        {
            result = record.select(aTable);
            return result;
        }
        //判断是否用索引来搜索	//result被赋值的地方说明按索引有结果，就返回
        if (conditions.size() == 1)
        {
            string attributeName = aTable.attributes[conditions[0].columnNum].name;	//conditions 对应的属性名
            if (catalog.existIndex(aTable.name,attributeName) && conditions[0].op == Eq)	//如果这个属性是index,而且条件是等于
            {
                Index aIndex = catalog.getIndex(aTable.name,attributeName);	//得到对应的Index信息
                result = indexm.selectEqual(aTable,aIndex,conditions[0].value);	//单属性index查询
                return result;
            }
        }
        if (conditions.size() == 2)
        {
            string attributeName1 = aTable.attributes[conditions[0].columnNum].name;
            string attributeName2 = aTable.attributes[conditions[1].columnNum].name;
            if (attributeName1.compare(attributeName2) == 0 && catalog.existIndex(aTable.name,attributeName1))
                //如果两属性相同而且是index
            {
                Index aIndex = catalog.getIndex(aTable.name,attributeName1);
                if (conditions[0].op == Lt && conditions[1].op == Gt)	//如果前为 < 后为 >
                {
                    result = indexm.selectBetween(aTable,aIndex,conditions[1].value,conditions[0].value);
                    return result;
                }
                if (conditions[1].op == Lt && conditions[0].op == Gt)	//如果前为 > 后为 <
                {
                    result = indexm.selectBetween(aTable,aIndex,conditions[0].value,conditions[1].value);
                    return result;
                }
            }
        }
        //到了此处必然为多条件的普通查询
        result = record.select(aTable,conditions);
        return result;
    }
    void insertValue(Table aTable,Row aRow)	//此处未检查catalog	//aRow.columns[i]
    {
        Catalog aCatalog = catalog.getCatalog(aTable.name);
        //先插入recorder?
        record.insertValue(aTable,aRow);
        catalog.update(aTable);
        //判断是否用index
        if (aCatalog.indexNum != 0)	//有index
        {
            for (int i = 0; i < aCatalog.indexNum; i++)	//对每一个index
            {
                int column = aCatalog.indexs[i].column;
                cout<<"fuck!"<<endl;
                indexm.insertKey(aRow.columns[column],aCatalog.indexs[i],aTable);
                catalog.update(aCatalog.indexs[i]);
            }
        }
    }
    void deleteValue(Table aTable,vector<Condition> conditions)	//此处未检查catalog
    {
        Catalog aCatalog = catalog.getCatalog(aTable.name);
        //判断是否删index
        if (aCatalog.indexNum != 0)	//有index
        {
            Data selectResult = select(aTable,conditions);	//用于提供删index需要的Key

            for (int i = 0; i < aCatalog.indexNum; i++)	//对每一个index
            {
                int column = aCatalog.indexs[i].column;
                for (int j = 0; j < selectResult.rows.size(); j++)	//对每一个记录执行这样的操作
                {
                    indexm.deleteKey(selectResult.rows[j].columns[column],aCatalog.indexs[i]);
                    catalog.update(aCatalog.indexs[i]);
                }
            }
        }
        //后删除recorder
        record.deleteValue(aTable,conditions);
        catalog.update(aTable);
    }

};

#endif

/*

CatalogManager:
void readCatalog();	//read from table.catalog
void writeCatalog();	//update table.catalog
void addCatalog(Catalog aTable);	//add catalog in vector
void deleteCatalog(string tableName);
bool existCatalog(string tableName);
Catalog getCatalog(string tableName);
void addIndex(Index indexName);
void deleteIndex(string indexName);
bool existIndex(string indexName);
bool existIndex(string tableName,string attributeName);
Index getIndex(string indexName);
Index getIndex(string tableName,string attributeName);


IndexManager:
void createIndex(const Table& tableinfo, Index& indexinfo);
void insertKey(string key, Index& indexinfo, Table& tableinfo);
void deleteKey(string key, Index& indexinfo);
Data selectEqual(const Table& tableinfo, const Index& indexinfo, string key, int blockOffset = 0);
Data selectBetween(const Table& tableinfo, const Index& indexinfo, string keyFrom, string keyTo, int blockOffset = 0);
void dropIndex(Index& indexinfo);

recorderManager:
Data select(Table& tableinfo);
Data select(Table tableinfo, vector<Condition> conditions);
void insertValue(Table& tableinfo, Row& splitedRow);
int deleteValue(Table tableinfo);
int deleteValue(Table tableinfo, vector<Condition> conditions);
void dropTable(Table tableinfo);
void createTable(Table tableinfo);

class Catalog{	//Catalog中一个表的 文件结构	//后来发现不需要具体空间信息
public:
    string tableName;	//255
    int attributeNum;	//4
    int indexNum;		//4
    int recordNum;	//4
    bool havePrimaryKey;	//4
    vector<Attribute> attributes;	// 255 + 4 + 4 + 4 + 4, name,type,isPrimaryKey,isUnique,length
    vector<Index> indexs;	//255 + 255 + 4, index_name,table_name
public:
    Catalog(){}
    Catalog(string t,int a,int i,int r,int h,vector<Attribute> va,vector<Index> vi)
    {
        tableName = t;
        attributeNum = a;
        indexNum = i;
        recordNum = r;
        havePrimaryKey = h;
        attributes = va;
        indexs = vi;
    }
};

class Attribute
{
public:
    string name;
    int type;
    int length;
    bool isPrimeryKey;
    bool isUnique;
    Attribute()
    {
     isPrimeryKey=false;
     isUnique=false;
    }
    Attribute(string n, int t, int l, bool isP, bool isU)
        :name(n), type(t), length(l), isPrimeryKey(isP), isUnique(isU){}
};

class Table
{
public:
    string name;   //all the datas is store in file name.table
    int blockNum;	//number of block the datas of the table occupied in the file name.table
    //int recordNum;	//number of record in name.table
    int attriNum;	//the number of attributes in the tables
    int totalLength;	//total length of one record, should be equal to sum(attributes[i].length)
    vector<Attribute> attributes;
    Table(): blockNum(0), attriNum(0), totalLength(0){}
};

class Index
{
public:
    string index_name;	//all the datas is store in file index_name.index
    string table_name;	//the name of the table on which the index is create
    int column;			//on which column the index is created
    int columnLength;
    int blockNum;		//number of block the datas of the index occupied in the file index_name.table
    Index(): column(0), blockNum(0){}
};

class Row
{
public:
    vector<string> columns;
};
class Data//这样就把Data搞成了一个二维数组
{
public:
    vector<Row> rows;
};

enum Comparison{Lt, Le, Gt, Ge, Eq, Ne};//stants for less than, less equal, greater than, greater equal, equal, not equal respectivly
class Condition{
public:
    Comparison op;
    int columnNum;
    string value;
};

*/
