#include "../incs/parse_config/server.hpp"
#include "../incs/cgi_config/cgi_conf.hpp"
#include <csignal>

server::server()
{
    i = 0;

    ExtentionsPost["text/plain"] = ".txt";
    ExtentionsPost["text/html"] = ".html";
    ExtentionsPost["text/css"] = ".css";
    ExtentionsPost["application/javascript"] = ".js";
    ExtentionsPost["application/json"] = ".json";
    ExtentionsPost["application/xml"] = ".xml";
    ExtentionsPost["image/jpg"] = ".jpg";
    ExtentionsPost["image/jpeg"] = ".jpeg";
    ExtentionsPost["image/png"] = ".png";
    ExtentionsPost["image/gif"] = ".gif";
    ExtentionsPost["audio/mpeg"] = ".mp3";
    ExtentionsPost["video/mp4"] = ".mp4";
    ExtentionsPost["application/pdf"] = ".pdf";
    ExtentionsPost["application/msword"] = ".doc";
    ExtentionsPost["application/zip"] = ".zip";
    ExtentionsPost["audio/mpeg"] = ".mp3";
    ExtentionsPost["application/x-httpd-php"] = ".php";
    ExtentionsPost["application/x-sh"] = ".sh";
    ExtentionsPost["application/x-tar"] = ".tar";
    ExtentionsPost["text/xml"] = ".xml";
    ExtentionsPost["application/octet-stream"] = ".x";
}

void server::Store_extention()
{
    Extentions[".txt"] = "text/plain";
    Extentions[".html"] = "text/html";
    Extentions[".css"] = "text/css";
    Extentions[".py"] = "text/python";
    Extentions[".php"] = "text/php";
    Extentions[".js"] = "application/javascript";
    Extentions[".json"] = "application/json";
    Extentions[".xml"] = "application/xml";
    Extentions[".jpg"] = "image/jpeg";
    Extentions[".png"] = "image/png";
    Extentions[".gif"] = "image/gif";
    Extentions[".mp3"] = "audio/mpeg";
    Extentions[".mp4"] = "video/mp4";
    Extentions[".pdf"] = "application/pdf";
    Extentions[".doc"] = "application/msword";
    Extentions[".zip"] = "application/zip";
}

void server::stor_code_statusFiles()
{
    code_statusFiles[403] = "code_status/403.html";
    code_statusFiles[414] = "code_status/414.html";
    code_statusFiles[400] = "code_status/400.html";
    code_statusFiles[404] = "code_status/404.html";
    code_statusFiles[405] = "code_status/405.html";
    code_statusFiles[411] = "code_status/411.html";
    code_statusFiles[413] = "code_status/413.html";
    code_statusFiles[415] = "code_status/415.html";
    code_statusFiles[500] = "code_status/500.html";
    code_statusFiles[501] = "code_status/501.html";
    code_statusFiles[502] = "code_status/502.html";
    code_statusFiles[504] = "code_status/504.html";
}

int Request::checkURL(std::string url)
{
    if (checkRequestLine == 400)
        return 400;
    int i = 0;
    while (url[i])
    {
        if ((url[i] >= 0 && url[i] <= 32) || url[i] == 34 || url[i] == 60 || url[i] == 62 || url[i] == 92 || url[i] == 94 || url[i] == 96 || url[i] == 123 || url[i] == 124 || url[i] == 125 || url[i] == 127)
            return (400);
        i++;
    }
    if (i > 2048)
        return (414);
    return 0;
}

void server::creat_servers()
{
    int soket_fd;
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if ((soket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            throw Filed_creatSocket();
        }
        uint16_t port = it->getPort();
        in_addr_t host = it->getHost();
        serversMap[soket_fd] = std::make_pair(port, host);
    }
}

void server::bindAnd_ListenServers()
{
    signal(SIGPIPE, SIG_IGN);
    int opt = -1;
    int num_evn = 0, client_socket;
    struct epoll_event evns[MAX_EVN];

    if ((epoll_fd = epoll_create(MAX_EVN)) < 0)
        throw Filed_epoll();
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (std::map<int, std::pair<uint16_t, in_addr_t> >::iterator e = serversMap.begin(); e != serversMap.end(); it++, e++)
    {
        setsockopt(e->first, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        it->set_sockadde(it->getPort(), it->getHost());
        struct sockaddr_in ServAdd = it->get_sockaddr();
        if (bind(e->first, (const sockaddr *)&ServAdd, sizeof(ServAdd)) < 0)
            throw Filed_bind();
        if (listen(e->first, 3) == -1)
            throw Filed_listen();
        struct epoll_event evn;
        evn.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
        evn.data.fd = e->first;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e->first, &evn) == -1)
            throw Filed_epoll();
    }
    stor_code_statusFiles();
    Store_extention();
    while (true)
    {
        try
        {
            if ((num_evn = epoll_wait(epoll_fd, evns, MAX_EVN, -1)) < 0)
                throw Filed_epoll();
            for (int i = 0; i < num_evn; i++)
            {
                if (serversMap.find(evns[i].data.fd) != serversMap.end())
                {
                    try
                    {
                        if ((client_socket = accept(evns[i].data.fd, NULL, NULL)) < 0)
                        {
                            throw std::runtime_error("Error : accept conection");
                        }
                        else
                        {
                            std::pair<uint16_t, in_addr_t> &getpair = serversMap[evns[i].data.fd];
                            hand[client_socket].port = getpair.first;
                            hand[client_socket].host = getpair.second;
                            std::vector<Server_storage>::iterator it_serv = serv.getServers().begin();
                            for (; it_serv != serv.getServers().end(); it_serv++)
                            {
                                if (it_serv->getHost() == hand[client_socket].host && it_serv->getPort() == hand[client_socket].port)
                                    hand[client_socket].store_reverse_host = it_serv->_reverse_host;
                            }
                            client_fd.push_back(client_socket);
                            struct epoll_event client_evn;
                            client_evn.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
                            client_evn.data.fd = client_socket;
                            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_evn) < 0)
                            {
                                close(client_socket);
                                throw Filed_epoll();
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        //(void)e;
                        std::cerr << e.what() << std::endl;
                    }
                }
                else
                {
                    std::vector<int>::iterator etr = client_fd.begin();
                    for (; etr != client_fd.end(); etr++)
                    {
                        if (*etr == evns[i].data.fd)
                            break;
                    }
                    if (evns[i].events & EPOLLERR || evns[i].events & EPOLLRDHUP || evns[i].events & EPOLLHUP)
                    {
                        close(hand[*etr].indexFile_fd);
                        if (hand[*etr].check)
                        {
                            kill(hand[*etr]._idcgi, SIGKILL);
                            waitpid(hand[*etr]._idcgi, &hand[*etr].statuscgi, 0);
                            close(hand[*etr].c_in[0]);
                            close(hand[*etr].c_out[1]);
                            close(hand[*etr].c_out[0]);
                        }
                        if (hand[*etr].dir_is_open)
                            closedir(hand[*etr].dir);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, *etr, 0);
                        hand.erase(*etr);
                        close(*etr);
                    }
                    else
                    {
                        hand[*etr].end_time = clock();
                        hand[*etr].Epoll_in = false;
                        if ((evns[i].events & EPOLLIN))
                        {
                            hand[*etr].start_time = clock();
                            if (hand[*etr].readflag)
                                Read_Request(*etr);
                            else if (hand[*etr].method == "POST")
                            {
                                hand[*etr].Epoll_in = true;
                                Request_handling(*etr);
                            }
                        }
                        else if (evns[i].events & EPOLLOUT && hand[*etr].flag)
                        {
                            hand[*etr].start_time = clock();
                            hand[*etr].Epoll_in = false;
                            hand[*etr].Epoll_out = true;
                            Request_handling(*etr);
                        }
                        else if (((hand[*etr].end_time - hand[*etr].start_time) / CLOCKS_PER_SEC) >= 30)
                        {

                            sendError(400, *etr, s.Bad_Request_400, "718");
                            hand[*etr].f = true;
                        }
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            (void)e;
        }
    }
}

void server::Request_handling(int fd)
{
    int check = hand[fd].checkURL(hand[fd].path_Get);
    if (check == 0)
    {
        if (hand[fd].path_Get != "/favicon.ico")
        {
            if (hand[fd].method != "POST" && hand[fd].method != "GET" && hand[fd].method != "DELETE")
            {
                if (hand[fd].method == "HEAD")
                {
                    send(fd, "HTTP/1.1 501 Not Implemented\r\n\r\n", 32, 0);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                    close(fd);
                    hand[fd].readflag = true;
                    hand[fd].flag = false;
                    hand.erase(fd);
                }
                else
                {
                    sendError(501, fd, s.Not_Implemented_501, "726");
                    return;
                }
            }
            else if (hand[fd].method == "GET")
                Get_handling(fd);
            else if (hand[fd].method == "POST")
                Post(fd);
            else if (hand[fd].method == "DELETE")
                Delete_handling(fd);
        }
        else
        {
            sendError(404, fd, s.NotFond_404, "714");
            return;
        }
    }
    else if (check == 400)
    {
        sendError(400, fd, s.Bad_Request_400, "718");
    }
    else
        sendError(414, fd, s.URI_Too_Long_414, "720");
}

std::string server::get_path(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                std::string ptr_pos = GetLocationPath(fd);
                if (ptr_pos == "x")
                    return "x";
                if (ptr_pos == itr->getLocaPath())
                {
                    if (hand[fd].method == "DELETE")
                    {
                        if (itr->getLocaAllowedMethods().at(2) == 0)
                            throw std::runtime_error("not allowed method");
                    }
                    else if (hand[fd].method == "GET")
                    {
                        if (itr->getLocaAllowedMethods().at(0) == 0)
                            throw std::runtime_error("not allowed method");
                    }
                    std::string p = itr->getLocaRoot();
                    if (!p.empty())
                    {

                        p += &hand[fd].path_Get[ptr_pos.length()];
                    }
                    else
                    {
                        p = it->getRoot();
                        p += &hand[fd].path_Get[ptr_pos.length()];
                    }
                    size_t query = p.find("?");
                    if (query == std::string::npos)
                        return (p);
                    else
                    {
                        hand[fd].query_string = p.substr(query + 1);
                        p.erase(query);
                        return (p);
                    }
                }
            }
        }
    }
    return ("");
}

std::string server::GetLocationPath(int fd)
{
    std::string pathHold = &hand[fd].path_Get[1];
    if (pathHold.empty())
        return "/";
    size_t pos = pathHold.find('/');
    if (pos == std::string::npos && hand[fd].path_Get.length() != 1)
    {
        size_t posp = pathHold.find('.');
        if (posp == std::string::npos)
        {
            if (access(("/" + pathHold).c_str(), F_OK))
            {
                DIR *dir = opendir(pathHold.c_str());
                if (dir)
                {

                    hand[fd].pathReturn = hand[fd].path_Get;
                    closedir(dir);
                    if (hand[fd].method == "GET")
                    {
                        return "x";
                    }
                    else
                        return hand[fd].pathReturn;
                }
                else
                    return ("/");
            }
        }
        else if (posp != std::string::npos)
        {
            if (access(hand[fd].path_Get.c_str(), F_OK))
            {
                DIR *dir = opendir(pathHold.c_str());
                if (dir)
                {
                    hand[fd].pathReturn = "/" + pathHold;
                    closedir(dir);
                    if (hand[fd].method == "GET")
                        return "x";
                    else
                        return hand[fd].pathReturn;
                }
                else
                    return ("/");
            }
        }
        else
            return ("/");
    }
    else if (hand[fd].path_Get.length() != 1)
    {
        std::string pathReturn = hand[fd].path_Get.substr(0, pos + 1);
        return pathReturn;
    }
    return ("");
}

bool checkDirIsEmpty(const char *path)
{
    std::string path_delete(path);
    DIR *dir = opendir((path_delete).c_str());
    if (dir)
    {
        struct dirent *centent;
        while ((centent = readdir(dir)) != NULL)
        {
            if (strcmp(centent->d_name, ".") != 0 && strcmp(centent->d_name, "..") != 0)
            {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
        return true;
    }
    return false;
}

void server::RecDelete(const char *path, int fd)
{
    DIR *dir = opendir((path));
    if (dir)
    {
        struct dirent *centent;
        while ((centent = readdir(dir)) != NULL)
        {
            if (strcmp(centent->d_name, ".") != 0 && strcmp(centent->d_name, "..") != 0)
            {
                std::string re(path);
                re += "/";
                re += centent->d_name;
                RecDelete(re.c_str(), fd);
            }
        }
        if (checkDirIsEmpty(path))
            std::remove(path);
    }
    else
    {
        if (access(path, X_OK) == 0)
        {

            std::remove(path);
            // send(fd, s.NoContent_204.c_str(), s.NoContent_204.length(), 0);
        }
        else
            sendError(403, fd, s.Forbidden_403, "714");
    }
    if (dir)
        closedir(dir);
}

void server::Delete_handling(int fd)
{
    try
    {
        if (hand[fd].check_pathForDelete == 0)
        {
            std::string path = get_path(fd);
            if (path.empty())
            {
                if (hand[fd].path_Get.empty())
                {
                    sendError(404, fd, s.NotFond_404, "714");
                    return;
                }
                else
                    path = hand[fd].path_Get;
            }
            if (path == "x")
                path = hand[fd].pathReturn;
            if (path[0] != '.')
                path.insert(0, ".");
            if (access((path).c_str(), F_OK) != 0)
            {
                sendError(404, fd, s.NotFond_404, "714");
            }
            else
            {
                RecDelete(path.c_str(), fd);
                if (access((path).c_str(), F_OK) != 0)
                    send(fd, s.NoContent_204.c_str(), s.NoContent_204.length(), 0);
            }
        }
        else
            sendError(403, fd, s.Forbidden_403, "714");
    }
    catch (const std::exception &e)
    {
        std::string what = e.what();
        if (what == "not allowed method")
        {
            sendError(405, fd, s.Method_NAllowed_405, "732");
        }
        else
            sendError(500, fd, s.Internal_server_Error_500, "738");
    }
}

bool server::Read_Request(int fd)
{
    memset(hand[fd].buff, 0, 1024);
    int f = recv(fd, hand[fd].buff, 1024, 0);
    hand[fd].header.append(hand[fd].buff, f);
    size_t pos = hand[fd].header.find("\r\n\r\n");
    if (pos != std::string::npos)
    {
        hand[fd].readflag = false;
        std::string re = hand[fd].header.substr(0, pos + 2);
        hand[fd].rest = f - pos - 4;
        hand[fd].body = hand[fd].header.substr(pos + 4, hand[fd].rest);
        char ar[re.length() + 1];
        std::strcpy(ar, re.c_str());
        hand[fd].read_request(ar);
        bzero(ar, re.length() + 1);
        re.clear();
        hand[fd].header.clear();
        if (hand[fd].method != "POST")
            hand[fd].flag = true;
        if (hand[fd].request_head.find("Content-Length") != hand[fd].request_head.end())
        {
            if ((long long)hand[fd].rest >= strtoll(hand[fd].request_head.find("Content-Length")->second.c_str(), NULL, 10))
                hand[fd].flag = true;
        }
        return true;
    }
    return (false);
}

/// ////////////////
/// exceptions /
const char *server::Filed_creatSocket::what() const throw()
{
    return ("Error");
}

const char *server::Filed_bind::what() const throw()
{
    return ("Error : bind function");
}

const char *server::Filed_listen::what() const throw()
{
    return ("Error : listen function");
}

const char *server::Filed_epoll::what() const throw()
{
    return ("Error : epoll functions");
}

server::~server()
{
}