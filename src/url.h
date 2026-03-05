#pragma once
#include <curl/curl.h>
#include <iostream>

class urlReader
{
  public:
    std::string scheme;
    std::string host;
    std::string path;

    urlReader();
    bool read(std::string input);
    void request(std::string &header, std::string &body);
};

class curlReader
{
  public:
    curlReader();
    ~curlReader();
    bool fetch(std::string url, std::string &data);

  private:
    CURL *curl;
};
