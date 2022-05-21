#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <list>
#include <string>
#include <stdarg.h>

class PrettyTable
{
private:
   std::wfstream*                     OutStream;
   std::vector<std::wstring>            Headers;
   std::vector<std::vector<std::wstring>> Text;
   size_t CurrI;
   size_t CurrJ;
   

public:
   PrettyTable():OutStream(nullptr),CurrI(0),CurrJ(0) {}
   
   bool SetStream(std::wfstream* Stream);

   void SetColumns(const wchar_t* Column, ...);
   
   template <typename T>
   friend PrettyTable& operator<<(PrettyTable& in, T Param);
   



};

template <typename T>
PrettyTable& operator<<(PrettyTable& in, T Param)
{

   if (in.Headers.size() - 1 == in.CurrJ)
   {
      in.CurrI++;
      in.CurrJ = 0;
      in.Text.push_back(std::vector<std::wstring>(0));
   }
   std::wstringstream tmp;
   tmp<< Param;
   in.Text[in.CurrI].push_back(tmp.str());
   return in;
}


