#pragma once

#include <map>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <vector>
#include "StatusCodes.hpp"
#include <csignal>
#include <dirent.h>
#include <fstream>
#include "files.hpp"
#include <cstdlib>

class Request
{
public:
    int check_pathForDelete;
    int checkRequestLine;
    std::string LocaRoot;
    std::string ServRoot;
    std::string Full_path;
    std::string pathReturn;
    std::string method;
    std::string path_Get;
    std::string http_version;
    std::string value;
    std::string key;
    std::string Port;
    std::string Host;
    std::string Index_file;
    bool redirection_Location;
    bool readflag;
    bool flag;
    bool Head_send;
    bool indexFile_open;
    bool send_filed;
    bool read_filed;
    bool read_true;
    bool dir_is_open;
    int indexFile_fd;
    std::string htmlStr;
    std::string redirection_path;
    DIR *dir;
    std::string Size_ofFile;
    off_t S_sizeOfFile;
    off_t size_fend;
    std::map<std::string, std::string> request_head;
    std::vector<std::string> listingFolder_files;
    char BufReadIndex[1024];
    ssize_t readByte;
    ssize_t sendByte;
    //post
    files postFiles;
    char buff[1024];
    int rest;
    size_t length;
    int numR;
    bool f;
    int readsize;
    ssize_t _read;
    std::string header;
    std::string body;
    int lengthBoundary;
    std::string restBoundary;
    std::string kh;
    std::string lastB;
    bool B;
    bool end_of_send;
    bool is_read;
    bool getFail;
    int bodylength;
    bool lenIsZero;
    bool check;
    bool lastboundary;
    std::string path;
    std::string fileN;
    int firstlength;
    bool isChunkedBounadry;
    bool is_max;
    std::vector<std::string> pathFile_boundary;
    size_t max_body;
    size_t _client_max_body_size;
    bool isLast;
    std::string pathOfUpload;
    bool up_allowed;
    bool notSepportedMedia;
    clock_t start_time;
    clock_t end_time;
    bool Epoll_in;
    bool Epoll_out;
    //
    uint16_t port;
    in_addr_t host;
    int store_reverse_host;
    //
    std::string query_string;
    //for cgi
    bool cgi_status;
    bool cgi_error;
    bool cgi_head;
    bool cgi_done;
    bool _cgiwait;
    pid_t _idcgi;
    int statuscgi;
    int c_in[2];
    int c_out[2];

    clock_t cgistart;
    clock_t cgiend;

public:
    Request();
    ~Request();
    void check_path_forDelete(std::string path_forDelete);
    void read_request(char b[]);
    uint16_t Get_port();
    in_addr_t Get_host();
    void ft_checkRequestLine(std::string);
    int checkURL(std::string url);
};
