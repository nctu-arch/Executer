/*************************************************

SystemC Hierarchical Parameters Parser
(c) 2/5/2013 W. Rhett Davis

**************************************************/

#pragma once
#include <map>
#include <string>

using namespace std;

class hier_param {
  public:
    map<string,map<string,string> > param;
  
    hier_param(const char *filename);
    map<string,string> &get(string hiername);
    void dump();
};

int stoi(string val);
float stof(string val);
