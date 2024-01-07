#ifndef CONF_READER_HPP
# define CONF_READER_HPP

#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

class Conf_reader
{
    private:
        std::string path;

    public:
        Conf_reader();
        Conf_reader(std::string path);
        ~Conf_reader();

        bool validate_file() const;
        std::string read_conf() const;

        std::string getPath() const;

        static int ValidateFile(std::string const path, int mode);
        static int identifyPath(std::string const path);
        static int permValidFile(std::string const path, std::string const index);

        class InvalidFileException : public std::exception
        {
            public:
                virtual const char * what() const throw()
                {
                    return ("Invalid file !!!");
                }
        };
};

#endif
                            // send(fd, "HTTP/1.1 201 Created\r\n\r\n", 25, 0);