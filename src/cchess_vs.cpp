// cchess_vs.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "Board.h"
#include "ChessManual.h"
#include "Tools.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <locale>

int main(int argc, char const* argv[])
{
    try {
        using namespace std::chrono;
        //std::locale loc = std::locale::global(std::locale(""));
        setlocale(LC_ALL, "chs");
        std::ios_base::sync_with_stdio(false);

        auto time0 = steady_clock::now();

        /* 
        string fname = "board.txt";
        wofstream ofs(fname);
        ofs << testBoard() << flush;
        ofs.close();
        //*/
        //Tools::writeFile(fname, testBoard());
        std::wcout << testBoard();
        //std::wcout << testChessmanual();
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

        auto time_d = steady_clock::now() - time0;
        std::cout << "use time: " << duration_cast<milliseconds>(time_d).count() / 1000.0 << "s\n";

        //std::locale::global(loc);
        return 0;
    } catch (runtime_error err) {
        cout << err.what() << endl;
    }
    return -74;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
