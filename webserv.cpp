#include "incs/parse_config/conf_reader.hpp"
#include "incs/parse_config/conf_parser.hpp"
#include "incs/parse_config/server.hpp"

int main(int argc, char **argv)
{
    try
    {
        if (argc == 1 || argc == 2)
        {
            std::string _path;
            if (argc == 1)
                _path = "configs/default.conf";
            else
                _path = argv[1];

            Conf_reader _config(_path);

            if (!_config.validate_file())
                throw Conf_reader::InvalidFileException();
            std::string _file;

            _file = _config.read_conf();
            if (_file.empty())
                throw Conf_reader::InvalidFileException();
            server k;
            k.serv.store_server(_file);
            k.creat_servers();
            k.bindAnd_ListenServers();
            return (0);
        }
        std::cerr << "Invalid number of arguments" << std::endl;
        return (1);
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}