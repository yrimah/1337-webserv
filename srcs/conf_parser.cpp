#include "../incs/parse_config/conf_parser.hpp"
#include "../incs/parse_config/conf_reader.hpp"

Conf_parser::Conf_parser()
{
    this->servers_nbr = 0;
}

Conf_parser::~Conf_parser()
{
}

void Conf_parser::delete_spaces(std::string &_file)
{
    std::string::size_type _start = 0;
    std::string::size_type _end = _file.size() - 1;

    while (_file[_start] && isspace(_file[_start]))
        _start++;
    while (_end != _start && isspace(_file[_end]))
        _end--;
    _file = _file.substr(_start, _end - _start + 1);
}

std::string Conf_parser::first_word(std::string::size_type _start, std::string _file)
{
	(void)_start;
    int counter = 0;

    while (isspace(_file[counter]))
        counter++;
    int tmp = counter;
    if (!_file[counter])
        throw OutOfServerBlocks();
    while (_file[counter] && !isspace(_file[counter]))
        counter++;
    return (_file.substr(tmp, counter - tmp));
}

std::string::size_type Conf_parser::begin_serv(std::string::size_type _start, std::string _file)
{
    std::string::size_type counter = _start;

    if (first_word(_start, _file) != "server")
        throw OutOfServerBlocks();
    counter += first_word(_start, _file).size() + 1;
    while (isspace(_file[counter]))
        counter++;
    if (_file[counter] != '{')
        throw OutOfServerBlocks();
    return (counter);
}

std::string::size_type Conf_parser::end_serv(std::string::size_type _start, std::string _file)
{
    std::string::size_type counter;
    int in_block = 0;

    for (counter = _start + 1; _file[counter]; counter++)
    {
        if (_file[counter] == '{')
            in_block++;
        if (_file[counter] == '}')
        {
            if (in_block == 0)
                return (counter);
            in_block--;
        }
    }
    throw OutOfServerBlocks();
}


void Conf_parser::extract_server_blocks(std::string _file)
{
    std::string::size_type _start = 0;
    std::string::size_type _end = _start + 1;

    delete_spaces(_file);
    if (_file.find("server") == std::string::npos)
        throw Conf_reader::InvalidFileException();
    while (_start != _end && _start < _file.size())
    {
        _start = begin_serv(_start, _file);
        _end = end_serv(_start, _file);
        block_conf.push_back(_file.substr(_start, _end - _start + 1));
        servers_nbr++;
        _start = _end + 1;
    }
}

void Conf_parser::store_server(std::string _file)
{
    extract_server_blocks(_file);
    if (this->servers_nbr != (int)this->block_conf.size())
        throw ErrorEncountered();
    for (int i = 0; i < this->servers_nbr; i++)
	{
		Server_storage server;
		Server_creator(this->block_conf[i], server);
		this->servers.push_back(server);
	}
	if (this->servers_nbr > 1)
		checkServers();
}

int Conf_parser::getServers_nbr() const
{
    return (this->servers_nbr);
}

std::vector<Server_storage> &Conf_parser::getServers()
{
	return (this->servers);
}

std::vector<std::string> splitWithSeps(std::string line, std::string separetor)
{
	std::vector<std::string> str;
	std::string::size_type _start;
	std::string::size_type _end;

	_start = 0;
	_end = 0;
	while (1)
	{
		_end = line.find_first_of(separetor, _start);
		if (_end == std::string::npos)
			break;
		std::string tmp = line.substr(_start, _end - _start);
		str.push_back(tmp);
		_start = line.find_first_not_of(separetor, _end);
		if (_start == std::string::npos)
			break;
	}
	return (str);
}

// Duplicated servers && required directives //
void Conf_parser::checkServers()
{
	std::vector<Server_storage>::iterator it1;
	std::vector<Server_storage>::iterator it2;

	for (it1 = this->servers.begin(); it1 != this->servers.end() - 1; it1++)
	{
		for (it2 = it1 + 1; it2 != this->servers.end(); it2++)
		{
			if (it1->getPort() == it2->getPort() && it1->getHost() == it2->getHost())// && it1->getServerName() == it2->getServerName())
                throw Server_storage::ExceptionMsg("Duplicated server directive(s)");
		}
	}
}

void Conf_parser::Server_creator(std::string &config, Server_storage &server)
{
	std::vector<std::string> splitedStr;
	std::vector<std::string> error_codes;
	int		flag_loc = 1;
	bool	flag_autoindex = false;
	bool	flag_max_size = false;

	splitedStr = splitWithSeps(config += ' ', std::string(" \n\t"));
	if (splitedStr.size() < 3)
		throw  Server_storage::ExceptionMsg("Invalid server");
	for (size_t i = 0; i < splitedStr.size(); i++)
	{
		if (splitedStr[i] == "listen" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (server.getPort())
				throw  Server_storage::ExceptionMsg("Duplicated port");
			server.setPort(splitedStr[++i]);
		}
		else if (splitedStr[i] == "location" && (i + 1) < splitedStr.size())
		{
			std::string	path;
			i++;
			if (splitedStr[i] == "{" || splitedStr[i] == "}")
				throw  Server_storage::ExceptionMsg("Server: wrong char scope");
			path = splitedStr[i];
			std::vector<std::string> codes;
			if (splitedStr[++i] != "{")
				throw  Server_storage::ExceptionMsg("Server: wrong char scope");
			i++;
			while (i < splitedStr.size() && splitedStr[i] != "}")
				codes.push_back(splitedStr[i++]);
			server.setLocation(path, codes);
			std::vector<Location_storage>::const_iterator v_it = server.getLocations().begin();
			for (; v_it!= server.getLocations().end(); v_it++)
			{
				if (v_it->getLocaUpallow() == true && v_it->getLocaUpdir().empty())
					throw Server_storage::ExceptionMsg("Must specify the upload directory");
				else if (v_it->getLocaUpallow() == false && !v_it->getLocaUpdir().empty())
					throw Server_storage::ExceptionMsg("Must not specify the upload directory");
				if (!v_it->getLocaCgiExtention().empty() && !v_it->getLocaCgipath().empty())
				{
					if ((v_it->getLocaCgiExtention()[0] == ".py" && v_it->getLocaCgipath()[0] != "/usr/bin/python3"
																	&& v_it->getLocaCgipath()[0] != "/bin/python3")
						|| (v_it->getLocaCgiExtention()[0] == ".php" && v_it->getLocaCgipath()[0] != "/usr/bin/php-cgi"
																		&& v_it->getLocaCgipath()[0] != "/bin/php-cgi"))
						throw Server_storage::ExceptionMsg("Invalid order from cgi_extention and cgi_path");
				}
			}
			if (i < splitedStr.size() && splitedStr[i] != "}")
				throw  Server_storage::ExceptionMsg("Server: wrong char scope");
			flag_loc = 0;
		}
		else if (splitedStr[i] == "host" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (server.getHost())
				throw  Server_storage::ExceptionMsg("Duplicated host");
			server.setHost(splitedStr[++i]);
		}
		else if (splitedStr[i] == "root" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (!server.getRoot().empty())
				throw  Server_storage::ExceptionMsg("Duplicated root");
			server.setRoot(splitedStr[++i]);
		}
		else if (splitedStr[i] == "error_page" && (i + 1) < splitedStr.size() && flag_loc)
		{
			while (++i < splitedStr.size())
			{
				error_codes.push_back(splitedStr[i]);
				if (splitedStr[i].find(';') != std::string::npos)
					break ;
				if (i + 1 >= splitedStr.size())
					throw Server_storage::ExceptionMsg("Server: wrong char scope");
			}
		}
		else if (splitedStr[i] == "client_max_body_size" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (flag_max_size)
				throw  Server_storage::ExceptionMsg("Duplicated client max body size");
			server.setClientMaxBodySize(splitedStr[++i]);
			flag_max_size = true;
		}
		else if (splitedStr[i] == "server_name" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (!server.getServerName().empty())
				throw  Server_storage::ExceptionMsg("uplicated server name");
			server.setServerName(splitedStr[++i]);
		}
		else if (splitedStr[i] == "index" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (!server.getIndex().empty())
				throw  Server_storage::ExceptionMsg("uplicated index");
			server.setIndex(splitedStr[++i]);
		}
		else if (splitedStr[i] == "autoindex" && (i + 1) < splitedStr.size() && flag_loc)
		{
			if (flag_autoindex)
				throw Server_storage::ExceptionMsg("Duplicated server autoindex");
			server.setAutoindex(splitedStr[++i]);
			flag_autoindex = true;
		}
		else if (splitedStr[i] != "}" && splitedStr[i] != "{")
		{
			if (!flag_loc)
				throw  Server_storage::ExceptionMsg("Out of location directive");
			else
				throw  Server_storage::ExceptionMsg("Invalid directive");
		}
	}
	if (server.getRoot().empty())
		server.setRoot("/;");
	if (server.getHost() == 0)
		server.setHost("localhost;");
	if (server.getIndex().empty())
		server.setIndex("index.html;");
	if (Conf_reader::permValidFile(server.getRoot(), server.getIndex()))
		throw Server_storage::ExceptionMsg("Invalid configuration file index");
	if (server.isLocaDup())
		throw Server_storage::ExceptionMsg("Duplicated location");
	if (!server.getPort())
		throw Server_storage::ExceptionMsg("No port found");
	server.setErrorPages(error_codes);
	if (!server.isValidErrorPages())
		throw Server_storage::ExceptionMsg("Invalid page/code error");
}