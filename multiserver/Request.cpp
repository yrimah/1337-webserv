#include "../incs/parse_config/Request.hpp"
#include <cstring>

Request::Request()
{
    S_sizeOfFile = 0;
    size_fend = 0;
    Head_send = false;
    indexFile_open = false;
    send_filed = true;
    read_filed = true;
    readflag = true;
    flag = false;
    read_true = true;
    dir_is_open = false;
    redirection_Location = true;
    readByte = 0;
    sendByte = 0;
    dir = NULL;
    // post
    _read = 0;
    readsize = 1024;
    numR = 0;
    length = 0;
    f = true;
    lengthBoundary = 0;
    B = false;
    getFail = true;
    bodylength = 0;
    check = false;
    lastboundary = true;
    start_time = clock();
    end_time = clock();
    //
    query_string = "";
    cgi_status = true;
    cgi_error = false;
    cgi_head = true;
    cgi_done = false;
    _cgiwait = false;
    store_reverse_host = 0;
    _idcgi = -2;
    //
    _client_max_body_size = 0;
    max_body = 0;
    check_pathForDelete = 0;

    checkRequestLine = 0;
}

Request::~Request()
{
}

void Request::ft_checkRequestLine(std::string check)
{
    int i = 0;
    int space = 0;
    while (check[i])
    {
        if(check[i] == ' ')
            space++;
        i++;
    }
    if(space != 2)
        checkRequestLine = 400;
}

void Request::check_path_forDelete(std::string path_forDelete)
{
    char resolvedPath[PATH_MAX];
    char resolvedPath_root[PATH_MAX];
    std::string path = "." + path_forDelete;
    realpath(path.c_str(), resolvedPath);
    realpath("./", resolvedPath_root);
    if (std::strncmp(resolvedPath, resolvedPath_root, strlen(resolvedPath_root))!= 0)
        check_pathForDelete = 1;
}

void Request::read_request(char b[])
{
    std::istringstream s(b);
    std::string read;
    if (method.empty())
    {
        std::getline(s, read, '\n');
        std::istringstream r(read);
        ft_checkRequestLine(read);
        std::getline(r, method, ' ');
        std::getline(r, path_Get, ' ');
        std::getline(r, http_version, '\r');
        if(path_Get[0] != '/' || http_version != "HTTP/1.1")
            checkRequestLine = 400;
        check_path_forDelete(path_Get);
    }
    while (std::getline(s, read, '\n'))
    {
        size_t i = read.find(':');
        if (i != std::string::npos)
        {
            key = read.substr(0, i);
            value = read.substr(i + 2, read.length() - i);
        }
        else
        {
            value += read;
        }
        value += "\n";
        request_head[key] = value;
    }
}

uint16_t Request::Get_port()
{
    return (this->port);
}

in_addr_t Request::Get_host()
{
    return(this->host);
}