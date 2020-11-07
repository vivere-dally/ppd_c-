#include "help.h"
#include <fstream>
#include <cstdlib>
#include <ctime>

void rand_gen_file(std::string file, int size, long long min, long long max)
{
    srand((int)time(0));
    std::ofstream fout(file);
    for (int i = 0; i < size; i++)
    {
        fout << min + rand() % (min + max + 1);
        if (i != size - 1)
        {
            fout << ",";
        }
    }

    fout << "\n";
    fout.close();
}

bool are_files_equals(std::string file1, std::string file2)
{

    return true;
}

void excel_write_line(std::string file, std::string data)
{
}

void excel_write_line(std::string file, std::string data, int line)
{
}

void trigger_program(std::string cmd, std::string args, int number_of_runs)
{
}
