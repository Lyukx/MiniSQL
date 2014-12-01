#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>

#include <iostream>
#include <strstream>
#include <string>
#include <vector>
#include <ctime>

#include "Structures.h"
#include "BufferManager.h"
#include "CatalogManager.cpp"
#include "RecordManager.h"
#include "IndexManager.h"
#include "Interpret.h"
#include "API.cpp"

#define MAXLENGTHOFTEXT 65536

using namespace std;

RecordManager record;
IndexManager indexm;
CatalogManager catalog;
Interpret parsetree;
BufferManager buf;
API api;

vector<string> ShowResult(Data data, Table tableinfor, vector<Attribute> column);
vector<string> Execute();
void AddSeperator(char *command);
short int IsComEnd(char *input);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //ui->groupBox_2->close();
    updateTableInfo();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Apply_SQL()
{
    QString str1 = ui->textEdit->toPlainText();
    QString str2 = "";
    //处理字符串
    QStringList input = str1.split(";");
    for(int i = 0; i < input.length(); i++){
        QString com = input.at(i);
        com.replace(QString("\n"),QString(""));
        com += " ";
        if(com.toStdString()==" ")  break;//过滤空字符串

        char *command;
        command = (char*)malloc((com.toStdString().length())*sizeof(char));
        com.toStdString().copy(command,com.toStdString().length(),0);
        *(command + com.toStdString().length()) = '\0';
        //计时开始
        clock_t start, end;
        start = clock();

        parsetree.Parse(command);
        vector<string> res;
        res = Execute();

        //计时结束
        end = clock();

        cout<<command<<": Takes time "<<end-start<<endl;

        string result = "";
        for (int i =0; i < res.size(); i++) {
            result += res[i];
            result += "\n";
        }
        str2 += "The query takes a time of " + QString::number(end - start) + "ms.\n";
        str2 += QString::fromStdString(result);
        str2 += "\n";
    }
    ui->textBrowser->setText(str2);
    this->updateTableInfo();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open file"), "",  tr("SQL File(*.sql*);;All files(*)"));
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);

    QString text = in.read(MAXLENGTHOFTEXT);

    ui->textEdit->setText(text);
}

void MainWindow::Quit()
{
    exit(0);
}

void MainWindow::updateTableInfo()
{
    ui->treeWidget->clear();
    for (int i = 0; i < catalog.catalogs.size(); i++)
    {
        QTreeWidgetItem *Table = new QTreeWidgetItem(QStringList()<<QString::fromStdString(catalog.catalogs[i].tableName));
        ui->treeWidget->addTopLevelItem(Table);
        for (int j = 0; j < catalog.catalogs[i].attributes.size(); j++)
        {
            string attr = catalog.catalogs[i].attributes[j].name;

            switch(catalog.catalogs[i].attributes[j].type){
            case INT :  attr += "\tint";     break;
            case FLOAT: attr += "\tfloat";   break;
            case CHAR:  attr += "\tchar";    break;
            }
            QTreeWidgetItem *Attribute = new QTreeWidgetItem(QStringList()<<QString::fromStdString(attr));

            if(catalog.catalogs[i].attributes[j].isPrimeryKey){
                QTreeWidgetItem *attr_pk = new QTreeWidgetItem(QStringList()<<"Primary Key");
                Attribute->addChild(attr_pk);
            }

            else if(catalog.catalogs[i].attributes[j].isUnique){
                QTreeWidgetItem *attr_uni = new QTreeWidgetItem(QStringList()<<"Unique");
                Attribute->addChild(attr_uni);
            }

            Table->addChild(Attribute);
        }

        if(catalog.catalogs[i].indexs.size() > 0){
            QTreeWidgetItem *Index = new QTreeWidgetItem(QStringList()<<"Index");
            for (int j = 0; j < catalog.catalogs[i].indexs.size(); j++)
            {
                QTreeWidgetItem *Indexname = new QTreeWidgetItem(QStringList()<<QString::fromStdString(catalog.catalogs[i].indexs[j].index_name));
                QTreeWidgetItem *Indexclm = new QTreeWidgetItem(QStringList()<<QString::fromStdString(catalog.catalogs[i].attributes[catalog.catalogs[i].indexs[j].column].name));
                Indexname->addChild(Indexclm);
                Index->addChild(Indexname);
            }
            Table->addChild(Index);
        }
    }
}

vector<string> Execute() //4次用到api
{
    vector<string> strings;	//add
    string line;	//add
    int i;
    int j;
    int k;
    Table tableinfor;
    Index indexinfor;
    string tempKeyValue;
    int tempPrimaryPosition=-1;
    int rowCount=0;
    Data data;
    switch(parsetree.m_operation)
    {
    case CRETAB:{
        parsetree.getTableInfo.attriNum = parsetree.getTableInfo.attributes.size();
        //catalog.createTable(parsetree.getTableInfo);
        //record.createTable(parsetree.getTableInfo);
        api.createTable(parsetree.getTableInfo);	//add
        cout<<"Table "<<parsetree.getTableInfo.name<<" has been created successfully"<<endl;
        line = "Table " + parsetree.getTableInfo.name + " has been created.";
        strings.push_back(line);
        }
        break;
    case TABLEEXISTED:{
        cout<<"The table has been created,please check the database"<<endl;
        line = "The table has been created,please check the database";
        strings.push_back(line);
        }
        break;

    case DRPTAB:{
        //record.dropTable(parsetree.getTableInfo);
        //for(int i = 0; i < parsetree.getTableInfo.attriNum; i++){//把这各表所有的index都删掉
        //	indexinfor = catalog.getIndexInformation(parsetree.getTableInfo.name, i);
        //	if(indexinfor.index_name != "")
        //		index.dropIndex(indexinfor);
        //}
        //catalog.dropTable(parsetree.getTableInfo);
        api.dropTable(parsetree.getTableInfo.name);	//add
        cout<<"Table "<<parsetree.getTableInfo.name<<" has been dropped successfully"<<endl;
        line = "Table " + parsetree.getTableInfo.name + " has been dropped.";
        strings.push_back(line);
        }
        break;
    case INSERT:{
        tableinfor = parsetree.getTableInfo;
        if(parsetree.PrimaryKeyPosition==-1&&parsetree.UniquePostion==-1){
            //record.insertValue(tableinfor, parsetree.row);
            //catalog.update(tableinfor);
            api.insertValue(tableinfor,parsetree.row);	//add
            cout<<"One record has been inserted successfully"<<endl;
            line = "One record has been inserted successfully";
            strings.push_back(line);
            break;
        }
        if(parsetree.PrimaryKeyPosition!=-1)
        {
            data=record.select(tableinfor, parsetree.condition);
            if(data.rows.size()>0){
                cout<<"Primary Key Redundancy occurs, thus insertion failed"<<endl;
                line = "Primary Key Redundancy occurs, thus insertion failed";
                strings.push_back(line);
                break;
            }
        }
        if(parsetree.UniquePostion!=-1){

            data=record.select(tableinfor, parsetree.UniqueCondition);
            if(data.rows.size()>0){
                cout<<"Unique Value Redundancy occurs, thus insertion failed"<<endl;
                line = "Unique Value Redundancy occurs, thus insertion failed";
                strings.push_back(line);
                break;
            }
        }
        //record.insertValue(tableinfor,parsetree.row);
        //catalog.update(tableinfor);
        api.insertValue(tableinfor,parsetree.row);	//add
        cout<<"One record has been inserted successfully"<<endl;
        line = "One record has been inserted successfully";
        strings.push_back(line);
        }
        break;
    case INSERTERR:{
        cout << "Incorrect usage of \"insert\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"insert\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case SELECT_NOWHERE_CAULSE:{
        tableinfor = parsetree.getTableInfo;
        data=record.select(tableinfor);

        if(data.rows.size()!=0)
            strings = ShowResult( data, tableinfor, parsetree.column);	//add
        else{
            cout << "No data is found!" << endl;
            line =  "No data is found!";
            strings.push_back(line);
        }
        }
        break;
    case SELECT_WHERE_CAULSE:{
        tableinfor=parsetree.getTableInfo;
        if(parsetree.condition.size()==1){
            for(int i=0;i<parsetree.getTableInfo.attributes.size();i++){
        /*修改*/if((parsetree.getTableInfo.attributes[i].isPrimeryKey==true||parsetree.getTableInfo.attributes[i].isUnique==true)
                &&parsetree.m_colname==parsetree.getTableInfo.attributes[i].name){
                    tempPrimaryPosition=i;
                    //indexinfor=catalog.getIndexInformation(tableinfor.name,i);
                    indexinfor = catalog.getIndex(tableinfor.name,tableinfor.attributes[i].name);	//add
                    break;
                }
            }
            if(tempPrimaryPosition==parsetree.condition[0].columnNum&&parsetree.condition[0].op==Eq&&indexinfor.table_name!=""){

                tempKeyValue=parsetree.condition[0].value;
                data= indexm.selectEqual(tableinfor,indexinfor,tempKeyValue);
            }
            else{

                data=record.select(tableinfor,parsetree.condition);
            }
        }
        else{
            data=record.select(tableinfor,parsetree.condition);
        }
        if(data.rows.size()!=0)
            strings = ShowResult( data, tableinfor, parsetree.column);
        else{
            cout << "No data is found!!!" << endl;
            line = "No data is found!";
            strings.push_back(line);
        }
        }
        break;
    case DELETE:{
        rowCount = record.deleteValue(parsetree.getTableInfo,parsetree.condition);
        cout<< rowCount <<"  rows have been deleted."<<endl;
        strstream ss;									//add
        string s;										//add
        ss << rowCount;									//add
        ss >> s;										//add
        line = s + "   rows hava been deleted.";
        strings.push_back(line);
        }
        break;
    case CREIND:{
        tableinfor = parsetree.getTableInfo;
        indexinfor = parsetree.getIndexInfo;
        if(!tableinfor.attributes[indexinfor.column].isPrimeryKey&&!tableinfor.attributes[indexinfor.column].isUnique){//不是primary key，不可以建index
            cout << "Column " << tableinfor.attributes[indexinfor.column].name <<"  is not unique."<< endl;
            line = "Column " + tableinfor.attributes[indexinfor.column].name +  "  is not unique.";
            strings.push_back(line);
            break;
        }
        catalog.addIndex(indexinfor);	//add
        indexm.createIndex(tableinfor, indexinfor);
        catalog.update(indexinfor);
        cout<<"The index "<< indexinfor.index_name << "has been created successfully"<<endl;
        line = "The index "+ indexinfor.index_name + "has been created successfully";
        strings.push_back(line);
        }
        break;
    case INDEXERROR:{
        cout<<"The index on primary key of table has been existed"<<endl;
        line = "The index on primary key of table has been existed";
        strings.push_back(line);
        }
        break;
    case DRPIND:{
        indexinfor = catalog.getIndex(parsetree.m_indname);
        if(indexinfor.index_name == ""){
            cout << "Index" << parsetree.m_indname << "does not exist!" << endl;
            line = "Index" + parsetree.m_indname + "does not exist!";
            strings.push_back(line);
        }
        indexm.dropIndex(indexinfor);
        //catalog.dropIndex(parsetree.m_indname);
        catalog.deleteIndex(parsetree.m_indname);
        cout<<"The index has been dropped successfully"<<endl;
        line = "The index has been dropped successfully";
        strings.push_back(line);
        }
        break;
    case CREINDERR:{
        cout << "Incorrect usage of \"create index\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"create index\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case QUIT:{
//        cout << "Have a good day! Press any key to close this window." << endl;
//        line = "Please press an key to quit";
//        strings.push_back(line);
//        getchar();
        exit(0);
        }
        break;
    case EMPTY:{
        cout << "Empty query! Please enter your command!" << endl;
        line = "Empty query! Please enter your command!";
        strings.push_back(line);
        }
        break;
    case UNKNOW:{
        cout << "UNKNOW query! Please check your input!" << endl;
        line = "UNKNOW query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case SELERR:{
        cout << "Incorrect usage of \"select\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"select\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case CRETABERR:{
        cout << "Incorrect usage of \"create table\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"create table\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case DELETEERR:{
        cout << "Incorrect usage of \"delete from\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"delete from\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case DRPTABERR:{
        cout << "Incorrect usage of \"drop table\" query! Please check your input!" << endl;
        line =  "Incorrect usage of \"drop table\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case DRPINDERR:{
        cout << "Incorrect usage of \"drop index\" query! Please check your input!" << endl;
        line = "Incorrect usage of \"drop index\" query! Please check your input!";
        strings.push_back(line);
        }
        break;
    case VOIDPRI:{
        cout << "Error: invalid primary key! Please check your input!" << endl;
        line = "Error: invalid primary key! Please check your input!";
        strings.push_back(line);
        }
        break;
    case VOIDUNI:{
        cout << "Error: invalid unique key! Please check your input!" << endl;
        line = "Error: invalid unique key! Please check your input!";
        strings.push_back(line);
        }
        break;
    case CHARBOUD:{
        cout << "Error: only 1~255 charactors is allowed! Please check your input!" << endl;
        line = "Error: only 1~255 charactors is allowed! Please check your input!";
        strings.push_back(line);
        }
        break;
    case NOPRIKEY:{
        cout << "No primary key is defined! Please check your input!" << endl;
        line = "No primary key is defined! Please check your input!";
        strings.push_back(line);
        }
        break;
    case TABLEERROR:{
        cout<<"Table is not existed,please check the database"<<endl;
        line = "Table is not existed,please check the database";
        strings.push_back(line);
        }
        break;
    case INDEXEROR:{
        cout<<"Index is not existed,please check the database"<<endl;
        line = "Index is not existed,please check the database";
        strings.push_back(line);
        }
        break;
    case COLUMNERROR:{
        cout<<"One column is not existed"<<endl;
        line = "One column is not existed";
        strings.push_back(line);
        }
        break;
    case INSERTNUMBERERROR:{
        cout<<"The column number is not according to the columns in our database"<<endl;
        line = "The column number is not according to the columns in our database";
        strings.push_back(line);
        break;
        }
    }
    return strings;
}

void AddSeperator(char *command)
{
    unsigned len = strlen(command);
    command[len] = ' ';
    command[len + 1] = '\0';
}

short int IsComEnd(char *input)
{
    int next = strlen(input) - 1;	//add unsigned
    char prev = ' ';
    while(next >= 0 && (prev == '\t' || prev ==' '))
    {
        prev = input[next];
        next --;
    }
    if(prev == ';')
    {
        input[next + 1] ='\0';
        return 1;
    }
    return 0;
}

vector<string> ShowResult(Data data, Table tableinfor, vector<Attribute> column){
    string line;
    vector<string> strings;
    if(column[0].name == "*"){
        cout << endl <<"+";
//        line = "";
//        strings.push_back(line);
//        cout << data.rows.size() << " rows have been found."<< endl;
        strstream ss;									//add
        string s;										//add
        ss << data.rows.size();							//add
        ss >> s;										//add
        line += s + " rows have been found.";
        line += "\n";
        strings.push_back(line);
        line = "";

        line += "+";
        for(int i = 0; i < tableinfor.attriNum; i++){
            for(int j = 0; j < tableinfor.attributes[i].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        cout << "| ";
        line += "| ";
        for(int i = 0; i < tableinfor.attriNum; i++){
            cout << tableinfor.attributes[i].name;
            line += tableinfor.attributes[i].name;
            int lengthLeft = tableinfor.attributes[i].length - tableinfor.attributes[i].name.length();
            for(int j = 0; j < lengthLeft; j++){
                cout << ' ';
                line += ' ';
            }
            cout << "| ";
            line += "| ";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        cout << "+";
        line += "+";
        for(int i = 0; i < tableinfor.attriNum; i++){
            for(int j = 0; j < tableinfor.attributes[i].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        //内容
        for(int i = 0; i < data.rows.size(); i++){
            cout << "| ";
            line += "| ";
            for(int j = 0; j < tableinfor.attriNum; j++){
                int lengthLeft = tableinfor.attributes[j].length;
                for(int k =0; k < data.rows[i].columns[j].length(); k++){
                    if(data.rows[i].columns[j].c_str()[k] == EMPTY) break;
                    else{
                        cout << data.rows[i].columns[j].c_str()[k];
                        line += data.rows[i].columns[j].c_str()[k];
                        lengthLeft--;
                    }
                }
                for(int k = 0; k < lengthLeft; k++)
                {
                    cout << " ";
                    line += " ";
                }
                cout << "| ";
                line += "| ";
            }
            cout << endl;
            strings.push_back(line);
            line = "";
        }

        cout << "+";
        line += "+";
        for(int i = 0; i < tableinfor.attriNum; i++){
            for(int j = 0; j < tableinfor.attributes[i].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
    }
    else{
        cout << endl <<"+";
        strings.push_back(line);
        line = "";
        line += "+";
        for(int i = 0; i < column.size(); i++){
            int col;
            for(col = 0; col < tableinfor.attributes.size(); col++){
                if(tableinfor.attributes[col].name == column[i].name) break;
            }
            for(int j = 0; j < tableinfor.attributes[col].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        cout << "| ";
        line += "| ";
        for(int i = 0; i < column.size(); i++){
            int col;
            for(col = 0; col < tableinfor.attributes.size(); col++){
                if(tableinfor.attributes[col].name == column[i].name) break;
            }
            cout << tableinfor.attributes[col].name;
            line += tableinfor.attributes[col].name;
            int lengthLeft = tableinfor.attributes[col].length - tableinfor.attributes[col].name.length();
            for(int j = 0; j < lengthLeft; j++){
                cout << ' ';
                line += ' ';
            }
            cout << "| ";
            line += "| ";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        cout << "+";
        line += "+";
        for(int i = 0; i < column.size(); i++){
            int col;
            for(col = 0; col < tableinfor.attributes.size(); col++){
                if(tableinfor.attributes[col].name == column[i].name) break;
            }
            for(int j = 0; j < tableinfor.attributes[col].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
        //内容
        for(int i = 0; i < data.rows.size(); i++){
            cout << "| ";
            line += "| ";
            for(int j = 0; j < column.size(); j++){
                int col;
                for(col = 0; col < tableinfor.attributes.size(); col++){
                    if(tableinfor.attributes[col].name == column[j].name) break;
                }
                int lengthLeft = tableinfor.attributes[col].length;
                for(int k =0; k < data.rows[i].columns[col].length(); k++){
                    if(data.rows[i].columns[col].c_str()[k] == EMPTY) break;
                    else{
                        cout << data.rows[i].columns[col].c_str()[k];
                        lengthLeft--;
                    }
                }
                for(int k = 0; k < lengthLeft; k++)
                {
                    cout << " ";
                    line += " ";
                }
                cout << "| ";
                line += "| ";
            }
            cout << endl;
            strings.push_back(line);
            line = "";
        }

        cout << "+";
        line += "+";
        for(int i = 0; i < column.size(); i++){
            int col;
            for(col = 0; col < tableinfor.attributes.size(); col++){
                if(tableinfor.attributes[col].name == column[i].name) break;
            }
            for(int j = 0; j < tableinfor.attributes[col].length + 1; j++){
                cout << "-";
                line += "-";
            }
            cout << "+";
            line += "+";
        }
        cout << endl;
        strings.push_back(line);
        line = "";
    }


    return strings;

}
