#include "../../incs/cgi_config/cgi_conf.hpp"
#include "../../incs/parse_config/server.hpp"
#include <ctime>

Cgi_conf::Cgi_conf()
{
    this->_env = NULL;
    this->_argv = NULL;
    this->status = 0;
    this->path = "";
    this->cgi_id = -1;
}

Cgi_conf::~Cgi_conf()
{
    if (this->_env)
    {
        for (int i = 0; this->_env[i]; i++)
            free(this->_env[i]);
        free(this->_env);
    }

    if (this->_argv)
    {
        for (int i = 0; this->_argv[i]; i++)
            free(this->_argv[i]);
        free(this->_argv);
    }
    this->cgi_env.clear();
}

static std::string ft_to_string(size_t value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void Cgi_conf::initialize_cgi_env(Request requestObj, Loca_it _it)
{
    if (Conf_reader::ValidateFile(this->path, 4) < 0)
    {
        status = 403;
        return;
    }

    std::map<std::string, std::string> cgi_execs;

    cgi_execs[_it->getLocaCgiExtention().at(0)] = _it->getLocaCgipath().at(0);
    cgi_execs[_it->getLocaCgiExtention().at(1)] = _it->getLocaCgipath().at(1);

    if (this->path.find(".") == std::string::npos)
    {
        status = 501;
        return;
    }

    std::string ext = this->path.substr(this->path.find("."));

    std::string cgi_exec = "";

    for (map_it it = cgi_execs.begin(); it != cgi_execs.end(); it++)
    {
        if (ext == it->first)
            cgi_exec = it->second;
    }
    if (cgi_exec.empty())
    {
        status = 501;
        return;
    }
    this->cgi_env["CONTENT_LENGTH"] = requestObj.request_head["Content-Length"];
    this->cgi_env["MAX_BODY"] = ft_to_string(requestObj._client_max_body_size);
    this->cgi_env["CONTENT_TYPE"] = requestObj.request_head["Content-Type"];
    this->cgi_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
    int i = requestObj.store_reverse_host;
    std::stringstream k;
    int chift = 24;
    for (int j = 0; j < 4; j++)
    {
        int resume = (i >> chift) & 0xFF;
        if (j < 3)
            k << resume << ".";
        else
            k << resume;
        chift -= 8;
    }
    this->cgi_env["REMOTE_ADDR"] = k.str();
    this->cgi_env["REMOTE_HOST"] = k.str();
    this->cgi_env["REQUEST_METHOD"] = requestObj.method;
    this->cgi_env["HTTP_COOKIE"] = requestObj.request_head["Cookie"];
    this->cgi_env["SCRIPT_FILENAME"] = this->path;
    this->cgi_env["SCRIPT_NAME"] = this->path;
    this->cgi_env["QUERY_STRING"] = requestObj.query_string;
    this->cgi_env["REDIRECT_STATUS"] = "200";
    this->cgi_env["SERVER_PORT"] = ft_to_string(requestObj.port);
    this->cgi_env["SERVER_PROTOCOL"] = std::string("HTTP/1.1");
    this->cgi_env["DOCUMENT_ROOT"] = std::string("./");
    this->cgi_env["AUTH_TYPE"] = std::string("Basic");
    if (this->cgi_env["REQUEST_METHOD"] == "POST")
    {
        std::string for_ext = requestObj.path;
        size_t up_ext = for_ext.find_last_of(".");
        for_ext = for_ext.substr(up_ext);
        this->cgi_env["FILE_EXT"] = for_ext;
        this->cgi_env["UP_FILE"] = requestObj.path;
    }
    this->_argv = (char **)malloc(sizeof(char *) * 3);
    this->_env = (char **)malloc(sizeof(char *) * (this->cgi_env.size() + 1));

    this->_argv[2] = NULL;
    this->_argv[1] = strdup((char *)this->path.c_str());
    this->_argv[0] = strdup((char *)cgi_exec.c_str());

    map_it it = this->cgi_env.begin();
    int j = 0;
    for (int i = 0; it != cgi_env.end(); it++, i++)
    {
        std::string value = it->first + "=" + it->second;
        this->_env[i] = strdup(value.c_str());
        j++;
    }
    this->_env[j] = NULL;
}

void Cgi_conf::setPath(std::string path)
{
    this->path = path;
}

int &Cgi_conf::getStatus(void)
{
    return (this->status);
}

static ssize_t _write(int fd, const char *buf, size_t _data_size)
{
    size_t _total_written = 0;
    while (_total_written < _data_size)
    {
        ssize_t _it_written = write(fd, buf + _total_written, _data_size - _total_written);
        _total_written += _it_written;
    }
    return (_total_written);
}

void Cgi_conf::cgi_executer(Request &oRequest, int &error)
{
    if (status == 501)
    {
        oRequest._cgiwait = false;
        error = status;
        return;
    }
    if (status == 403)
    {
        oRequest._cgiwait = false;
        error = status;
        return;
    }
    if (!this->_argv[0] || !this->_argv[1])
    {
        oRequest._cgiwait = false;
        error = 500;
        return;
    }
    if (pipe(_in) < 0)
    {
        oRequest._cgiwait = false;
        error = 500;
        return;
    }
    if (pipe(_out) < 0)
    {
        oRequest._cgiwait = false;
        close(_in[0]);
        close(_in[1]);
        error = 500;
        return;
    }
    oRequest.cgistart = clock();
    this->cgi_id = fork();
    oRequest.c_in[0] = _in[0];
    oRequest.c_in[1] = _in[1];
    oRequest.c_out[1] = _out[1];
    oRequest.c_out[0] = _out[0];
    oRequest._idcgi = this->cgi_id;
    if (this->cgi_id < 0)
    {
        oRequest._cgiwait = false;
        error = 500;
        close(_in[0]);
        close(_in[1]);
        close(_out[0]);
        close(_out[1]);
    }
    else if (this->cgi_id == 0)
    {
        dup2(_in[0], STDIN_FILENO);
        dup2(_out[1], STDOUT_FILENO);
        close(_in[0]);
        close(_in[1]);
        close(_out[0]);
        close(_out[1]);
        if (!oRequest._cgiwait)
        {
            if ((status = execve(this->_argv[0], this->_argv, this->_env) == -1))
            {
            }
            exit(status);
        }
    }
    else if (this->cgi_id > 0)
    {
        if (this->cgi_env["REQUEST_METHOD"] == "POST")
        {
            _write(_in[1], final.c_str(), final.length());
            close(_in[0]);
            close(_out[1]);
            waitpid(this->cgi_id, &status, WNOHANG);
            if (WIFEXITED(status))
                waitpid(this->cgi_id, &status, WNOHANG);
        }
        close(_in[1]);

        return;
    }
}

void Cgi_conf::clear()
{
    this->cgi_id = -1;
    this->status = 0;
    this->path = "";
    this->_env = NULL;
    this->_argv = NULL;
    this->cgi_env.clear();
}