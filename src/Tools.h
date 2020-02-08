//#pragma once
#ifndef TOOLS_H
#define TOOLS_H

#include <codecvt>
#include <locale>
#include <map>
#include <string>
#include <vector>

using namespace std;

extern wstring_convert<codecvt_utf8<wchar_t>> wscvt;

namespace Tools {

template <typename StrType>
StrType trim(const StrType& str);

std::wstring s2ws(const std::string& s);
std::string ws2s(const std::wstring& ws);

const std::string getExtStr(const std::string& filename);

const std::wstring getWString(std::wistream& wis);

std::wstring readFile(const std::string& fileName);

void writeFile(const std::string& fileName, const std::wstring& ws);

void getFiles(const std::string& path, std::vector<std::string>& files);

int copyFile(const char* sourceFile, const char* newFile);

const std::wstring test();

} //

#endif