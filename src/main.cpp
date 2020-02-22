#include "Board.h"
#include "ChessManual.h"
#include "Console.h"
#include "Tools.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>

int main(int argc, char const* argv[])
{
    using namespace std::chrono;
    //std::locale loc = std::locale::global(std::locale(""));
    setlocale(LC_ALL, "chs");
    std::ios_base::sync_with_stdio(false);

    auto time0 = steady_clock::now();

    /*
    string fname = "a.txt";
    wofstream wofs(fname);
    //wcout = wofs;
    wofs << testBoard();
    wofs << testChessmanual();
    wofs.close();
    //*/
    /*
    if (argc == 7)
        testTransDir(std::stoi(argv[1]), std::stoi(argv[2]),
            std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]), std::stoi(argv[6]));
    else {
        //testTransDir(0, 2, 0, 6, 1, 6);
        //std::cout << "------------------------------------------------------------------" << std::endl;
        testTransDir(0, 2, 0, 1, 1, 6);
        testTransDir(0, 2, 1, 5, 5, 6);
        testTransDir(0, 2, 5, 6, 1, 2);
        //std::cout << "------------------------------------------------------------------" << std::endl;
        //testTransDir(0, 2, 2, 3, 1, 5);
    }
    //*/

    using namespace ConsoleSpace;
    //Console console{};
    //Console console{ };
    Console console{ "01.xqf" };

    auto time_d = steady_clock::now() - time0;
    wcout << L"use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << L"s\n";

    //std::locale::global(loc);
    return 0;
}