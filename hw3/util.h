

# pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <map>

class ParameterReader
{
public:
    ParameterReader(std::string filename="./config" )
    {
        std::ifstream fin( filename.c_str() );
        if (!fin)
        {
            std::cerr<<"parameter file does not exist."<<std::endl;
            return;
        }
        while(!fin.eof())
        {
            std::string str;
            std::getline( fin, str );
            if (str[0] == '#' || str.empty())  continue;
            data.push_back(str);
            // int pos = str.find("=");
            // if (pos == -1) continue;
            // std::string key = str.substr( 0, pos );
            // std::string value = str.substr( pos+1, str.length() );
            // data[key] = value;

            if (!fin.good()) break;
        }
    }
    // std::string getData(std::string key )
    // {
    //     auto iter = data.find(key);
    //     if (iter == data.end())
    //     {
    //         std::cerr<<"Parameter name "<<key<<" not found!"<<std::endl;
    //         return std::string("NOT_FOUND");
    //     }
    //     return iter->second;
    // }
    // std::vector<std::string> getServerNames() {
    //         std::vector<std::string> res;
    //         for(auto const& item: data) res.push_back(item.first);
    //         return res;
    // }
    // std::vector<std::string> getServerPorts() {
    //         std::vector<std::string> res;
    //         for(auto const& item: data) res.push_back(item.second);
    //         return res;
    // }
    std::vector<std::string> getData() {
        return data;
    }

private:
    // std::map<std::string, std::string> data;
    std::vector<std::string> data;
};