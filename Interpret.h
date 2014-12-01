#ifndef _INTERPRET_H_
#define _INTERPRET_H_

#include "Structures.h"
#include "BufferManager.h"
#include "CatalogManager.cpp"
#include "RecordManager.h"
#include "IndexManager.h"
//#include "Interpret.h"
//#include "API.h"


class Interpret {
public:
    //解析树,所有的命令均可以反映到这样一棵树上
    int m_operation;		//要执行的操作或错误代码,用宏表示
    string m_tabname;		//要操作的表格名
    string m_indname;		//要操作的索引名
    string m_colname;
    vector<Attribute> column;
    vector<Condition> condition;			//要比较的where字句的链表
    Row row;		//要插入的值链表
    Table getTableInfo;
    Index getIndexInfo;
    int PrimaryKeyPosition;
    int UniquePostion;
    vector<Condition> UniqueCondition;

    friend vector<string> Execute();

    Interpret(){
        m_operation =UNKNOW;
        m_tabname = "";
        m_indname = "";
        m_colname="";
        PrimaryKeyPosition=-1;
        UniquePostion=-1;
    }
    ~Interpret(){}

    void Parse(char* command);
    void initcol();
    void initcond();
    void initvalue();
    void inittable(){
        getTableInfo.name="";
        getTableInfo.attriNum=getTableInfo.blockNum=getTableInfo.totalLength=0;
        getTableInfo.attributes.clear();
    }
    void initindex(){
        getIndexInfo.blockNum=getIndexInfo.column=-1;
        getIndexInfo.index_name="";
        //getIndexInfo.table_name="";
    }
    bool GetWord(string & src, string & des);

protected:

    void MakeInitilate(){
        m_operation =UNKNOW;
        m_tabname = "";
        m_indname = "";
        initcol();
        initcond();
        initvalue();
        inittable();
        initindex();
    }

    //bool GetWord(string & src, string & des);
    bool GetStr(string & src, string & des);

    short int IsInt(const char *);
    short int IsFloat(char *input);
};

#endif
