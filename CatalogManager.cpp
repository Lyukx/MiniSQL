#ifndef _CATALOGMANAGER_H_
#define _CATALOGMANAGER_H_

#include "Structures.h"
//#include "BufferManager.h"
//#include "CatalogManager.h"
///#include "RecordManager.h"
//#include "IndexManager.h"
//#include "Interpret.h"
//#include "API.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;


//不使用buffer缓冲，直接全部读入内存. 对catalog的修改在内存中进行，最后从内存重新写入文件

class Catalog{	//Catalog中一个表的 文件结构	//后来发现不需要具体空间信息
public:
    string tableName;	//255
    int attributeNum;	//4
    int indexNum;		//4
    int recordNum;	//4
    bool havePrimaryKey;	//4
    int blockNum;
    vector<Attribute> attributes;	// 255 + 4 + 4 + 4 + 4, name,type,isPrimaryKey,isUnique,length
    vector<Index> indexs;	//255 + 255 + 4, index_name,table_name
public:
    Catalog(){}
    Catalog(string t,int a,int i,int r,int h,int b,vector<Attribute> va,vector<Index> vi)
    {
        tableName = t;
        attributeNum = a;
        indexNum = i;
        recordNum = r;
        havePrimaryKey = h;
        attributes = va;
        indexs = vi;
        blockNum = b;
    }
};

class CatalogManager{	//信息存放在table.catalog文件中 //信息的每一项用空格作为区分
public:
    vector<Catalog> catalogs;	//文件所有信息传到这个catalog表里
public:
    void readCatalog()	//read from table.catalog
    {
        Catalog Table;
        string filename = "catalog.txt";
        fstream  fin(filename.c_str(), ios::in);	//默认以文本方式打开
        //catalogs.clear();
        while (fin >> Table.tableName)	//如果读取到表名
        {
            Catalog aTable;
            aTable.tableName = Table.tableName;
            fin >> aTable.attributeNum;
            fin >> aTable.indexNum;
            fin >> aTable.recordNum;
            fin >> aTable.havePrimaryKey;
            fin >> aTable.blockNum;
            for (int i = 0; i < aTable.attributeNum; i++)
            {
                Attribute aaa;
                aTable.attributes.push_back(aaa);
                fin >> aTable.attributes[i].name;
                fin >> aTable.attributes[i].type;
                fin >> aTable.attributes[i].isPrimeryKey;
                fin >> aTable.attributes[i].isUnique;
                fin >> aTable.attributes[i].length;
            }
            for (int i = 0; i < aTable.indexNum; i++)
            {
                Index iii;
                aTable.indexs.push_back(iii);
                fin >> aTable.indexs[i].index_name;
                fin >> aTable.indexs[i].table_name;
                fin >> aTable.indexs[i].column;
            }
            catalogs.push_back(aTable);
        }
        fin.close();
    }
    void writeCatalog()	//update table.catalog
    {
        string filename = "catalog.txt";
        fstream fout(filename.c_str(),ios::out);
        for (int i = 0; i < catalogs.size(); i++)
        {
            fout << catalogs[i].tableName << " ";
            fout << catalogs[i].attributeNum << " ";
            fout << catalogs[i].indexNum << " ";
            fout << catalogs[i].recordNum << " ";
            fout << catalogs[i].havePrimaryKey << " ";
            fout << catalogs[i].blockNum << " ";
            for (int j = 0; j < catalogs[i].attributeNum; j++)
            {
                fout << catalogs[i].attributes[j].name << " ";
                fout << catalogs[i].attributes[j].type << " ";
                fout << catalogs[i].attributes[j].isPrimeryKey << " ";
                fout << catalogs[i].attributes[j].isUnique << " ";
                fout << catalogs[i].attributes[j].length << " ";
            }
            for (int j = 0; j < catalogs[i].indexNum; j++)
            {
                fout << catalogs[i].indexs[j].index_name << " ";
                fout << catalogs[i].indexs[j].table_name << " ";
                fout << catalogs[i].indexs[j].column << " ";
            }
        }
        fout.close();
    }
    CatalogManager()
    {
        readCatalog();
    }
    ~CatalogManager()
    {
        writeCatalog();
    }


    //1
    void addCatalog(Catalog aTable)	//add catalog in vector
    {
        catalogs.push_back(aTable);
        return;
    }
    //2
    void deleteCatalog(string tableName)	//delete catalog in vector
    {
        for(int i = 0; i < catalogs.size(); i++)
        {
            if (tableName.compare(catalogs[i].tableName) == 0){
                catalogs.erase(catalogs.begin() + i);
                return;
            }
        }
        cout << "error in delete Catalog" << endl;
    }
    //3
    bool existCatalog(string tableName)
    {
        for(int i = 0; i < catalogs.size(); i++)
        {
            if (tableName.compare(catalogs[i].tableName) == 0)
                return true;
        }
        return false;
    }
    //4
    Catalog getCatalog(string tableName)
    {
        Catalog emptyCatalog;
        for(int i = 0; i < catalogs.size(); i++)
        {
            if (tableName.compare(catalogs[i].tableName) == 0)
                return catalogs[i];
        }
        cout << "error in getCatalog" << endl;
        return emptyCatalog;
    }


    //对Index的函数是方便专门使用Index的考虑，Index 与 Catalog 是对等的关系
    //5
    void addIndex(Index indexName)
    {
        string tableName = indexName.table_name;
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (tableName.compare(catalogs[i].tableName) == 0)
            {
                catalogs[i].indexNum++;
                catalogs[i].indexs.push_back(indexName);
                return;
            }
        }
        cout << "error in addIndex" << endl;
    }
    //6
    void deleteIndex(string indexName)
    {
        for (int i = 0; i < catalogs.size(); i++)
        {
                for (int j = 0; j < catalogs[i].indexs.size(); j++)
                {
                    if (indexName.compare(catalogs[i].indexs[j].index_name) == 0)
                    {
                        catalogs[i].indexNum--;
                        catalogs[i].indexs.erase(catalogs[i].indexs.begin() + j);
                        return;
                    }
                }
        }
        cout << "error in deleteIndex" << endl;
    }
    //7
    bool existIndex(string indexName)
    {
        for(int i = 0; i < catalogs.size(); i++)
        {
            for (int j = 0; j < catalogs[i].indexs.size(); j++)
            {
                if (catalogs[i].indexs[j].index_name.compare(indexName) == 0)
                    return true;
            }
        }
        return false;
    }
    //8
    Index getIndex(string indexName)
    {
        Index emptyIndex;
        for(int i = 0; i < catalogs.size(); i++)
        {
            for (int j = 0; j < catalogs[i].indexs.size(); j++)
            {
                if (catalogs[i].indexs[j].index_name.compare(indexName) == 0)
                    return catalogs[i].indexs[j];
            }
        }
        cout << "error in getIndex" << endl;
        return emptyIndex;
    }
    //9
    bool existIndex(string tableName,string attributeName)
    {
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName.compare(tableName) == 0)	//如果是这张表
            {
                for (int j = 0; j < catalogs[i].indexs.size(); j++)
                {
                    if (catalogs[i].attributes[catalogs[i].indexs[j].column].name.compare(attributeName) == 0)
                        //如果该Index对应的attribute与提供的相同
                        return true;
                }
            }
        }
        return false;
    }
    //10
    Index getIndex(string tableName,string attributeName)
    {
        Index emptyIndex;
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName.compare(tableName) == 0)	//如果是这张表
            {
                for (int j = 0; j < catalogs[i].indexs.size(); j++)
                {
                    if (catalogs[i].attributes[catalogs[i].indexs[j].column].name.compare(attributeName) == 0)
                        //如果该Index对应的attribute与提供的相同
                        return catalogs[i].indexs[j];
                }
            }
        }
        cout << "error ind getIndex 2 " << endl;
        return emptyIndex;
    }
    //11
    Table getTable(string tableName)
    {
        Table aTable;
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName.compare(tableName) == 0)
            {
                aTable.name = catalogs[i].tableName;
                aTable.blockNum = catalogs[i].blockNum;
                aTable.attriNum = catalogs[i].attributeNum;
                aTable.attributes = catalogs[i].attributes;
                aTable.totalLength = 0;
                for (int j = 0; j < aTable.attriNum; j++)
                {
                    aTable.totalLength += catalogs[i].attributes[j].length;
                }
                return aTable;
            }
        }
        cout << "Table not found" << endl;
        return aTable;
    }
    //12
    void update(Table& aTable)
    {
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName == aTable.name)
                catalogs[i].blockNum = aTable.blockNum;
        }
    }
    //13
    void update(Index aIndex)
    {
        for(int i = 0; i < catalogs.size(); i++)
        {
            for (int j = 0; j < catalogs[i].indexs.size(); j++)
            {
                if (catalogs[i].indexs[j].index_name.compare(aIndex.index_name) == 0)
                    catalogs[i].indexs[j].blockNum = aIndex.blockNum;
            }
        }
    }
    //14
    int GetColumnNumber(Table aTable,string attributeName)
    {
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName.compare(aTable.name) == 0)
                for (int j = 0; j < catalogs[i].attributes.size(); j++)
                {
                    if (catalogs[i].attributes[j].name.compare(attributeName) == 0)
                        return j;
                }
        }
        return -1;	//表示未找到
    }
    //15
    int GetColumnAmount(Table& tableinfo){
        return tableinfo.attributes.size();
    }
    //16
    bool existIndex(string tableName,int columnNum)
    {
        for (int i = 0; i < catalogs.size(); i++)
        {
            if (catalogs[i].tableName.compare(tableName) == 0)	//如果是这张表
            {
                for (int j = 0; j < catalogs[i].indexs.size(); j++)
                {
                    if (catalogs[i].indexs[j].column == columnNum)
                        //如果该Index对应的attribute与提供的相同
                        return true;
                }
            }
        }
        return false;
    }
    void showCatalog()
    {
        cout << "table number: " << catalogs.size() << endl;
        for (int i = 0; i < catalogs.size(); i++)
        {
            cout << "tableName: " << catalogs[i].tableName << endl;
            cout << "attributeNum: " << catalogs[i].attributeNum << endl;
            cout << "indexNum:" << catalogs[i].indexNum << endl;
            cout << "recordNum:" << catalogs[i].recordNum << endl;
            cout << "havePrimaryKey:" << catalogs[i].havePrimaryKey << endl;
            cout << "blockNum" << catalogs[i].blockNum << endl;
            for (int j = 0; j < catalogs[i].attributes.size(); j++)
            {
                cout << "Attribute number " << j << ":" << endl;
                cout << "\tname: " << catalogs[i].attributes[j].name << endl;
                cout << "\ttype: " << catalogs[i].attributes[j].type << endl;
                cout << "\tisPrimaryKey: " << catalogs[i].attributes[j].isPrimeryKey << endl;
                cout << "\tisUnique: " << catalogs[i].attributes[j].isUnique << endl;
                cout << "\tlength: " << catalogs[i].attributes[j].length << endl;
            }
            for (int j = 0; j < catalogs[i].indexs.size(); j++)
            {
                cout << "Index number " << j << ":" << endl;
                cout << "\tindex_name: " << catalogs[i].indexs[j].index_name << endl;
                cout << "\ttable_name: " << catalogs[i].indexs[j].table_name << endl;
                cout << "\tcolumn: " << catalogs[i].indexs[j].column << endl;
            }
        }
    }

};

#endif

//test module

/*
int main(void)
{
    //(string n, int t, int l, bool isP, bool isU)
    //name(n), type(t), length(l), isPrimeryKey(isP), isUnique(isU){}

    //string index_name;	//all the datas is store in file index_name.index
    //string table_name;	//the name of the table on which the index is create
    //int column;			//on which column the index is created
    //int columnLength;
    //int blockNum;		//number of block the datas of the index occupied in the file index_name.table
    //Index(): column(0), blockNum(0){}

    //tableName = t;
    //attributeNum = a;
    //indexNum = i;
    //recordNum = r;
    //havePrimaryKey = h;
    //attributes = va;
    //indexs = vi;


    Attribute a1("name1",CHAR,10,true,true);
    Attribute a2("name2",INT,4,true,true);
    Index i1;
    i1.index_name = "index_for_name1";
    i1.table_name = "table1";
    i1.column = 0;

    vector<Attribute> va;
    va.push_back(a1);
    va.push_back(a2);
    vector<Index> vi;
    vi.push_back(i1);

    Catalog ca1("table1",2,1,0,1,0,va,vi);
    CatalogManager cm;
    cm.addCatalog(ca1);		//add ca1

    va.erase(va.begin());	//只有a2的属性
    vector<Index> vEmpty;	//没有index
    Catalog ca2("table2",1,0,0,0,0,va,vEmpty);
    cm.addCatalog(ca2);		//add ca2

    cout << "#####################################################################" << endl;
    cout << "Origin catalog" << endl;
    cm.showCatalog();

    cm.deleteCatalog("table1");
    cout << "Test delteCatalog" << endl;
    cm.showCatalog();
    cout << "Test existCatalog" << endl;
    cout << cm.existCatalog("table1") << " " << cm.existCatalog("table2") << endl;
    cout << endl;

    cout << "#####################################################################" << endl;
    cout << "Test for index function" << endl;
    cm.addCatalog(ca1);
    Index i2;
    i2.index_name = "index_for_name2";
    i2.table_name = "table2";
    i1.column = 0;
    cout << "Test addIndex" << endl;
    cm.addIndex(i2);
    cm.showCatalog();
    cout << "Test deleteIndex existCatalog" << endl;
    cm.deleteIndex("index_for_name2");
    cm.showCatalog();
    cout << cm.existIndex("index_for_name1") << " " << cm.existIndex("index_for_name2") << endl;

    cout << "#####################################################################" << endl;
    cout << "Test two get funcion" << endl;
    Index getIndex = cm.getIndex("index_for_name1");
    cout << getIndex.index_name << " " << getIndex.table_name << " " << getIndex.column << endl;
    Catalog getCatalog = cm.getCatalog("table1");
    cm.catalogs.clear();
    cm.addCatalog(getCatalog);
    cm.showCatalog();
    cout << "existIndex" << cm.existIndex("table1","name2") << endl;

    cout << "########################### test for updata ###########################" << endl;
    Table t438 = cm.getTable("table1");
    t438.blockNum = 999;
    cm.update(t438);
    cout << "########GetColumnNumber" << endl;
    cout << cm.GetColumnNumber(cm.getTable("table1"),"name1") << endl;
    cout << cm.GetColumnNumber(cm.getTable("table1"),"name3") << endl;
    cm.showCatalog();


    CatalogManager cm;
    cout << "wait" << endl;
    cm.showCatalog();
    return 0;
}
*/
