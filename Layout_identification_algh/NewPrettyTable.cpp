#include "NewPrettyTable.hpp"

bool
PrettyTable::SetStream(
   std::wfstream* Stream)
{
   if (!Stream) { return false; }

   OutStream = Stream;
   return true;
}


void 
PrettyTable::SetColumns(
   const wchar_t* FirstHeader, ...)
{
   if (nullptr == FirstHeader) { return; }

   const wchar_t** strPtr = &FirstHeader;
   while (*strPtr)
   {
      if(0 == std::wcslen(*strPtr)) { break;}

      Headers.push_back(*strPtr);
      *strPtr++;
   }
   Text.push_back(std::vector<std::wstring>(0));
   *strPtr = nullptr;
   strPtr = nullptr;

}


