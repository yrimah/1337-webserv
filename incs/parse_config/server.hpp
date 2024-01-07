#pragma once

#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include "conf_parser.hpp"
#include <cstring>
#include <fcntl.h>
#include <algorithm>
#include "Request.hpp"
#include "StatusCodes.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <ctime>
#define MAX_EVN 1024

class server
{
public:
    std::vector<int> soket_fd;
    std::vector<int> client_fd;
    int epoll_fd;
    std::map<int,std::pair<uint16_t, in_addr_t> > serversMap;
    std::map<int, Request> hand;
    Conf_parser serv;
    StatusCodes s;
    std::map<std::string, std::string> Extentions;
    std::map<std::string, std::string> ExtentionsPost;
    
    
    
    int i;
    
    std::string nameFile_Cgi;
    // std::string cgi_error_code;
    std::map<int , std::string> code_statusFiles;
    
public:
    server();
    std::string Check_redirectionName(std::string cent_Name);
    void creat_servers();
    void RecDelete(const char *path, int fd);
    void bindAnd_ListenServers();
    void Request_handling(int fd);
    bool Read_Request(int fd);
    void Get_handling(int fd);
    void Delete_handling(int fd);
    std::string GetLocationPath(int fd);
    bool CheckLocIndex_file(int fd);
    bool CheckServIndex_file(int fd);
    bool Check_Cgi(int fd);
    bool CheckLocAuto_index(int fd);
    bool CheckServAuto_index(int fd);
    bool OpenLocIDXFile_forSend(const char *path, int fd);
    void OpenServIDXFile_forSend(int fd);
    void OpenFile_forSend(const char *path, int fd);
    void send_locAutoIndex(int fd, DIR *dir);
    void store_filesOf_listingFolder(DIR *dir, int fd);
    void ReadAnd_SendContent(int fd);
    void handel_redirection_path(int fd);
    //
    void ReadAnd_SendContentER(int fd , std::string code);
    std::string Get_file_Extention(const char *path);
    void Store_extention();
    void stor_code_statusFiles();
    void sendHandel(int fd, int read, int send);
    void GetCgiFalse(const char *path, int fd);
    bool Check_redirection(int fd);
    void handel_redirection(int fd);
    const std::string getPathErrorPage(int code , int fd);
    std::string Get_code_statusFile(int code);
    //
    void cgi_exec(const char *path, int fd);
    void send_cgi_error(const char *path, int fd, int cgi_fd);
    void OpenErrorFIle_forSend(const char *path, int fd);
    //
    std::string get_path(int fd);
    void Get_F(const char *path, int fd);
    std::ofstream & creat_file(int fd);
    std::string convert_file_extention(const char *path);
    /// 
    /// for post
    void PostBoudary(int fd);
    void Post(int fd);
    void rest_bady(int fd, std::ofstream &file);
    void restBodyBoundary(int fd);
    void max_body_size(int fd);
    void cgi_exec_post(std::string path, int fd);
    void sendError(int numEroor, int fd, std::string response, std::string length);
    
    ~server();
    class Filed_creatSocket : public std::exception
    {
    public:
        const char *what() const throw();
    };
    class Filed_epoll : public std::exception
    {
        public:
            const char *what() const throw();
    };
    class Filed_bind : public std::exception
    {
    public:
        const char *what() const throw();
    };
    class Filed_listen : public std::exception
    {
    public:
        const char *what() const throw();
    };
};
