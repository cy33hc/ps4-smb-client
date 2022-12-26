#include <stdlib.h>
#include <vector>
#include "http_request.h"
#include "util.h"

/* Converts a hex character to its integer value */
char Request::FromHex(char ch)
{
    return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
}

/* Converts an integer value to its hex character*/
char Request::ToHex(char code)
{
    static char hex[] = "0123456789ABCDEF";
    return hex[code & 15];
}

std::string Request::UrlEncode(std::string url)
{
    const char *str = url.c_str();
    std::vector<char> v(url.size());
    v.clear();
    for (size_t i = 0, l = url.size(); i < l; i++)
    {
        char c = str[i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
            c == '*' || c == '\'' || c == '(' || c == ')' || c =='/')
        {
            v.push_back(c);
        }
        else
        {
            v.push_back('%');
            v.push_back(ToHex(c >> 4));
            v.push_back(ToHex(c & 15));
        }
    }

    return std::string(v.cbegin(), v.cend());
}

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
std::string Request::UrlDecode(std::string url)
{
    const char *str = url.c_str();
    std::vector<char> v(url.size());
    v.clear();
    for (size_t i = 0, l = url.size(); i < l; i++)
    {
        char c = str[i];
        if (c == '%')
        {
            if (i+2 < l)
            {
                v.push_back(FromHex(str[i+1]) << 4 | FromHex(str[i+2]));
                i = i + 2;
            }
        }
        else if (c == '+')
        {
            v.push_back(' ');
        }
        else
        {
            v.push_back(c);
        }
    }

    return std::string(v.cbegin(), v.cend());
}

Request::Request(char *buffer, int buffer_length)
{
    if (buffer_length == 0)
        return;
    std::vector<std::string> lines;
    int header_ends = 0;
    for (int i = 0; i < buffer_length; i++)
    {
        std::string a;
        while (i < buffer_length && buffer[i] != '\n' && buffer[i] != EOF)
        {
            a += buffer[i];
            i++;
        }
        if ((a.size() == 0 || a[0] == '\n' || a[0] == '\r') && header_ends == 0)
        {
            header_ends = lines.size();
            break;
        }
        lines.push_back(a);
    }

    std::string tmp[3];
    int tmp_cnt = 0;
    for (int i = 0; i < lines[0].size(); i++)
    {
        if (lines[0][i] == ' ')
            tmp_cnt++;
        else
            tmp[tmp_cnt].push_back(lines[0][i]);
    }

    HeaderParser(lines);

    if (tmp[0] == "GET")
        request_type = HTTP_GET;
    else if (tmp[0] == "POST")
        request_type = HTTP_POST;
    else if (tmp[0] == "PUT")
        request_type = HTTP_PUT;
    else if (tmp[0] == "HEAD")
        request_type = HTTP_HEAD;

    url = tmp[1];
    if (request_type == HTTP_GET || request_type == HTTP_HEAD)
        UrlParser(url);

    {
        int t = buffer_length--;
        while (t >= 0 && buffer[t] != '\n' && buffer[t] != '=')
            t--;
        t++;
        while (t < buffer_length)
        {
            text += buffer[t];
            t++;
        }
        text += buffer[t];
    }
}

RequestType Request::GetRequestType()
{
    return request_type;
}

std::string Request::GetPath()
{
    return url_path;
}

void Request::HeaderParser(std::vector<std::string> &lines)
{
    for (int i = 1; i < lines.size(); i++)
    {
        size_t colon = lines[i].find_first_of(":");
        std::string name = lines[i].substr(0, colon);
        name = Util::Trim(name, " ");
        std::string value = lines[i].substr(colon+1);
        headers[Util::ToLower(name)] = Util::Trim(value, " ");
    }
}

void Request::UrlParser(std::string url)
{
    size_t params_start = url.find_first_of("?");

    url_path = url.substr(0, params_start);

    // return if "?" not found
    if (params_start == std::string::npos) return;

    std::string params = url.substr(params_start+1);
    bool has_next = true;
    while (has_next)
    {
        size_t param_separator_pos = params.find_first_of("&");
        has_next = param_separator_pos != std::string::npos && param_separator_pos != params.length()-1;
        std::string param = UrlDecode(params.substr(0, param_separator_pos));
        size_t equal_pos = param.find_first_of("=");
        if (equal_pos == std::string::npos)
        {
            request_inputs[param] = "";
        }
        else
        {
            std::string name = param.substr(0, equal_pos);
            std::string value = param.substr(equal_pos+1);
            request_inputs[name] = value;
        }
        params = params.substr(param_separator_pos+1);
    }
}

std::string Request::GetParameter(std::string a)
{
    if (request_inputs.count(a) == 0)
        return "";
    return request_inputs[a];
}

std::string Request::GetBody()
{
    return text;
}
