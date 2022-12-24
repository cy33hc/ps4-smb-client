#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <set>

enum RequestType
{
    HTTP_GET = 0,
    HTTP_POST = 1,
    HTTP_PUT = 2,
    HTTP_HEAD = 3
};

class Request
{
private:
    RequestType request_type; // get => 0 , post => 1 , put=> 2
    std::string url;
    std::string text;
    std::string url_path;
    std::map<std::string, std::string> request_inputs;
    void UrlParser(std::string url);
    static char FromHex(char ch);
    static char ToHex(char code);

public:
    Request(char *buffer, int buffer_length);
    RequestType GetRequestType();
    std::string GetParameter(std::string a);
    std::string GetBody();
    std::string GetPath();
    static std::string UrlEncode(std::string url);
    static std::string UrlDecode(std::string url);
};

#endif