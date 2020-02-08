//#pragma once
#ifndef TOOLS_H
#define TOOLS_H

#include <locale>
#include <map>
#include <string>
#include <vector>

using namespace std;

namespace Tools {

extern wstring_convert<codecvt<wchar_t, char, mbstate_t>> cvt;

template <typename StrType>
StrType trim(const StrType& str);

const std::string getExtStr(const std::string& filename);

const std::wstring getWString(std::wistream& wis);

std::wstring readFile(const std::string& fileName);

void writeFile(const std::string& fileName, const std::wstring& ws);
 
void getFiles(const std::string& path, std::vector<std::string>& files);

int copyFile(const char* sourceFile, const char* newFile);

// ²âÊÔº¯Êý
const std::wstring test();

} //

#endif