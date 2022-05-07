#include "MskStaticAnalyzer.hpp"


RuleParser::RuleParser(
   const std::string& FileName)
{
   SetFileName(FileName);
}

void 
RuleParser::Reset()
{
   if (ruleFileStream) { ruleFileStream.close(); }
   if (rules.get()) { rules.reset(); }
   isParsed = false;
}


void 
RuleParser::SetFileName(
   const std::string FileName)
{
   if (!CheckFileName(FileName)) throw std::runtime_error("Invalid file name!");

   if(isParsed) 
   {
      Reset();
   }
   fileName = FileName;
}


bool 
RuleParser::ParseFile()
{
   if (fileName.empty()) { return false; }

   try
   {
      if (isParsed)
      {
         Reset();
      }

      ruleFileStream.open(fileName, std::ios::in);
      if (!ruleFileStream.is_open()) throw std::exception();

      std::string fileLine;
      while (!ruleFileStream.eof())
      {
         std::getline(ruleFileStream, fileLine);
         if (fileLine.find("NAME") != std::string::npos)
         {
            ReadSectionCommonParams();
         }

      }
   }
   catch (std::exception& ex)
   {
      std::wcerr << "\nSmth bad happened:" << ex.what();
      return false;
   }
   return true;
}


void 
RuleParser::ReadSectionCommonParams()
{
  if (!ruleFileStream.is_open()) throw std::exception();
  
  std::string fileLine;
  do
  {
     std::getline(ruleFileStream, fileLine);
  } while (fileLine.find_first_of("*") == 0);

  do
  {
     if (fileLine.find_first_of("lambda") == 0)
     {
        rules->lamda = GetNumVal(fileLine);
     }
     else if (fileLine.find_first_of("metalLayers") == 0)
     {
        rules->numMetalLayers = GetNumVal(fileLine);
     }

  } while (fileLine.find_first_of("*") != 0);

}

bool 
RuleParser::CheckFileName(
   const std::string& FileName)
{
   if (FileName.empty())
   {
      return false;
   }
   if (FileName.length() < g_RuleFileExtLowerCase.length() + 1)
   {
      return false;
   }
   if (FileName.find(g_RuleFileExtLowerCase) == std::string::npos && FileName.find(g_RuleFileExtUpperCase) == std::string::npos)
   {
      return false;
   }
   return true;
}


double 
RuleParser::GetNumVal(
   const std::string& RuleStr)
{
   if (RuleStr.empty()) throw std::runtime_error("Empty RuleStr");

   const std::vector<std::string> vecStr(RuleStr.begin(), RuleStr.end());
   bool flag = false;
   for (auto it = vecStr.begin(); it != vecStr.end(); it++)
   {
      if ("=" == *it)
      {
         it++;
         flag = true;
         return std::atof(it->c_str());
      }
   }
   if (!flag) std::runtime_error("Invalid RuleStr");
}