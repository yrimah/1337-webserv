#include "../incs/parse_config/server.hpp"
#include "../incs/cgi_config/cgi_conf.hpp"
#include <csignal>

void server::Get_handling(int fd)
{
    try
    {
        std::string path = get_path(fd);
        if (path == "x")
            handel_redirection_path(fd);
        else
        {
            if (access(path.c_str(), F_OK) != 0)
            {
                sendError(404, fd, s.NotFond_404, "714");
                return;
            }
            else
            {
                if (path.empty())
                {
                    sendError(404, fd, s.NotFond_404, "714");
                    return;
                }
                else
                    Get_F(path.c_str(), fd);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::string what = e.what();
        if (what == "not allowed method")
            sendError(405, fd, s.Method_NAllowed_405, "732");
        else
            sendError(500, fd, s.Internal_server_Error_500, "738");
    }
}

const std::string server::getPathErrorPage(int code, int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            std::map<int, std::string>::const_iterator itr = it->getErrorPages().begin();
            for (; itr != it->getErrorPages().end(); itr++)
            {
                if (itr->first == code)
                {
                    return (itr->second);
                }
            }
        }
    }
    return ("");
}

bool server::OpenLocIDXFile_forSend(const char *path, int fd)
{
    std::string pathIndex(path);
    pathIndex += "/";
    pathIndex += hand[fd].Index_file;
    int fd_index;
    if (hand[fd].indexFile_open == false)
    {
        fd_index = open(pathIndex.c_str(), O_RDONLY);
        if (fd_index < 0)
        {
            sendError(404, fd, s.NotFond_404, "714");
            return false;
        }
        struct stat FileInfo;
        fstat(fd_index, &FileInfo);
        hand[fd].S_sizeOfFile = FileInfo.st_size;
        std::ostringstream s;
        s << FileInfo.st_size;
        hand[fd].Size_ofFile = s.str();
        hand[fd].indexFile_fd = fd_index;
        hand[fd].indexFile_open = true;
    }
    return true;
}

void server::OpenServIDXFile_forSend(int fd)
{
    std::string pathIndex;
    int fd_index;
    if (hand[fd].indexFile_open == false)
    {
        std::vector<Server_storage>::iterator it = serv.getServers().begin();
        for (; it != serv.getServers().end(); it++)
        {
            if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
            {
                pathIndex = it->getIndex();
            }
        }
        fd_index = open(pathIndex.c_str(), O_RDONLY);
        struct stat FileInfo;
        fstat(fd_index, &FileInfo);
        hand[fd].S_sizeOfFile = FileInfo.st_size;
        std::ostringstream s;
        s << FileInfo.st_size;
        hand[fd].Size_ofFile = s.str();
        hand[fd].indexFile_fd = fd_index;
        hand[fd].indexFile_open = true;
    }
}

void server::OpenFile_forSend(const char *path, int fd)
{
    int fd_index;
    if (hand[fd].indexFile_open == false)
    {
        fd_index = open(path, O_RDONLY);
        struct stat FileInfo;
        fstat(fd_index, &FileInfo);
        hand[fd].S_sizeOfFile = FileInfo.st_size;
        std::ostringstream s;
        s << FileInfo.st_size;
        hand[fd].Size_ofFile = s.str();
        hand[fd].indexFile_fd = fd_index;
        hand[fd].indexFile_open = true;
    }
}
//
void server::OpenErrorFIle_forSend(const char *path, int fd)
{
    int fd_error;
    if (hand[fd].indexFile_open == false)
    {
        fd_error = open(path, O_RDONLY);
        struct stat FileInfo;
        fstat(fd_error, &FileInfo);
        hand[fd].S_sizeOfFile = FileInfo.st_size;
        std::ostringstream s;
        s << FileInfo.st_size;
        hand[fd].Size_ofFile = s.str();
        hand[fd].indexFile_fd = fd_error;
        hand[fd].indexFile_open = true;
    }
}
//

void server::GetCgiFalse(const char *path, int fd)
{
    if (OpenLocIDXFile_forSend(path, fd))
    {
        if (!hand[fd].Head_send)
        {
            std::string ext = convert_file_extention(hand[fd].Index_file.c_str());
            s.OK_200 += "Content-Type: ";
            s.OK_200 += ext;
            s.OK_200 += "\r\nContent-Length: ";
            s.OK_200 += hand[fd].Size_ofFile;
            s.OK_200 += "\r\n\r\n";
            ssize_t sendH = send(fd, s.OK_200.c_str(), s.OK_200.length(), 0);
            sendHandel(fd, s.OK_200.length(), sendH);
            s.OK_200.clear();
            s.OK_200 = "HTTP/1.1 200 ok\r\n";
            hand[fd].Head_send = true;
        }
        else
            ReadAnd_SendContent(fd);
    }
}

void server::sendHandel(int fd, int read, int send)
{
    if (send < 1)
    {
        hand[fd].send_filed = true;
    }
    else if (send == read)
    {
        hand[fd].size_fend += send;
        hand[fd].send_filed = true;
        hand[fd].read_filed = true;
    }
    else if (send < read)
    {
        hand[fd].size_fend += send;
        read -= send;
        hand[fd].send_filed = false;
    }
}

void server::ReadAnd_SendContent(int fd)
{
    if (hand[fd].send_filed)
    {
        if (hand[fd].read_filed)
        {
            hand[fd].readByte = read(hand[fd].indexFile_fd, hand[fd].BufReadIndex, sizeof(hand[fd].BufReadIndex));
            if (hand[fd].readByte < 1)
            {
                hand[fd].send_filed = true;
                hand[fd].read_true = false;
            }
            hand[fd].read_filed = false;
        }
        else
        {
            if (hand[fd].readByte < 1024 && hand[fd].read_true)
            {
                std::string subBufF(hand[fd].BufReadIndex, hand[fd].BufReadIndex + hand[fd].readByte);
                if (!hand[fd].cgi_head && !hand[fd].cgi_error)
                {
                    std::string tmp = "HTTP/1.1 200 OK\r\n";
                    tmp += subBufF;
                    subBufF = tmp;
                    hand[fd].cgi_head = true;
                }
                hand[fd].cgi_error = false;
                subBufF += "\r\n\r\n";
                hand[fd].sendByte = send(fd, subBufF.c_str(), subBufF.length(), 0);
                sendHandel(fd, subBufF.length(), hand[fd].sendByte);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                close(hand[fd].indexFile_fd);
                close(fd);
                hand[fd].readflag = true;
                hand[fd].cgi_status = true;
                hand[fd].cgi_done = true;
                hand[fd].flag = false;
                hand[fd]._cgiwait = false;
                if (hand[fd].dir_is_open)
                {
                    if (closedir(hand[fd].dir))
                        hand[fd].dir_is_open = false;
                }
                hand.erase(fd);
            }
            else if (hand[fd].read_true)
            {
                if (!hand[fd].cgi_status && !hand[fd].cgi_head)
                {
                    std::string ctmp = "HTTP/1.1 200 OK\r\n";
                    hand[fd].cgi_head = true;
                    hand[fd].sendByte = send(fd, ctmp.c_str(), ctmp.size(), 0);
                }
                hand[fd].sendByte = send(fd, hand[fd].BufReadIndex, hand[fd].readByte, 0);
                sendHandel(fd, hand[fd].readByte, hand[fd].sendByte);
            }
        }
    }
    else
    {
        hand[fd].sendByte = send(fd, hand[fd].BufReadIndex, hand[fd].readByte, 0);
        sendHandel(fd, hand[fd].readByte, hand[fd].sendByte);
    }
}
//

void server::ReadAnd_SendContentER(int fd, std::string code)
{
    if (hand[fd].send_filed)
    {
        hand[fd].readByte = read(hand[fd].indexFile_fd, hand[fd].BufReadIndex, sizeof(hand[fd].BufReadIndex));
        code.append(hand[fd].BufReadIndex, hand[fd].readByte);
        if (hand[fd].readByte < 1)
        {
            hand[fd].send_filed = true;
            hand[fd].read_true = false;
        }
        if (hand[fd].readByte < 1024 && hand[fd].read_true)
        {
            hand[fd].sendByte = send(fd, code.c_str(), code.length(), 0);
            sendHandel(fd, code.length(), hand[fd].sendByte);
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
            code.clear();
            close(hand[fd].indexFile_fd);
            close(fd);

            hand[fd].readflag = true;
            hand[fd].flag = false;
            hand.erase(fd);
        }
        else if (hand[fd].read_true)
        {
            hand[fd].sendByte = send(fd, code.c_str(), code.length(), 0);
            sendHandel(fd, hand[fd].readByte, hand[fd].sendByte);
        }
    }
    else
    {
        hand[fd].sendByte = send(fd, code.c_str(), code.length(), 0);
        sendHandel(fd, hand[fd].readByte, hand[fd].sendByte);
    }
}

void server::store_filesOf_listingFolder(DIR *dir, int fd)
{
    struct dirent *centent;
    while ((centent = readdir(dir)) != NULL)
    {
        if (strcmp(centent->d_name, ".") != 0 && strcmp(centent->d_name, "..") != 0)
        {
            hand[fd].listingFolder_files.push_back(centent->d_name);
        }
    }
}

std::string server::Check_redirectionName(std::string cent_Name)
{

    DIR *dir = opendir(cent_Name.c_str());
    if (dir)
    {
        cent_Name += "/";
        closedir(dir);
        dir = NULL;
        return cent_Name;
    }
    return cent_Name;
}

void server::send_locAutoIndex(int fd, DIR *dir)
{
    if (!hand[fd].Head_send)
    {

        hand[fd].htmlStr = "<!DOCTYPE html>\n<html>\n<head>\n"
                           "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css' integrity='sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO' crossorigin='anonymous'>\n"
                           "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>"
                           "<title>Folder Listing</title>\n"
                           "<style>"
                           "@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);"
                           "body{"
                           "font-family: 'Amiri', serif;"
                           "margin: 50px;"
                           "}"
                           "ul{"
                           "list-style-type: none;"
                           "}"
                           "i{"
                           "margin-right: 6px;"
                           "}"
                           "</style>\n</head>\n<body>"
                           "\n<h1>Folder Listing</h1>"
                           "\n<ul >\n";
        struct dirent *centent;
        while ((centent = readdir(dir)) != NULL)
        {
            if (strcmp(centent->d_name, ".") != 0 && strcmp(centent->d_name, "..") != 0)
            {
                hand[fd].htmlStr += "<li ><i class='far fa-hand-point-right'></i>\n<a class='link-secondary' href=\"";
                hand[fd].htmlStr += Check_redirectionName(centent->d_name);
                hand[fd].htmlStr += "\">";
                hand[fd].htmlStr += centent->d_name;
                hand[fd].htmlStr += "</a> \n</li>\n";
            }
        }
        hand[fd].htmlStr += "</ul></body><script src='https://kit.fontawesome.com/6120a1ecc7.js' crossorigin='anonymous'></script></html>";
        std::stringstream ss;
        ss << hand[fd].htmlStr.length();
        s.OK_200 += "Content-Type: text/html\r\nContent-Length: ";
        s.OK_200 += ss.str();
        s.OK_200 += "\r\n\r\n";
        ssize_t sendH = send(fd, s.OK_200.c_str(), s.OK_200.length(), 0);
        sendHandel(fd, s.OK_200.length(), sendH);
        s.OK_200.clear();
        s.OK_200 = "HTTP/1.1 200 ok\r\n";
        hand[fd].Head_send = true;
    }
    else
    {
        hand[fd].sendByte = send(fd, hand[fd].htmlStr.c_str(), hand[fd].htmlStr.length(), 0);
        sendHandel(fd, hand[fd].htmlStr.length(), hand[fd].sendByte);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
        close(fd);
        hand[fd].readflag = true;
        hand[fd].flag = false;
        if (hand[fd].dir_is_open)
            if (closedir(dir))
                hand[fd].dir_is_open = false;
        hand.erase(fd);
    }
}

std::string server::convert_file_extention(const char *path)
{
    std::string ext;
    std::string File_extention = Get_file_Extention(path);
    for (std::map<std::string, std::string>::iterator it = Extentions.begin(); it != Extentions.end(); it++)
    {
        if (File_extention == it->first)
        {
            ext = it->second;
            break;
        }
    }
    return ext;
}

void server::cgi_exec(const char *path, int fd)
{

    std::string tmp = path;
    Cgi_conf cgi;
    cgi.clear();
    std::string new_path = tmp.substr(tmp.find_first_of("/") + 1);
    cgi.setPath(new_path);

    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    std::vector<Location_storage>::iterator save;
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                if (itr->getLocaPath() == "/cgi-bin")
                    save = itr;
            }
        }
    }
    int error = 0;
    hand[fd].cgi_error = false;
    cgi.initialize_cgi_env(hand[fd], save);
    if (!hand[fd]._cgiwait)
        cgi.cgi_executer(hand[fd], error);
    pid_t tmpp = waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, WNOHANG);
    if (tmpp)
    {
        hand[fd]._cgiwait = false;
        if (hand[fd]._idcgi != -2)
        {
            close(hand[fd].c_in[0]);
            close(hand[fd].c_out[1]);
        }
    }
    else
    {
        if (((clock() - hand[fd].cgistart) / CLOCKS_PER_SEC) >= 30)
        {
            hand[fd]._cgiwait = false;
            close(hand[fd].c_in[0]);
            close(hand[fd].c_out[1]);
            close(hand[fd].c_out[0]);

            kill(hand[fd]._idcgi, SIGKILL);
            waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, 0);

            error = 504;
        }
        else
        {
            hand[fd].cgi_status = true;
            hand[fd]._cgiwait = true;
        }
    }
    if (!hand[fd]._cgiwait)
    {
        if (error == 500)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(500, fd, s.Internal_server_Error_500, "738");
        }
        else if (error == 504)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(504, fd, s.Gateaway_Timeout_504, "726");
        }
        else if (error == 501)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(501, fd, s.Not_Implemented_501, "726");
        }
        else if (error == 403)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(403, fd, s.Forbidden_403, "714");
        }
        else if (hand[fd].statuscgi != 0)
        {
            close(hand[fd].c_in[0]);
            close(hand[fd].c_out[1]);
            close(hand[fd].c_out[0]);
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(502, fd, s.Bad_Gateaway_502, "720");
        }
        if (!hand[fd].cgi_done && !hand[fd].cgi_error)
        {
            hand[fd].indexFile_fd = hand[fd].c_out[0];
        }
    }
}

void server::handel_redirection(int fd)
{
    s.Moved_Permanently_301 += "Location: ";
    s.Moved_Permanently_301 += hand[fd].redirection_path;
    s.Moved_Permanently_301 += "/";
    s.Moved_Permanently_301 += "\r\n\r\n";
    ssize_t sendH = send(fd, s.Moved_Permanently_301.c_str(), s.Moved_Permanently_301.length(), 0);
    sendHandel(fd, s.Moved_Permanently_301.length(), sendH);
    hand[fd].redirection_path.clear();
    s.Moved_Permanently_301.clear();
    s.Moved_Permanently_301 = "HTTP/1.1 301 Moved Permanently\r\n";
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(hand[fd].indexFile_fd);
    close(fd);
    hand[fd].readflag = true;
    hand[fd].flag = false;
    hand.erase(fd);
}

void server::handel_redirection_path(int fd)
{
    s.Moved_Permanently_301 += "Location: ";
    s.Moved_Permanently_301 += hand[fd].pathReturn;
    s.Moved_Permanently_301 += "/";
    s.Moved_Permanently_301 += "\r\n\r\n";
    ssize_t sendH = send(fd, s.Moved_Permanently_301.c_str(), s.Moved_Permanently_301.length(), 0);
    sendHandel(fd, s.Moved_Permanently_301.length(), sendH);
    s.Moved_Permanently_301.clear();
    hand[fd].pathReturn.clear();
    s.Moved_Permanently_301 = "HTTP/1.1 301 Moved Permanently\r\n";
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(hand[fd].indexFile_fd);
    close(fd);
    hand[fd].redirection_Location = true;
    hand[fd].readflag = true;
    hand[fd].flag = false;
    hand.erase(fd);
}

std::string server::Get_code_statusFile(int code)
{
    std::map<int, std::string>::iterator it = code_statusFiles.begin();
    for (; it != code_statusFiles.end(); it++)
    {
        if (it->first == code)
            return it->second;
    }
    return "";
}

void server::Get_F(const char *path, int fd)
{
    hand[fd].read_true = true;
    if (!hand[fd].dir_is_open)
        hand[fd].dir = opendir(path);
    if (hand[fd].dir)
    {
        hand[fd].dir_is_open = true;
        if (Check_redirection(fd))
            handel_redirection(fd);
        else if (CheckLocIndex_file(fd))
        {
            if (Check_Cgi(fd))
            {
                hand[fd].check = true;
                std::string index;
                std::vector<Server_storage>::iterator it = serv.getServers().begin();
                for (; it != serv.getServers().end(); it++)
                {
                    if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
                    {
                        for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
                        {
                            if (itr->getLocaPath() == "/cgi-bin")
                            {
                                index = itr->getLocaPath() + "/" + itr->getLocaIndex();
                                break;
                            }
                        }
                    }
                    if (!index.empty())
                        break;
                }
                if (hand[fd].cgi_status)
                {
                    hand[fd].cgi_done = false;
                    hand[fd].cgi_status = false;
                    hand[fd].cgi_head = false;
                    cgi_exec(index.c_str(), fd);
                }
                if (!hand[fd].cgi_status && !hand[fd].cgi_done && !hand[fd]._cgiwait)
                {
                    ReadAnd_SendContent(fd);
                }
            }
            else if (!Check_Cgi(fd))
                GetCgiFalse(path, fd);
        }
        else if (CheckLocAuto_index(fd))
            send_locAutoIndex(fd, hand[fd].dir);
        else if (CheckServAuto_index(fd))
        {
        }
        else
        {
            sendError(403, fd, s.Forbidden_403, "714");
            return;
        }
    }
    else
    {
        if (!Check_Cgi(fd))
        {
            if (!hand[fd].Head_send)
            {
                std::string ext = convert_file_extention(path);
                OpenFile_forSend(path, fd);
                s.OK_200 += "Content-Type: ";
                s.OK_200 += ext;
                s.OK_200 += "\r\nContent-Length: ";
                s.OK_200 += hand[fd].Size_ofFile;
                s.OK_200 += "\r\n\r\n";
                ssize_t sendH = send(fd, s.OK_200.c_str(), s.OK_200.length(), 0);
                sendHandel(fd, s.OK_200.length(), sendH);
                s.OK_200.clear();
                s.OK_200 = "HTTP/1.1 200 ok\r\n";
                hand[fd].Head_send = true;
            }
            else
            {
                ReadAnd_SendContent(fd);
            }
        }
        else
        {
            hand[fd].check = true;
            if (hand[fd].cgi_status)
            {
                hand[fd].cgi_done = false;
                hand[fd].cgi_status = false;
                hand[fd].cgi_head = false;
                cgi_exec(path, fd);
            }
            if (!hand[fd].cgi_status && !hand[fd].cgi_done && !hand[fd]._cgiwait)
            {
                ReadAnd_SendContent(fd);
            }
        }
    }
}
//

std::string server::Get_file_Extention(const char *path)
{
    std::string tofindFI(path + 1);
    size_t pos = tofindFI.find_last_of(".");
    if (pos == std::string::npos)
    {
        return (".txt");
    }
    std::string FileIn = tofindFI.substr(pos, tofindFI.length() - pos);
    return FileIn;
}

bool server::Check_Cgi(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                size_t pos = GetLocationPath(fd).find("cgi");
                if (pos != std::string::npos)
                    return true;
            }
        }
    }
    return false;
}

bool server::Check_redirection(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                if (GetLocationPath(fd) == itr->getLocaPath())
                {
                    if (!itr->getLocaRedirect().empty())
                    {

                        hand[fd].redirection_path = itr->getLocaRedirect();
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool server::CheckLocAuto_index(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                if (GetLocationPath(fd) == itr->getLocaPath())
                {
                    if (itr->getLocaAutoindex() == true)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool server::CheckServAuto_index(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {

            if (it->getAutoindex() == true)
                return true;
        }
    }
    return false;
}

bool server::CheckLocIndex_file(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            for (std::vector<Location_storage>::iterator itr = it->_locations.begin(); itr != it->_locations.end(); itr++)
            {
                if (GetLocationPath(fd) == itr->getLocaPath())
                {
                    if (!itr->getLocaIndex().empty())
                    {
                        hand[fd].Index_file = itr->getLocaIndex();
                        return true;
                    }
                    else
                    {
                        hand[fd].Index_file = it->getIndex();
                        return false;
                    }
                }
            }
        }
    }
    return false;
}

bool server::CheckServIndex_file(int fd)
{
    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {

            if (!it->getIndex().empty())
                return true;
        }
    }
    return false;
}
