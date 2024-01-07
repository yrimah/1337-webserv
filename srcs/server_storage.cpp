#include "../incs/parse_config/server_storage.hpp"
#include "stdlib.h"

Server_storage::Server_storage()
{
	this->_reverse_host = 0;
	this->_port = 0;
	this->_host = 0;
	this->_server_name = "";
	this->_root = "";
	this->_client_max_body_size = 1024; // to change it;
	this->_index = "";
	this->_listen_fd = 0;
	this->_autoindex = false;
	this->initErrorPages();
}

Server_storage::~Server_storage()
{
}

/* copy constructor */
Server_storage::Server_storage(const Server_storage &oServer_storage)
{
	if (this != &oServer_storage)
	{
		this->_server_name = oServer_storage._server_name;
		this->_root = oServer_storage._root;
		this->_host = oServer_storage._host;
		this->_port = oServer_storage._port;
		this->_client_max_body_size = oServer_storage._client_max_body_size;
		this->_index = oServer_storage._index;
		this->_error_pages = oServer_storage._error_pages;
		this->_locations = oServer_storage._locations;
		this->_listen_fd = oServer_storage._listen_fd;
		this->_autoindex = oServer_storage._autoindex;
		this->_server_address = oServer_storage._server_address;
		this->_reverse_host = oServer_storage._reverse_host;

	}
	return ;
}

/* assinment operator */
Server_storage &Server_storage::operator=(const Server_storage & oServer_storage)
{
	if (this != &oServer_storage)
	{
		this->_server_name = oServer_storage._server_name;
		this->_root = oServer_storage._root;
		this->_port = oServer_storage._port;
		this->_host = oServer_storage._host;
		this->_client_max_body_size = oServer_storage._client_max_body_size;
		this->_index = oServer_storage._index;
		this->_error_pages = oServer_storage._error_pages;
		this->_locations = oServer_storage._locations;
		this->_listen_fd = oServer_storage._listen_fd;
		this->_autoindex = oServer_storage._autoindex;
		this->_server_address = oServer_storage._server_address;
		this->_reverse_host = oServer_storage._reverse_host;
	}
	return (*this);
}

/* error page def */
void Server_storage::initErrorPages()
{
	_error_pages[301] = "";
	_error_pages[302] = "";
	_error_pages[400] = "";
	_error_pages[401] = "";
	_error_pages[402] = "";
	_error_pages[403] = "";
	_error_pages[404] = "";
	_error_pages[405] = "";
	_error_pages[406] = "";
	_error_pages[500] = "";
	_error_pages[501] = "";
	_error_pages[502] = "";
	_error_pages[503] = "";
	_error_pages[504] = "";
	_error_pages[505] = "";
}

/* Set functions */
void Server_storage::setServerName(std::string server_name)
{
	removeDelim(server_name);
	this->_server_name = server_name;
}

void Server_storage::setHost(std::string str)
{
	removeDelim(str);
	if (str == "localhost")
		str = "127.0.0.1";
	if (!validateHost(str))
		throw ExceptionMsg("Invalid host");
    std::istringstream k(str);
    std::vector<int> add;
    std::string b;
    while(getline(k, b, '.'))
        add.push_back(atoi(b.c_str()));
    int i = (add[0] << 24) | (add[1] << 16) | (add[2] << 8) | add[3]; 
	this->_reverse_host = i;
	this->_host = htonl(i);
}

void Server_storage::setRoot(std::string root_path)
{
	removeDelim(root_path);
	if (Conf_reader::identifyPath(root_path) == 2)
	{
		this->_root = root_path;
		return ;
	}
	char currentdir[1024];
	getcwd(currentdir, 1024);
	std::string root = currentdir + root_path;
	if (Conf_reader::identifyPath(root) != 2)
		throw ExceptionMsg("Invalid root");
	this->_root = root;
}

void Server_storage::setPort(std::string str)
{
	unsigned int port;
	
	port = 0;
	removeDelim(str);
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			throw ExceptionMsg("Invalid port");
	}
	port = convertStrToInt((str));
	if (port < 1 || port > 65636)
		throw ExceptionMsg("Invalid port");
	this->_port = (uint16_t)port;
}

void Server_storage::setClientMaxBodySize(std::string str)
{
	size_t body_size;
	
	body_size = 0;
	removeDelim(str);
	for (size_t i = 0; i < str.size(); i++)
	{
		if (str[i] < '0' || str[i] > '9')
			throw ExceptionMsg("Invalid client_max_body_size");
	}
	if (!convertStrToInt(str))
		throw ExceptionMsg("Invalid client_max_body_size");
	body_size = convertStrToInt(str);
	this->_client_max_body_size = body_size;
}

void Server_storage::setIndex(std::string index)
{
	removeDelim(index);
	this->_index = index;
}

void Server_storage::setAutoindex(std::string autoindex)
{
	removeDelim(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ExceptionMsg("Invalid autoindex");
	if (autoindex == "on")
		this->_autoindex = true;
}

// set error pages if default, if not add pair if it "code,path"
void Server_storage::setErrorPages(std::vector<std::string> &str)
{
	if (str.empty())
		return;
	if (str.size() % 2 != 0)
		throw ExceptionMsg ("Error initializer failed");
	for (size_t i = 0; i < str.size() - 1; i++)
	{
		for (size_t j = 0; j < str[i].size(); j++) {
			if (!std::isdigit(str[i][j]))
				throw ExceptionMsg("Invalid error code");
		}
		if (str[i].size() != 3)
			throw ExceptionMsg("Error code is invalid");
		int code_error = convertStrToInt(str[i]);
		// re test it -----------//
		if (status(code_error)  == "Undefined" || code_error < 400)
			throw ExceptionMsg ("Incorrect error code: " + str[i]);
		// ----------------------//
		i++;
		std::string path = str[i];
		removeDelim(path);
		if (Conf_reader::identifyPath(path) != 2)
		{
			if (Conf_reader::identifyPath(this->_root + path) != 1)
				throw ExceptionMsg ("Invalid error page path: " + this->_root + path);
			if (Conf_reader::ValidateFile(this->_root + path, 0) == -1 || Conf_reader::ValidateFile(this->_root + path, 4) == -1)
				throw ExceptionMsg ("Invalid error page " + this->_root + path);
		}
		std::map<int, std::string>::iterator it = this->_error_pages.find(code_error);
		if (it != _error_pages.end())
			this->_error_pages[code_error] = path;
		else
			this->_error_pages.insert(std::make_pair(code_error, path));
	}
}

// config locations
void Server_storage::setLocation(std::string path, std::vector<std::string> vec)
{
	Location_storage new_location;
	std::vector<std::string> methods;
	bool flag_methods = false;
	bool flag_autoindex = false;
	bool up = false;
	int valid;

	new_location.setLocaPath(path);
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i] == "root" && (i + 1) < vec.size())
		{
			if (!new_location.getLocaRoot().empty())
				throw ExceptionMsg("Duplicated location root");
			removeDelim(vec[++i]);
			if (Conf_reader::identifyPath(vec[i]) == 2)
				new_location.setLocaRoot(vec[i]);
			else
			{
				std::string tmp = this->_root;
				tmp.erase(tmp.length() - 1);
				new_location.setLocaRoot(tmp + vec[i]);
			}
		}
		else if ((vec[i] == "allow_methods") && (i + 1) < vec.size())
		{
			if (flag_methods)
				throw ExceptionMsg("Duplicated location allowed method");
			std::vector<std::string> methods;
			while (++i < vec.size())
			{
				if (vec[i].find(";") != std::string::npos)
				{
					removeDelim(vec[i]);
					methods.push_back(vec[i]);
					break ;
				}
				else
				{
					methods.push_back(vec[i]);
					if (i + 1 >= vec.size())
						throw ExceptionMsg("Invalid delim");
				}
			}
			new_location.setLocaAllowedMethods(methods);
			flag_methods = true;
		}
		else if (vec[i] == "up_allow" && (i + 1) < vec.size())
		{
			if (up)
				throw ExceptionMsg("Duplicated up_allow");
			removeDelim(vec[++i]);
			new_location.setLocaUpallow(vec[i]);
			up = true;
		}
		else if (vec[i] == "autoindex" && (i + 1) < vec.size())
		{
			if (path == "/cgi-bin")
				throw ExceptionMsg("Autoindex not allow for CGI");
			if (flag_autoindex)
				throw ExceptionMsg("Duplicated location autoindex");
			removeDelim(vec[++i]);
			new_location.setLocaAutoindex(vec[i]);
			flag_autoindex = true;
		}
		else if (vec[i] == "index" && (i + 1) < vec.size())
		{
			if (!new_location.getLocaIndex().empty())
				throw ExceptionMsg("Duplicated location index");
			removeDelim(vec[++i]);
			new_location.setLocaIndex(vec[i]);
		}
		else if (vec[i] == "up_dir" && (i + 1) < vec.size())
		{
			if (!new_location.getLocaUpdir().empty())
				throw ExceptionMsg("Duplicated upload location");
			removeDelim(vec[++i]);
			new_location.setLocaUPdir(vec[i]);
		}
		else if (vec[i] == "return" && (i + 1) < vec.size())
		{
			if (path == "/cgi-bin")
				throw ExceptionMsg("Return not allowed for CGI");
			if (!new_location.getLocaRedirect().empty())
				throw ExceptionMsg("Duplicated location return");
			removeDelim(vec[++i]);
			new_location.setLocaRedirect(vec[i]);
		}
		else if (vec[i] == "alias" && (i + 1) < vec.size())
		{
			if (path == "/cgi-bin")
				throw ExceptionMsg("Alias not allowed for CGI");
			if (!new_location.getLocaAlias().empty())
				throw ExceptionMsg("Duplicated location alias");
			removeDelim(vec[++i]);
			new_location.setLocaAlias(vec[i]);
		}
		else if (vec[i] == "cgi_extention" && (i + 1) < vec.size())
		{
			std::vector<std::string> cgi_ext;
			while (++i < vec.size())
			{
				if (vec[i].find(";") != std::string::npos)
				{
					removeDelim(vec[i]);
					cgi_ext.push_back(vec[i]);
					break ;
				}
				else
				{
					cgi_ext.push_back(vec[i]);
					if (i + 1 >= vec.size())
						throw ExceptionMsg("Invalid delim");
				}
				if ((vec[i] != ".py" && vec[i + 1] != ".py;")
					|| (vec[i] != ".php" && vec[i + 1] != ".php;"))
					throw ExceptionMsg("Invalid cgi path");
			}
			new_location.setLocaCgiExtention(cgi_ext);
		}
		else if (vec[i] == "cgi_path" && (i + 1) < vec.size())
		{
			std::vector<std::string> path;
			while (++i < vec.size())
			{
				if (vec[i].find(";") != std::string::npos)
				{
					removeDelim(vec[i]);
					if (Conf_reader::ValidateFile(vec[i], 1))
						throw ExceptionMsg("Invalid cgi path");
					path.push_back(vec[i]);
					break ;
				}
				else
				{
					if (Conf_reader::ValidateFile(vec[i], 1))
						throw ExceptionMsg("Invalid cgi path");
					path.push_back(vec[i]);
					if (i + 1 >= vec.size())
						throw ExceptionMsg("Invalid delim");
				}
				if ((vec[i].find("/php-cgi") == std::string::npos && vec[i].find("/python3") == std::string::npos) 
					|| (vec[i + 1].find("/php-cgi") == std::string::npos && vec[i + 1].find("/python3") == std::string::npos))
					throw ExceptionMsg("Invalid cgi path");
			}
			new_location.setLocaCgipath(path);
		}
		else if (i < vec.size())
			throw ExceptionMsg("Invalid location directive");
	}
	valid = isValidLocation(new_location);
	if (valid == 1)
		throw ExceptionMsg("Invalid cgi");
	else if (valid == 2)
		throw ExceptionMsg("Invalid location path");
	else if (valid == 3)
		throw ExceptionMsg("Invalid location redirect file");
	else if (valid == 4)
		throw ExceptionMsg("Invalid location alias");
	this->_locations.push_back(new_location);
}

void	Server_storage::setFd(int fd)
{
	this->_listen_fd = fd;
}

/*directives validator */
bool Server_storage::validateHost(std::string host) const
{
    if (host[host.length() - 1] == '.')
        return (false);
    std::istringstream _hold(host);
    std::string split;
    int octets[4] = {0};
    int count = 0;

    while (std::getline(_hold, split, '.'))
    {
        if (count >= 4)
            return (false);

        int value = atoi(split.c_str());
        if (value < 0 || value > 255
            || (split.find_first_not_of("0123456789") != std::string::npos))
            return (false);
        octets[count++] = value;
    }
    return (count == 4);
}

bool Server_storage::isValidErrorPages()
{
	std::map<int, std::string>::const_iterator it;
	for (it = this->_error_pages.begin(); it != this->_error_pages.end(); it++)
	{
		if (it->first < 100 || it->first > 599)
			return (false);
		if (Conf_reader::ValidateFile(getRoot() + it->second, 0) < 0 || Conf_reader::ValidateFile(getRoot() + it->second, 4) < 0)
			return (false);
	}
	return (true);
}

/* location validator */
int Server_storage::isValidLocation(Location_storage &location) const
{
	if (location.getLocaPath() == "/cgi-bin")
	{
		std::vector<std::string> tmp_path = location.getLocaCgipath();
		std::vector<std::string> tmp_ext = location.getLocaCgiExtention();
		if (tmp_path.size() < 2 || tmp_ext.size() < 2)
			return (1);
		if (location.getLocaCgipath().empty() || location.getLocaCgiExtention().empty() || location.getLocaIndex().empty())
			return (1);
		
		if (Conf_reader::ValidateFile(location.getLocaIndex(), 4) < 0)
		{
			std::string path = location.getLocaRoot() + location.getLocaPath() + "/" + location.getLocaIndex();
			if (Conf_reader::identifyPath(path) != 1)
			{	
				char * root = getcwd(NULL, 0);
				// location.setLocaRoot(root);
				path = root + location.getLocaPath() + "/" + location.getLocaIndex();
				free (root);
			}
			if (path.empty() || Conf_reader::identifyPath(path) != 1)// || Conf_reader::ValidateFile(path, 4) < 0)
				return (1);
			
		}
	}
	else
	{
		if (location.getLocaPath()[0] != '/')
			return (2);
		// if (location.getLocaRoot().empty())
		// 	location.setLocaRoot(this->_root);
		if (Conf_reader::permValidFile(location.getLocaRoot() + location.getLocaPath() + "/", location.getLocaIndex()))
			return (5);
		if (!location.getLocaRedirect().empty())
		{
			if (Conf_reader::permValidFile(location.getLocaRoot(), location.getLocaRedirect()))
				return (3);
		}
		if (!location.getLocaAlias().empty())
		{
			if (Conf_reader::permValidFile(location.getLocaRoot(), location.getLocaAlias()))
			 	return (4);
		}
	}
	return (0);
}

/* Get functions */
const std::string &Server_storage::getServerName()
{
	return (this->_server_name);
}

const std::string &Server_storage::getRoot()
{
	return (this->_root);
}

const bool &Server_storage::getAutoindex()
{
	return (this->_autoindex);
}

const in_addr_t &Server_storage::getHost()
{
	return (this->_host);
}

const uint16_t &Server_storage::getPort()
{
	return (this->_port);
}

const size_t &Server_storage::getClientMaxBodySize()
{
	return (this->_client_max_body_size);
}

const std::vector<Location_storage> &Server_storage::getLocations()
{
	return (this->_locations);
}

const std::map<int, std::string> &Server_storage::getErrorPages()
{
	return (this->_error_pages);
}

const std::string &Server_storage::getIndex()
{
	return (this->_index);
}

int Server_storage::getFd() 
{ 
	return (this->_listen_fd); 
}

/* delim ; */
void Server_storage::removeDelim(std::string &str)
{
	size_t pos = str.rfind(';');
	if (pos != str.size() - 1)
		throw ExceptionMsg("Invalid delim");
	str.erase(pos);
}

/* duplicate location */
bool Server_storage::isLocaDup() const
{
	if (this->_locations.size() < 2)
		return (false);
	std::vector<Location_storage>::const_iterator it1;
	std::vector<Location_storage>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getLocaPath() == it2->getLocaPath())
				return (true);
		}
	}
	return (false);
}



/////foresrvers//////

void Server_storage::set_sockadde(uint16_t Port, in_addr_t Host)
{
	_server_address.sin_family= AF_INET;
    _server_address.sin_port = htons(Port);
    _server_address.sin_addr.s_addr = Host;
}

struct sockaddr_in Server_storage::get_sockaddr() const
{
	return _server_address;
}