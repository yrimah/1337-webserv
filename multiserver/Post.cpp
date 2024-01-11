#include "../incs/parse_config/server.hpp"
#include "../incs/cgi_config/cgi_conf.hpp"

void server::sendError(int numEroor, int fd, std::string response, std::string length)
{
    response +=
        "Content-Type: text/html\r\n"
        "Content-Length: ";
    response += length;
    response += "\r\n\r\n";
    if (getPathErrorPage(numEroor, fd).empty())
    {
        OpenFile_forSend(Get_code_statusFile(numEroor).c_str(), fd);
        ReadAnd_SendContentER(fd, response);
    }
    else
    {
        OpenFile_forSend(getPathErrorPage(numEroor, fd).c_str(), fd);
        ReadAnd_SendContentER(fd, response);
    }
}

void server::cgi_exec_post(std::string pathm, int fd)
{
    std::string tmp = pathm;
    Cgi_conf cgi;
    cgi.clear();
    std::string new_path = tmp.substr(tmp.find_first_of("/") + 1);
    cgi.setPath(new_path);

    std::vector<Server_storage>::iterator it = serv.getServers().begin();
    std::vector<Server_storage>::iterator serv_save;
    std::vector<Location_storage>::iterator save;
    for (; it != serv.getServers().end(); it++)
    {
        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
        {
            serv_save = it;
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
    std::ifstream file(hand[fd].path.c_str());

    if (!file.is_open())
    {
        hand[fd].cgi_status = true;
        hand[fd].cgi_done = true;
        hand[fd].cgi_error = true;
        sendError(500, fd, "HTTP/1.1 500 internal server error\r\n", "738");
    }
    else
    {

        std::ostringstream buffer;
        buffer << file.rdbuf();

        cgi.final = buffer.str();

        file.close();
        if (!hand[fd]._cgiwait)
        {
            cgi.cgi_executer(hand[fd], error);
        }
        pid_t tmpp = waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, WNOHANG);
        if (tmpp == hand[fd]._idcgi)
        {
            waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, 0);
            hand[fd]._cgiwait = false;
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
        if (error == 500)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(500, fd, "HTTP/1.1 500 internal server error\r\n", "738");
        }
        else if (error == 504)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(504, fd, "HTTP/1.1 504 gateaway timeout\r\n", "726");
        }
        else if (error == 501)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(501, fd, "HTTP/1.1 501 not implemented\r\n", "726");
        }
        else if (error == 403)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(403, fd, "HTTP/1.1 403 forbidden\r\n", "714");
        }
        else if (cgi.getStatus() != 0)
        {
            hand[fd].cgi_status = true;
            hand[fd].cgi_done = true;
            hand[fd].cgi_error = true;
            sendError(502, fd, "HTTP/1.1 502 bad gateway\r\n", "720");
        }
        else
        {
            char cgi_413[1024];
            int _413 = read(cgi._out[0], cgi_413, 1024);
            close(cgi._out[0]);
            if (_413 > 0)
            {
                std::string _413_tmp = cgi_413;
                if (_413_tmp.find("413") != std::string::npos)
                {
                    hand[fd].cgi_status = true;
                    hand[fd].cgi_done = true;
                    hand[fd].cgi_error = true;
                    sendError(413, fd, "HTTP/1.1 413 Content Too Large\r\n", "730");
                }
                if (WIFEXITED(hand[fd].statuscgi))
                    waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, WNOHANG);
            }
        }
    }
}

void server::rest_bady(int fd, std::ofstream &k)
{
    size_t pos = hand[fd].body.find("\r\n");
    if (pos != std::string::npos)
    {
        std::string kh = hand[fd].body.substr(0, pos);
        hand[fd].numR = strtoul(kh.c_str(), NULL, 16);
        hand[fd]._read = hand[fd].rest - pos - 2;
        if (hand[fd].numR != 0)
        {
            if (hand[fd].body.find("\r\n0\r\n") != std::string::npos)
            {
                k.write(hand[fd].body.substr(pos + 2, hand[fd].body.find("\r\n0\r\n")).c_str(), hand[fd].numR);
            }
            else if (hand[fd].numR < hand[fd]._read)
                k.write(hand[fd].body.substr(pos + 2, hand[fd]._read - 2).c_str(), hand[fd]._read - 2);
            else
                k.write(hand[fd].body.substr(pos + 2, hand[fd]._read).c_str(), hand[fd]._read);
        }
        hand[fd].numR -= hand[fd]._read;
        if (hand[fd].numR < 0)
        {
            hand[fd].numR = 0;
            hand[fd].max_body += (hand[fd]._read - 2);
        }
        else
            hand[fd].max_body += hand[fd]._read;
        if (hand[fd].max_body > hand[fd]._client_max_body_size)
        {
            max_body_size(fd);
            hand[fd].is_max = true;
        }
        hand[fd].body.clear();
    }
}

std::string generateRandomString(size_t length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t charsetSize = sizeof(charset) - 1;

    std::string randomString;
    randomString.reserve(length);
    std::srand(static_cast<unsigned int>(std::time(0)));

    for (size_t i = 0; i < length; ++i)
    {
        randomString += charset[std::rand() % charsetSize];
    }

    return randomString;
}

void server::restBodyBoundary(int fd)
{
    hand[fd].getFail = true;
    if ((hand[fd].body.find(hand[fd].lastB) != std::string::npos && std::string::npos == hand[fd].body.find(hand[fd].kh, hand[fd].firstlength)))
        hand[fd].flag = true;
    if (hand[fd].Epoll_out && hand[fd].body.find(hand[fd].lastB) != std::string::npos && std::string::npos == hand[fd].body.find(hand[fd].kh, hand[fd].firstlength))
    {
        send(fd, "HTTP/1.1 201 Created\r\n\r\n", 25, 0);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
        close(fd);
        hand[fd].postFiles.file.close();
        hand[fd].flag = false;
        hand[fd].readflag = true;
        hand[fd].body.clear();
        hand[fd].restBoundary.clear();
        hand[fd].header.clear();
        hand[fd].B = false;
        hand[fd].is_read = false;
        hand[fd].f = true;
        hand[fd].lengthBoundary = 0;
        hand[fd].length = 0;
        hand[fd]._read = 0;
        if (!hand[fd].kh.empty())
            hand[fd].kh.clear();
        if (!hand[fd].lastB.empty())
            hand[fd].lastB.clear();
        hand[fd].end_of_send = false;
        hand[fd].isLast = false;
        hand.erase(fd);
    }
    else if (!(hand[fd].body.find(hand[fd].lastB) != std::string::npos && std::string::npos == hand[fd].body.find(hand[fd].kh, hand[fd].firstlength)))
    {
        size_t pos;
        size_t pos_;
        std::istringstream read(hand[fd].body);
        std::string get;
        std::getline(read, get, '\n');
        if (std::string::npos != get.find(hand[fd].kh))
        {
            get.clear();
            std::getline(read, get, '\n');
            pos = get.find("name=\"");
            pos_ = get.find("\"", pos + 6);
            std::string filename(get.substr(pos + 6, pos_ - pos - 6).c_str());
            get.clear();
            std::getline(read, get, '\n');
            if (get.find("Content-Type") != std::string::npos)
                get = get.substr(14, get.length() - 15);
            std::map<std::string, std::string>::iterator extention = ExtentionsPost.find(get);
            if (extention != ExtentionsPost.end())
            {
                hand[fd].path.clear();
                filename += extention->second;
                hand[fd].path = hand[fd].pathOfUpload;
                if (hand[fd].pathOfUpload[hand[fd].pathOfUpload.length() - 1] != '/')
                    hand[fd].path += "/";
                hand[fd].path += filename;
                hand[fd].postFiles.file.open(hand[fd].path.c_str());
                hand[fd].pathFile_boundary.push_back(hand[fd].path);
            }
            else
            {
                hand[fd].path.clear();
                filename += ".x";
                hand[fd].path = hand[fd].pathOfUpload;
                if (hand[fd].pathOfUpload[hand[fd].pathOfUpload.length() - 1] != '/')
                    hand[fd].path += "/";
                hand[fd].path += filename;
                hand[fd].postFiles.file.open(hand[fd].path.c_str());
                hand[fd].pathFile_boundary.push_back(hand[fd].path);
            }
            size_t rst = hand[fd].body.find("\r\n\r\n");
            hand[fd]._read = hand[fd].rest - rst - 4;
            if (hand[fd]._read > 0)
            {
                std::string l = hand[fd].body.substr(rst + 4, hand[fd]._read);
                pos = l.find(hand[fd].kh);
                if (l.find(hand[fd].lastB) != std::string::npos && l.find(hand[fd].lastB) == pos - 2)
                    hand[fd].isLast = true;
                if (pos == std::string::npos)
                {
                    if (hand[fd]._read > hand[fd].lengthBoundary)
                    {
                        hand[fd].restBoundary = l.substr(hand[fd]._read - hand[fd].lengthBoundary, hand[fd].lengthBoundary);
                        hand[fd].max_body += (hand[fd]._read - hand[fd].lengthBoundary);
                        if (hand[fd].max_body > hand[fd]._client_max_body_size)
                        {
                            max_body_size(fd);
                            hand[fd].is_max = true;
                        }
                        hand[fd].postFiles.file.write(l.substr(0, hand[fd]._read - hand[fd].lengthBoundary).c_str(), hand[fd]._read - hand[fd].lengthBoundary);
                    }
                    else
                        hand[fd].restBoundary = l;
                    hand[fd]._read = 0;
                }
                else
                {
                    hand[fd].max_body += (pos - 2);
                    if (hand[fd].max_body > hand[fd]._client_max_body_size)
                    {
                        max_body_size(fd);
                        hand[fd].is_max = true;
                    }
                    hand[fd].postFiles.file.write(l.substr(0, pos - 2).c_str(), pos - 2);
                    hand[fd].restBoundary = l.substr(pos - 2, hand[fd]._read - pos + 2);
                    hand[fd].postFiles.file.close();
                    hand[fd]._read = hand[fd]._read - pos + 2;
                    hand[fd].bodylength = hand[fd]._read;
                    hand[fd].getFail = false;
                }
            }
            else
            {
                hand[fd].getFail = false;
                hand[fd].bodylength = hand[fd].body.length();
                hand[fd].restBoundary.append(hand[fd].body.c_str(), hand[fd].body.length());
                hand[fd]._read = 0;
            }
            hand[fd].rest = 0;
            hand[fd].body.clear();
        }
    }
    if (hand[fd].isLast)
    {
        hand[fd].body = hand[fd].lastB;
        hand[fd].isLast = false;
        restBodyBoundary(fd);
    }
}

void server::PostBoudary(int fd)
{
    if (Check_Cgi(fd))
        hand[fd].check = true;
    hand[fd].is_max = false;
    memset(hand[fd].buff, 0, 1024);
    std::string key = "Content-Type";
    std::map<std::string, std::string>::iterator ite = hand[fd].request_head.find(key);
    if (ite->first == key)
    {
        size_t pos = ite->second.find(';');
        if (pos != std::string::npos && ite->second.substr(0, pos) == "multipart/form-data")
        {
            key = "Content-length";
            std::map<std::string, std::string>::iterator iter = hand[fd].request_head.find(key);
            if (key == iter->first && atoi(iter->second.c_str()) == 0)
            {
                sendError(400, fd, "HTTP/1.1 400 Bad Request\r\n", "718");
                hand[fd].f = true;
            }
            else
            {
                if (hand[fd].f)
                {
                    hand[fd].lastboundary = true;
                    hand[fd].end_of_send = true;
                    size_t pos_ = ite->second.find("\r");
                    hand[fd].kh = ite->second.substr(pos + 11, pos_ - pos - 11);
                    hand[fd].lastB = "--";
                    hand[fd].lastB += hand[fd].kh;
                    hand[fd].lastB += "--";
                    hand[fd].lengthBoundary = hand[fd].kh.length();
                    hand[fd].firstlength = hand[fd].kh.length();
                    hand[fd].f = false;
                }
                if (hand[fd].check)
                {

                    hand[fd]._read = 0;
                    hand[fd].header.clear();
                    if (!hand[fd].postFiles.file_Cgi.is_open())
                    {
                        hand[fd].path = generateRandomString(5) + ".txt";
                        hand[fd].postFiles.file_Cgi.open(hand[fd].path.c_str(), std::ios::binary);
                    }
                    if (!hand[fd].body.empty())
                    {
                        if (hand[fd].body.find(hand[fd].lastB) != std::string::npos)
                        {
                            hand[fd].lastboundary = false;
                            hand[fd].postFiles.file_Cgi.write(hand[fd].body.c_str(), hand[fd].rest);
                            hand[fd].postFiles.file_Cgi.flush();
                        }
                        else
                        {
                            if (hand[fd].rest < hand[fd].lengthBoundary)
                                hand[fd].restBoundary.append(hand[fd].body.c_str(), hand[fd].rest);
                            else
                            {

                                hand[fd].restBoundary.append(hand[fd].body.substr(hand[fd].rest - hand[fd].lengthBoundary, hand[fd].lengthBoundary).c_str(), hand[fd].lengthBoundary);
                                hand[fd].postFiles.file_Cgi.write(hand[fd].body.substr(0, hand[fd].rest - hand[fd].lengthBoundary).c_str(), hand[fd].rest - hand[fd].lengthBoundary);
                                hand[fd].postFiles.file_Cgi.flush();
                            }
                        }
                        hand[fd].body.clear();
                    }
                    if (hand[fd].Epoll_in && hand[fd].lastboundary)
                    {
                        bzero(hand[fd].buff, 1024);
                        hand[fd]._read = recv(fd, hand[fd].buff, 1024, 0);
                        hand[fd].length = hand[fd]._read;
                        hand[fd].Epoll_in = false;
                    }
                    if (!hand[fd].restBoundary.empty())
                    {
                        hand[fd].header.append(hand[fd].restBoundary.c_str(), hand[fd].lengthBoundary);
                        hand[fd]._read += hand[fd].lengthBoundary;
                        hand[fd].restBoundary.clear();
                    }
                    if (hand[fd].length != 0)
                        hand[fd].header.append(hand[fd].buff, hand[fd].length);
                    if (hand[fd].header.find(hand[fd].lastB) != std::string::npos)
                    {
                        hand[fd].postFiles.file_Cgi.write(hand[fd].header.c_str(), hand[fd]._read);
                        hand[fd].lastboundary = false;
                        hand[fd]._read = 0;
                        hand[fd].postFiles.file_Cgi.close();
                    }
                    else if (!hand[fd].header.empty())
                    {
                        if (hand[fd]._read < hand[fd].lengthBoundary)
                            hand[fd].restBoundary.append(hand[fd].header.c_str(), hand[fd]._read);
                        else
                        {
                            hand[fd].restBoundary.append(hand[fd].header.substr(hand[fd]._read - hand[fd].lengthBoundary, hand[fd].lengthBoundary).c_str(), hand[fd].lengthBoundary);
                            hand[fd].postFiles.file_Cgi.write(hand[fd].header.substr(0, hand[fd]._read - hand[fd].lengthBoundary).c_str(), hand[fd]._read - hand[fd].lengthBoundary);
                            hand[fd].postFiles.file_Cgi.flush();
                        }
                    }
                    if (!hand[fd].lastboundary)
                    {
                        hand[fd].cgi_done = false;
                        hand[fd].cgi_status = false;
                        hand[fd].cgi_head = false;
                        hand[fd].cgi_error = false;
                        cgi_exec_post(hand[fd].path_Get, fd);
                        if (!hand[fd].cgi_done && !hand[fd].cgi_error)
                        {
                            const char *cgi_resp = "HTTP/1.1 201 Created\r\nContent-type: text/html\r\n\r\n"
                                                   "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"
                                                   "<style>"
                                                   "@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);"
                                                   "body{"
                                                   "font-family : 'Amiri', serif;"
                                                   "margin: 50px;"
                                                   "}"
                                                   "</style>"
                                                   "<h2><center>File uploaded Successfuly</center></h2><br>"
                                                   "<center><a href='upload.php' class='btn btn-outline-success'>GO BACK</a></center>"
                                                   "\r\n\r\n";
                            send(fd, cgi_resp, 527, 0);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                            hand[fd].flag = false;
                            hand[fd].readflag = true;
                            close(fd);
                            if (WIFEXITED(hand[fd].statuscgi))
                                waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, 0);
                            hand.erase(fd);
                        }
                        hand[fd].f = true;
                        hand[fd].lastboundary = true;
                        hand[fd].restBoundary.clear();
                        hand[fd].check = false;
                        hand[fd].header.clear();
                    }
                }
                else
                {
                    if (!hand[fd].body.empty())
                    {
                        restBodyBoundary(fd);
                    }
                    if (hand[fd].end_of_send)
                    {
                        if (hand[fd].Epoll_in && ((!hand[fd].is_max && hand[fd]._read == 0 && !hand[fd].B) || !hand[fd].getFail))
                        {
                            hand[fd].Epoll_in = false;
                            hand[fd]._read = recv(fd, hand[fd].buff, 1024, 0);
                            hand[fd].length = hand[fd]._read;
                            hand[fd].is_read = true;
                        }
                        if (!hand[fd].restBoundary.empty())
                        {
                            if (!hand[fd].getFail)
                            {
                                hand[fd].header.append(hand[fd].restBoundary.c_str(), hand[fd].bodylength);
                                hand[fd]._read += hand[fd].bodylength;
                            }
                            else
                            {
                                hand[fd].header.append(hand[fd].restBoundary.c_str(), hand[fd].lengthBoundary);
                                hand[fd]._read += hand[fd].lengthBoundary;
                            }
                            hand[fd].restBoundary.clear();
                        }
                        if (hand[fd].is_read)
                        {
                            hand[fd].header.append(hand[fd].buff, hand[fd].length);
                            hand[fd].is_read = false;
                        }
                        if (hand[fd].header.find(hand[fd].lastB) != std::string::npos)
                        {
                            hand[fd].B = true;
                        }
                        if (!hand[fd].is_max && !hand[fd].header.empty() && hand[fd].header.find(hand[fd].kh) == std::string::npos)
                        {
                            hand[fd].max_body += (hand[fd]._read - hand[fd].lengthBoundary);
                            if (hand[fd].max_body > hand[fd]._client_max_body_size)
                            {
                                max_body_size(fd);
                                hand[fd].is_max = true;
                            }
                            hand[fd].postFiles.file.write(hand[fd].header.substr(0, hand[fd]._read - hand[fd].lengthBoundary).c_str(), hand[fd]._read - hand[fd].lengthBoundary);
                            hand[fd].restBoundary.clear();
                            hand[fd].restBoundary = hand[fd].header.substr(hand[fd]._read - hand[fd].lengthBoundary, hand[fd].lengthBoundary);
                            hand[fd]._read = 0;
                        }
                        else if (!hand[fd].is_max && hand[fd].header.find(hand[fd].kh) != std::string::npos)
                        {
                            if (((double)hand[fd].header.find(hand[fd].kh) - 4) > 0)
                            {
                                hand[fd].max_body += (hand[fd].header.find(hand[fd].kh) - 4);
                            }
                            if (hand[fd].max_body > hand[fd]._client_max_body_size)
                            {
                                max_body_size(fd);
                                hand[fd].is_max = true;
                            }
                            hand[fd].postFiles.file.write(hand[fd].header.substr(0, hand[fd].header.find(hand[fd].kh) - 4).c_str(), hand[fd].header.find(hand[fd].kh) - 4);
                            hand[fd].body = hand[fd].header.substr(hand[fd].header.find(hand[fd].kh) - 2, hand[fd]._read - hand[fd].header.find(hand[fd].kh) + 2);
                            hand[fd].postFiles.file.close();
                            if ((hand[fd].body.find(hand[fd].lastB) != std::string::npos && std::string::npos == hand[fd].body.find(hand[fd].kh, hand[fd].firstlength)))
                            {
                                hand[fd].flag = true;
                            }
                            hand[fd].rest = hand[fd]._read - hand[fd].header.find(hand[fd].kh) + 2;
                            if (hand[fd].body.find(hand[fd].kh, hand[fd].firstlength) - 2 == hand[fd].body.find(hand[fd].lastB, hand[fd].firstlength))
                                restBodyBoundary(fd);
                        }
                        hand[fd].header.clear();
                    }
                }
            }
        }
        else
        {
            std::string key = "Content-Length";
            std::map<std::string, std::string>::iterator it = hand[fd].request_head.lower_bound(key);
            if (key == it->first)
            {
                if (hand[fd].f)
                {
                    hand[fd].length = 0;
                    hand[fd].lenIsZero = false;
                    hand[fd].length = std::strtoll(it->second.c_str(), NULL, 10);
                    if (hand[fd].length == 0)
                        hand[fd].lenIsZero = true;
                    hand[fd].f = false;
                }
                if (hand[fd].length > hand[fd]._client_max_body_size)
                {
                    sendError(413, fd, "HTTP/1.1 413 Content Too Large\r\n", "730");
                    hand[fd].f = true;
                }
                else
                {
                    if (!hand[fd].postFiles.k.is_open())
                        creat_file(fd);
                    if (!hand[fd].body.empty())
                    {
                        hand[fd].postFiles.k.write(hand[fd].body.c_str(), hand[fd].rest);
                        hand[fd].body.clear();
                        hand[fd].length -= hand[fd].rest;
                    }
                    if (hand[fd].length == 0)
                    {
                        if (!hand[fd].check && hand[fd].Epoll_out)
                        {
                            hand[fd].f = true;
                            hand[fd].postFiles.k.close();
                            hand[fd].max_body = 0;
                            hand[fd].flag = false;
                            if (!hand[fd].lenIsZero)
                            {
                                send(fd, "HTTP/1.1 201 Created\r\n\r\n", 25, 0);
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                                close(fd);
                                hand[fd].flag = false;
                                hand[fd].readflag = true;
                                hand.erase(fd);
                                return;
                            }
                            else
                                sendError(400, fd, "HTTP/1.1 400 Bad Request\r\n", "718");
                        }
                        else
                        {
                            hand[fd].cgi_done = false;
                            hand[fd].cgi_status = false;
                            hand[fd].cgi_head = false;
                            hand[fd].cgi_error = false;
                            cgi_exec_post(hand[fd].path_Get, fd);
                            if (!hand[fd].cgi_done && !hand[fd].cgi_error)
                            {
                                const char *cgi_resp = "HTTP/1.1 201 Created\r\nContent-type: text/html\r\n\r\n"
                                                       "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"
                                                       "<style>"
                                                       "@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);"
                                                       "body{"
                                                       "font-family : 'Amiri', serif;"
                                                       "margin: 50px;"
                                                       "}"
                                                       "</style>"
                                                       "<h2><center>File uploaded Successfuly</center></h2><br>"
                                                       "<center><a href='upload.php' class='btn btn-outline-success'>GO BACK</a></center>"
                                                       "\r\n\r\n";
                                send(fd, cgi_resp, 527, 0);
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                                if (WIFEXITED(hand[fd].statuscgi))
                                    waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, 0);
                                close(fd);
                                hand[fd].flag = false;
                                hand[fd].readflag = true;
                                hand.erase(fd);
                            }
                            hand[fd].max_body = 0;
                            hand[fd].postFiles.k.close();
                            hand[fd].f = true;
                            return;
                        }
                    }
                    else if (hand[fd].Epoll_in && (hand[fd].length <= (size_t)hand[fd].readsize))
                    {
                        hand[fd].Epoll_in = false;
                        hand[fd]._read = recv(fd, hand[fd].buff, hand[fd].length, 0);
                        hand[fd].length -= hand[fd]._read;
                        hand[fd].postFiles.k.write(hand[fd].buff, hand[fd]._read);
                    }
                    else if (hand[fd].Epoll_in)
                    {
                        hand[fd].Epoll_in = false;
                        hand[fd]._read = recv(fd, hand[fd].buff, hand[fd].readsize, 0);
                        hand[fd].length -= hand[fd]._read;
                        hand[fd].postFiles.k.write(hand[fd].buff, hand[fd]._read);
                    }
                    if (hand[fd].length == 0)
                        hand[fd].flag = true;
                }
            }
            else
            {
                sendError(411, fd, "HTTP/1.1 411 Length Required\r\n", "726");
                hand[fd].f = true;
            }
        }
    }
    else
    {
        sendError(400, fd, "HTTP/1.1 400 Bad Request\r\n", "718");
        hand[fd].f = true;
    }
}

void server::max_body_size(int fd)
{
    for (std::vector<std::string>::iterator it = hand[fd].pathFile_boundary.begin(); it != hand[fd].pathFile_boundary.end(); it++)
    {
        std::remove((*it).c_str());
    }
    hand[fd].f = true;
    hand[fd].max_body = 0;
    sendError(413, fd, "HTTP/1.1 413 Content Too Large\r\n", "730");
}

void server::Post(int fd)
{
    try
    {
        std::vector<Server_storage>::iterator itpar = serv.getServers().begin();
        for (; itpar != serv.getServers().end(); itpar++)
        {
            if (itpar->getPort() == hand[fd].Get_port() && itpar->getHost() == hand[fd].Get_host())
            {
                hand[fd]._client_max_body_size = itpar->getClientMaxBodySize();
                for (std::vector<Location_storage>::iterator itr2 = itpar->_locations.begin(); itr2 != itpar->_locations.end(); itr2++)
                {
                    std::string locationPath = GetLocationPath(fd);
                    if (locationPath.empty())
                    {
                        sendError(404, fd, s.NotFond_404, "714");
                        return;
                    }
                    else if (locationPath == "x")
                        locationPath = hand[fd].pathReturn;
                    if (locationPath == itr2->getLocaPath())
                    {
                        if (itr2->getLocaAllowedMethods().at(1) == 0)
                            throw std::runtime_error("not allowed method");
                        hand[fd].up_allowed = itr2->getLocaUpallow();
                        hand[fd].pathOfUpload = itr2->getLocaUpdir();
                        break;
                    }
                }
            }
        }

        if (hand[fd].up_allowed)
        {
            hand[fd].check = false;
            if (Check_Cgi(fd))
                hand[fd].check = true;
            int flags = false;
            if ((hand[fd].numR == 0 && !hand[fd].Epoll_out) || (hand[fd].numR == 0 && hand[fd].Epoll_in))
                flags = true;
            hand[fd].isChunkedBounadry = false;
            std::string key = "Transfer-Encoding";
            std::map<std::string, std::string>::iterator it = hand[fd].request_head.lower_bound(key);
            std::string key1 = "Content-Type";
            std::map<std::string, std::string>::iterator ite = hand[fd].request_head.lower_bound(key1);
            if (ite->first == key1 && it->first == key)
            {
                size_t pos = ite->second.find(';');
                if (pos != std::string::npos && ite->second.substr(0, pos) == "multipart/form-data")
                {
                    hand[fd].isChunkedBounadry = true;
                    sendError(501, fd, "HTTP/1.1 501 Not Implemented\r\n", "726");
                }
            }
            if (!hand[fd].isChunkedBounadry && it->first == key)
            {

                hand[fd].is_max = false;
                hand[fd].notSepportedMedia = true;
                if (!hand[fd].postFiles.k.is_open())
                    creat_file(fd);
                if (hand[fd].notSepportedMedia && it->second == "chunked\r\n")
                {
                    hand[fd].max_body = 0;
                    std::vector<Server_storage>::iterator it = serv.getServers().begin();
                    for (; it != serv.getServers().end(); it++)
                    {
                        if (it->getPort() == hand[fd].Get_port() && it->getHost() == hand[fd].Get_host())
                        {
                            hand[fd]._client_max_body_size = it->getClientMaxBodySize();
                            break;
                        }
                    }
                    if (!hand[fd].body.empty())
                        rest_bady(fd, hand[fd].postFiles.k);
                    if (hand[fd].Epoll_in && (!hand[fd].is_max && hand[fd].numR > 0 && hand[fd].numR > hand[fd].readsize))
                    {
                        bzero(hand[fd].buff, 1024);
                        hand[fd]._read = recv(fd, hand[fd].buff, hand[fd].readsize, 0);
                        hand[fd].numR -= hand[fd]._read;
                        hand[fd].max_body += hand[fd]._read;
                        if (hand[fd].max_body > hand[fd]._client_max_body_size)
                        {
                            max_body_size(fd);
                            hand[fd].is_max = true;
                        }
                        hand[fd].postFiles.k.write(hand[fd].buff, hand[fd]._read);
                        flags = true;
                    }
                    else if (hand[fd].Epoll_in && (!hand[fd].is_max && hand[fd].numR > 0 && hand[fd].numR <= hand[fd].readsize))
                    {
                        bzero(hand[fd].buff, 1024);
                        hand[fd]._read = recv(fd, hand[fd].buff, hand[fd].numR, 0);
                        hand[fd].numR -= hand[fd]._read;
                        hand[fd].max_body += hand[fd]._read;
                        if (hand[fd].max_body > hand[fd]._client_max_body_size)
                        {
                            max_body_size(fd);
                            hand[fd].is_max = true;
                        }
                        hand[fd].postFiles.k.write(hand[fd].buff, hand[fd]._read);
                        flags = true;
                    }
                    if (hand[fd].Epoll_in && !hand[fd].is_max && hand[fd].numR == 0 && flags)
                    {
                        bzero(hand[fd].buff, 1024);
                        hand[fd]._read = recv(fd, hand[fd].buff, hand[fd].readsize, 0);
                        std::string l;
                        if (hand[fd].buff[0] == '\r' && hand[fd].buff[1] == '\n')
                        {
                            if (hand[fd]._read == 2)
                            {
                                bzero(hand[fd].buff, 1024);
                                hand[fd]._read = recv(fd, hand[fd].buff, 1024, 0);
                                l.append(hand[fd].buff, hand[fd]._read);
                                hand[fd]._read += 2;
                            }
                            else
                                l.append(&(hand[fd].buff[2]), hand[fd]._read - 2);
                        }
                        else
                            l.append(hand[fd].buff, hand[fd]._read);
                        size_t pos = l.find("\r\n");
                        if (pos != std::string::npos)
                        {
                            hand[fd]._read -= (pos + 4);
                            hand[fd].numR = strtoul(l.substr(0, pos).c_str(), NULL, 16);
                            if (hand[fd].numR == 0)
                            {
                                flags = false;
                            }
                            else
                            {
                                hand[fd].max_body += hand[fd]._read;
                                if (hand[fd].max_body > hand[fd]._client_max_body_size)
                                {
                                    max_body_size(fd);
                                    hand[fd].is_max = true;
                                }
                                hand[fd].postFiles.k.write(l.substr(pos + 2, hand[fd]._read).c_str(), hand[fd]._read);
                                hand[fd].numR -= hand[fd]._read;
                            }
                        }
                    }
                    hand[fd].Epoll_in = false;
                    if (!hand[fd].is_max && hand[fd].numR == 0 && !flags)
                        hand[fd].flag = true;
                    if (hand[fd].Epoll_out && !hand[fd].is_max && hand[fd].numR == 0 && !flags)
                    {
                        if (!hand[fd].check)
                        {
                            send(fd, "HTTP/1.1 201 Created\r\n\r\n", 25, 0);
                            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                            close(fd);
                            hand[fd].postFiles.k.close();
                            hand[fd].flag = false;
                            hand[fd].readflag = true;
                            hand[fd].max_body = 0;
                            hand.erase(fd);
                        }
                        else
                        {
                            hand[fd].cgi_done = false;
                            hand[fd].cgi_status = false;
                            hand[fd].cgi_head = false;
                            hand[fd].cgi_error = false;
                            cgi_exec_post(hand[fd].path_Get, fd);
                            if (!hand[fd].cgi_done && !hand[fd].cgi_error)
                            {
                                const char *cgi_resp = "HTTP/1.1 201 Created\r\nContent-type: text/html\r\n\r\n"
                                                       "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.1.3/dist/css/bootstrap.min.css\" integrity=\"sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO\" crossorigin=\"anonymous\">"
                                                       "<style>"
                                                       "@import url(https://fonts.googleapis.com/earlyaccess/amiri.css);"
                                                       "body{"
                                                       "font-family : 'Amiri', serif;"
                                                       "margin: 50px;"
                                                       "}"
                                                       "</style>"
                                                       "<h2><center>File uploaded Successfuly</center></h2><br>"
                                                       "<center><a href='upload.php' class='btn btn-outline-success'>GO BACK</a></center>"
                                                       "\r\n\r\n";
                                send(fd, cgi_resp, 527, 0);
                                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
                                hand[fd].flag = false;
                                hand[fd].readflag = true;
                                close(fd);
                                if (WIFEXITED(hand[fd].statuscgi))
                                    waitpid(hand[fd]._idcgi, &hand[fd].statuscgi, 0);
                                hand.erase(fd);
                            }
                            hand[fd].max_body = 0;
                            hand[fd].postFiles.k.close();
                        }
                    }
                }
            }
            else if (!hand[fd].isChunkedBounadry && it->second != "chunked\r\n")
            {
                PostBoudary(fd);
            }
        }
        else
        {
            sendError(403, fd, "HTTP/1.1 403 Forbidden\r\n", "714");
            hand[fd].postFiles.k.close();
        }
    }
    catch (std::exception &e)
    {
        std::string what = e.what();
        if (what == "not allowed method")
            sendError(405, fd, "HTTP/1.1 405 Method Not Allowed\r\n", "732");
        else
            sendError(500, fd, "HTTP/1.1 500 internal server error\r\n", "738");
    }
}

std::ofstream &server::creat_file(int fd)
{
    std::string key = "Content-Type";
    std::map<std::string, std::string>::iterator it = hand[fd].request_head.find(key);
    if (it != hand[fd].request_head.end() && ExtentionsPost.find(it->second.substr(0, it->second.length() - 2)) != ExtentionsPost.end())
    {
        hand[fd].fileN = generateRandomString(5);
        hand[fd].fileN += ExtentionsPost.find(it->second.substr(0, it->second.length() - 2))->second;
        hand[fd].path = hand[fd].pathOfUpload;
        if (hand[fd].pathOfUpload[hand[fd].pathOfUpload.length() - 1] != '/')
            hand[fd].path += "/";
        hand[fd].path += hand[fd].fileN;
        hand[fd].postFiles.k.open(hand[fd].path.c_str(), std::ios::binary);
        hand[fd].pathFile_boundary.push_back(hand[fd].path);
    }
    else if (it == hand[fd].request_head.end())
        sendError(400, fd, "HTTP/1.1 400 Bad Request\r\n", "718");
    else
    {
        hand[fd].notSepportedMedia = false;
        sendError(415, fd, "HTTP/1.1 415 Unsupported Media Type\r\n", "740");
    }
    return (hand[fd].postFiles.k);
}