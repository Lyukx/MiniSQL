#ifndef _STRUCTURES_H_
#define _STRUCTURES_H_

#define INT 1
#define FLOAT 2
#define CHAR 3
#define INTLENGTH 11
#define FLOATLENGTH 10
#define INTLEN		11
#define FLOATLEN	10

#define ISPRIMARYKEY 1
#define NOTPRIMARYKEY 0
#define ISUNIQUE 1
#define NOTUNIQUE 0


#define MAXBLOCKNUMBER 1024	//should be 10000	//add 1000->1024
#define BLOCKSIZE 4096	//should be 4096
#define EMPTY '\0'
#define END '\0'
#define NOTEMPTY '1'
#define DELETED '\0'


#define MAXPRIMARYKEYLENGTH  25    //should change sometime

#define UNKNOW		12
#define SELERR		13
#define	CREINDERR	14
#define CRETABERR	15
#define DELETEERR	16
#define INSERTERR	17
#define DRPTABERR	18
#define DRPINDERR	19
#define EXEFILERR	20
#define NOPRIKEY    27
#define VOIDPRI		21
#define VOIDUNI		22
#define CHARBOUD    23
#define TABLEERROR  24
#define COLUMNERROR 25
#define INSERTNUMBERERROR 26
#define SELECT_NOWHERE_CAULSE  29
#define SELECT_WHERE_CAULSE    28
#define TABLEEXISTED  30
#define INDEXERROR    31
#define INDEXEROR     32

#define SELECT		0
#define CRETAB		1
#define	CREIND		2
#define	DRPTAB		3
#define DRPIND		4
#define DELETE		5
#define INSERT		6
#define QUIT		7
#define EXEFILE		8

#define COMLEN		400
#define INPUTLEN	200
#define WORDLEN		100
#define VALLEN		300
#define NAMELEN		100


#include <string>
#include <vector>
using namespace std;

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

/*
class buffer{
public:
    bool isWritten;
    bool isValid;
    string filename;
    int blockOffset;	//block offset in file, indicate position in file
    int LRUvalue;		//用于实现LRU算法,the lower, the better
    char values[BLOCKSIZE + 1];	//c++的用法怎么不对？ char* values = new char[BLOCKSIZE];
    buffer(){
        initialize();
    }
    void initialize(){
        isWritten  = 0;
        isValid = 0;
        filename = "NULL";
        blockOffset = 0;
        LRUvalue= 0;
        for(int i = 0; i< BLOCKSIZE; i++) values[i] = EMPTY;
        values[BLOCKSIZE] = '\0';
    }
    string getvalues(int startpos, int endpos){
        string tmpt = "";
        if(startpos >=0 && startpos <= endpos && endpos <= BLOCKSIZE)
            for(int i = startpos; i< endpos; i++)
                tmpt += values[i];
        return tmpt;
    }
    char getvalues(int pos){
        if(pos >=0 && pos <= BLOCKSIZE)
            return values[pos];
        return '\0';
    }
};

*/

class insertPos{
public:
    int bufferNUM;
    int position;
};

#endif
