/*************************************************

SystemC Hierarchical Parameters Parser
(c) 2/5/2013 W. Rhett Davis

**************************************************/

#include "hier_param.h"

#include <fstream>
#include <iostream>
#include <sstream>

hier_param::hier_param(const char *filename)
{
  string line,hiername,paramname,paramval;
  size_t pos;
  ifstream f(filename);

  while (f.good()) {
    getline(f,line);
    // Strip out comment
    pos=line.find_first_of('#');
    line=line.substr(0,pos);
    //cout << "Line: \"" << line << "\"\n";
    if (line.size()==0)
      continue;
    // Strip out leading white-space
    pos=line.find_first_not_of(" \t\n\r");
    if (pos>=line.size())
      continue;
    else
      line=line.substr(pos);
    // Strip out trailing white-space
    pos=line.find_last_not_of(" \t\n\r");
    if (pos>=line.size())
      continue;
    else
      line=line.substr(0,pos+1);
    //cout << "Line: \"" << line << "\"\n";
    // Find and strip param value
    pos=line.find_last_of(" \t=");
    if (pos>=line.size()-1)
      continue;
    paramval=line.substr(pos+1);
    pos=line.find_first_of(" \t=");
    line=line.substr(0,pos);
    // If the remaining line contains ".", then find the hier & param names
    pos=line.find_last_of('.');
    if (pos>line.size()-2) {
      // Assume that this is a root param
      hiername="";
      paramname=line;
    }
    else {
      hiername=line.substr(0,pos);
      paramname=line.substr(pos+1);
    }
    //cout << "hiername: " << hiername << " paramname: " << paramname 
    //      << " val: " << paramval << endl;
    param[hiername][paramname]=paramval;
  }
}

map<string,string> &hier_param::get(string hiername)
{
  return param[hiername];
}


void hier_param::dump()
{
  map<string,map<string,string> >::iterator it1;
  map<string,string>::iterator it2;
  bool root;

  for (it1=param.begin(); it1!=param.end(); it1++) {
    if (it1->first=="")
      root=true;
    else
      root=false;
    for (it2=it1->second.begin(); it2!=it1->second.end(); it2++) {
      if (root)
        cout << it2->first << '=' << it2->second << endl;
      else
        cout << it1->first << '.' << it2->first << '=' << it2->second << endl;
    }
  }
}
  
int stoi(string val) {
  stringstream s(val);
  int ival;
  s >> ival;
  return ival;
}

float stof(string val) {
  stringstream s(val);
  float fval;
  s >> fval;
  return fval;
}
