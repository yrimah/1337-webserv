#pragma once

#include <fstream>

class files
{
    public :
        std::ofstream file_Cgi;
        std::ofstream file;
        std::ofstream k;
    public:
        files();
        files(const files&);
        ~files();
};

