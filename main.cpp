#include "mainwindow.h"
#include <QApplication>

//#include "Structures.h"
//#include "BufferManager.h"
//#include "CatalogManager.cpp"
//#include "RecordManager.h"
//#include "IndexManager.h"
//#include "Interpret.h"
//#include "API.cpp"

//int main()
//{
//    vector<Condition> conditions;
//    Table tableinfor;
//    Index indexinfor;
//    Row insertValue;
//    Data datas;
//    char command[COMLEN] = "";
//    char input[INPUTLEN] = "";
//    char word[WORDLEN] = "";
//    short int ComEnd = 0;
//    vector<string> result;	//add
//    while(1)
//    {
//        strcpy(command, "");//command清零
//        ComEnd = 0;
//        cout<<"MiniSQL>>";
//        while(!ComEnd)
//        {
//            gets(input);
//            if(IsComEnd(input))
//                ComEnd = 1;
//                strcat(command, input);//保存分号结束之前的命令，不含分号
//            AddSeperator(command);//在command命令的字符串加一个空格在字符串结尾，并在空格后补上'\0'
//        }
//        parsetree.Parse(command);
//        result = Execute();
//    }
//    getchar();
//    return 0;
//}

//RecordManager record;
//IndexManager indexm;
//CatalogManager catalog;
//Interpret parsetree;
//BufferManager buf;
//API api;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
