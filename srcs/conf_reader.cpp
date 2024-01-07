#include "../incs/parse_config/conf_reader.hpp"

Conf_reader::Conf_reader()
{
	path = "";
}

Conf_reader::Conf_reader(std::string path)
{
    this->path = path;
}

Conf_reader::~Conf_reader()
{
}

bool Conf_reader::validate_file() const
{
    std::fstream _conf;

    _conf.open(this->path.c_str(), std::ios::in);
    if (!_conf.is_open())
        return (false);
    _conf.close();
    return (true);
}

std::string Conf_reader::read_conf() const
{
    std::ifstream _conf;
    std::stringstream _strConf;

    _conf.open(this->path.c_str(), std::ios::in);
    _strConf << _conf.rdbuf();
    _conf.close();
    return (_strConf.str());
}

// found file && access to it //
int	Conf_reader::ValidateFile(std::string const path, int mode)
{
	return (access(path.c_str(), mode));
}

int Conf_reader::permValidFile(std::string const path, std::string const index)
{
	if (identifyPath(index) == 1 && ValidateFile(index, 4) == 0)
		return (0);
	if (identifyPath(path + index) == 1 && ValidateFile(path + index, 4) == 0)
		return (0);
	return (-1);
}

int Conf_reader::identifyPath(std::string const path)
{
	struct stat	buffer;
	int			result;
	
	result = stat(path.c_str(), &buffer);
	if (result == 0)
	{
		if (buffer.st_mode & S_IFREG)
			return (1);
		else if (buffer.st_mode & S_IFDIR)
			return (2);
		else
			return (3);
	}
	else
		return (-1);
}

std::string Conf_reader::getPath() const
{
    return (this->path);
}