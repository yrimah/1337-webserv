#ifndef CONF_PARSER_HPP
# define CONF_PARSER_HPP

# include <iostream>
# include <vector>
# include "server_storage.hpp"

class Server_storage;

typedef std::vector<std::string> storage;      

class Conf_parser
{
    private:
        int servers_nbr;
        storage block_conf;
        std::vector<Server_storage>	servers;

    public:
        Conf_parser();
        ~Conf_parser();

        void store_server(std::string _file);
        void extract_server_blocks(std::string _file);
        void Server_creator(std::string &config, Server_storage &server);
        void checkServers();
        int	strCmp(std::string str1, std::string str2, size_t pos);

        void delete_spaces(std::string &_file);
        std::string first_word(std::string::size_type _start, std::string _file);

        std::string::size_type begin_serv(std::string::size_type _start, std::string _file);
        std::string::size_type end_serv(std::string::size_type _start, std::string _file);

        int getServers_nbr() const;
        std::vector<Server_storage> &getServers();

        class OutOfServerBlocks : public std::exception
        {
            public:
                virtual const char * what() const throw()
                {
                    return ("Invalid file: charachters out of server block");
                }
        };

        class ErrorEncountered : public std::exception
        {
            public:
                virtual const char * what() const throw()
                {
                    return ("Error Encountered during parsing");
                }
        };
};

#endif