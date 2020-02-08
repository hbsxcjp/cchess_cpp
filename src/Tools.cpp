#include "Tools.h"

#include <algorithm>
#include <direct.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <sstream>

using namespace std;

wstring_convert<codecvt_utf8<wchar_t>> wscvt;

namespace Tools {


template <typename StrType>
StrType trim(const StrType& str)
{
    /*
    string::iterator pl = find_if(str.begin(), str.end(), not1(ptr_fun<int, int>(isspace)));
    str.erase(str.begin(), pl);
    string::reverse_iterator pr = find_if(str.rbegin(), str.rend(), not1(ptr_fun<int, int>(isspace)));
    str.erase(pr.base(), str.end());
    return str;
    //*/
    size_t first{ 0 }, last{ str.size() };
    for (auto& c : str)
        if (isspace(c))
            ++first;
        else
            break;
    for (int i = last - 1; i >= 0; --i)
        if (isspace(str[i]))
            --last;
        else
            break;
    return str.substr(first, last - first);
}

//string中的UTF-8字节流转换成UTF-16并保存在std::wstring中
wstring s2ws(const string& s)
{
    const char* str = s.c_str();
    size_t len = s.size() + 1;
    wchar_t* wstr = new wchar_t[len];
    mbstowcs(wstr, str, len);
    wstring ret(wstr);
    delete[] wstr;
    return ret;
}

//wstring转换到std::string
string ws2s(const wstring& ws)
{
    const wchar_t* wstr = ws.c_str();
    size_t len = 2 * ws.size() + 1;
    char* str = new char[len];
    wcstombs(str, wstr, len);
    string ret(str);
    delete[] str;
    return ret;
}

const string getExtStr(const string& filename)
{
    string ext{ filename.substr(filename.rfind('.')) };
    for (auto& c : ext)
        c = tolower(c);
    return ext;
}

const wstring getWString(wistream& wis)
{
    wstringstream wss{};
    wis >> noskipws >> wss.rdbuf(); // C++ standard library p847
    return wss.str();
}

wstring readFile(const string& fileName)
{
    wifstream wifs(fileName);
    wstring wstr = getWString(wifs);
    wifs.close();
    return wstr;
}

void writeFile(const string& fileName, const wstring& wstr)
{
    wofstream wofs(fileName);
    if (wofs)
        wofs << wstr << flush;
    wofs.close();
}

void getFiles(const string& path, vector<string>& files)
{
    long hFile = 0; //文件句柄
    struct _finddata_t fileinfo; //文件信息
    string p{};
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
        do { //如果是目录,迭代之  //如果不是,加入列表
            if (fileinfo.attrib & _A_SUBDIR) {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
            } else
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
        } while (_findnext(hFile, &fileinfo) == 0);
        _findclose(hFile);
    }
}

/*****************************************************************************************
Function:       CopyFile
Description:    复制文件
Input:          SourceFile:原文件路径 NewFile:复制后的文件路径
Return:         1:成功 0:失败
******************************************************************************************/
int copyFile(const char* SourceFile, const char* NewFile)
{
    ifstream in;
    ofstream out;

    //try
    //{
    in.open(SourceFile, ios::binary); //打开源文件
    if (in.fail()) //打开源文件失败
    {
        cout << "Error 1: Fail to open the source file." << endl;
        in.close();
        out.close();
        return 0;
    }
    out.open(NewFile, ios::binary); //创建目标文件
    if (out.fail()) //创建文件失败
    {
        cout << "Error 2: Fail to create the new file." << endl;
        out.close();
        in.close();
        return 0;
    } else //复制文件
    {
        out << in.rdbuf();
        out.close();
        in.close();
        return 1;
    }
    //}
    //catch (exception e)
    //{
    //}
}

// 测试
const wstring test()
{
    wstringstream wss{};
    return wss.str();
}
}