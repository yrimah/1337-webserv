#include "../incs/parse_config/location_storage.hpp"

Location_storage::Location_storage()
{
	init_location();
}

Location_storage::~Location_storage()
{
}

void Location_storage::init_location()
{
    loca_path = "";
	loca_root = "";
	loca_autoindex = false;
	loca_upallow = false;
	loca_updir = "";
	loca_index = "";
	loca_redirect = "";
	loca_alias = "";
	loca_allowed_methods.push_back(0);
	loca_allowed_methods.push_back(0);
	loca_allowed_methods.push_back(0);
}

// setters //
void Location_storage::setLocaPath(std::string loca_path)
{
    this->loca_path = loca_path;
}

void Location_storage::setLocaRoot(std::string loca_root)
{
	if (Conf_reader::identifyPath(loca_root) != 2)
		throw InvalidRootDir();
	this->loca_root = loca_root;
}

void Location_storage::setLocaAllowedMethods(storage allowed_methods)
{
	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (allowed_methods[i] != "GET"
			&& allowed_methods[i] != "POST"
			&& allowed_methods[i] != "DELETE")
			throw InvalidMethodException();
	}
	for (size_t i = 0; i < allowed_methods.size(); i++)
	{
		if (allowed_methods[i] == "GET")
			this->loca_allowed_methods[0] = 1;
		if (allowed_methods[i] == "POST")
			this->loca_allowed_methods[1] = 1;
		if (allowed_methods[i] == "DELETE")
			this->loca_allowed_methods[2] = 1;
	}
}

void Location_storage::setLocaAutoindex(std::string loca_autoindex)
{
    if (loca_autoindex != "on" && loca_autoindex != "off")
        throw InvalidAutoindex();
    this->loca_autoindex = (loca_autoindex == "on") ? true : false; 
}

void Location_storage::setLocaUpallow(std::string loca_upallow)
{
    if (loca_upallow != "on" && loca_upallow != "off")
        throw InvalidUpallow();
    this->loca_upallow = (loca_upallow == "on") ? true : false; 
}

void Location_storage::setLocaUPdir(std::string loca_updir)
{
	if (Conf_reader::identifyPath(loca_updir) != 2)
		throw InvalidUpDir();
	this->loca_updir = loca_updir;
}

void Location_storage::setLocaIndex(std::string loca_index)
{
	this->loca_index = loca_index;
}

void Location_storage::setLocaAlias(std::string loca_alias)
{
	this->loca_alias = loca_alias;
}

void Location_storage::setLocaCgipath(storage loca_cgi_path)
{
	this->loca_cgi_path = loca_cgi_path;
}

void Location_storage::setLocaCgiExtention(storage loca_cgi_extention)
{
	this->loca_cgi_extension = loca_cgi_extention;
}

void Location_storage::setLocaRedirect(std::string loca_redirect)
{
	this->loca_redirect = loca_redirect;
}

bool Location_storage::check_maxsize(std::string maxsize)
{
    for (size_t i = 0; i < maxsize.size(); i++)
    {
        if (maxsize[i] > '9' || maxsize[i] < '0')
            throw InvalidMaxSize();
    }
    return (true);
}

// getters //
const std::string &Location_storage::getLocaPath() const
{
	return (this->loca_path);
}

const std::string &Location_storage::getLocaRoot() const
{
	return (this->loca_root);
}

const storage_int &Location_storage::getLocaAllowedMethods() const
{
	return (this->loca_allowed_methods);
}

const bool &Location_storage::getLocaAutoindex() const
{
	return (this->loca_autoindex);
}

const bool &Location_storage::getLocaUpallow() const
{
	return (this->loca_upallow);
}

const std::string &Location_storage::getLocaUpdir() const
{
	return (this->loca_updir);
}

const std::string &Location_storage::getLocaIndex() const
{
	return (this->loca_index);
}

const std::string &Location_storage::getLocaAlias() const
{
	return (this->loca_alias);
}

const storage &Location_storage::getLocaCgipath() const
{
	return (this->loca_cgi_path);
}

const storage &Location_storage::getLocaCgiExtention() const
{
	return (this->loca_cgi_extension);
}

const std::string &Location_storage::getLocaRedirect() const
{
	return (this->loca_redirect);
}