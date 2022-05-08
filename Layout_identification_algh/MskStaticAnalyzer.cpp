#include "MskStaticAnalyzer.hpp"


RuleParser::RuleParser(
   const std::string& FileName):RuleParser()
{
   try {
      SetFileName(FileName);
   }
   catch (...) 
   {
      Reset();
   }
}

void 
RuleParser::Reset()
{
   if (ruleFileStream) { ruleFileStream.close(); }
   if (rules.get()) { rules.reset(); }
   isParsed = false;
}

std::shared_ptr<MskRules>
RuleParser::GetRules()
{
   if (!isParsed)
   {
      if (!ParseFile())
      {
         return nullptr;
      }
   }
   return rules;
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
   if (isParsed) { return true; }
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
            //TODO:delete this break
            break;
         }

      }
   }
   catch (std::exception& ex)
   {
      std::wcerr << "\nSmth bad happened:" << ex.what();
      return false;
   }
   if (size_t lastSepIndex = fileName.find_last_of("/"); lastSepIndex != std::string::npos)
   {
      rules->RuleFileName = std::string(fileName.begin() + lastSepIndex + 1, fileName.end());
   }
   else 
   {
      rules->RuleFileName = fileName;
   }
   isParsed = true;
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
     if (fileLine.find("lambda") == 0)
     {
        rules->Lamda = GetNumVal(fileLine);
     }
     else if (fileLine.find("metalLayers") == 0)
     {
        rules->NumMetalLayers = GetNumVal(fileLine);
     }
     std::getline(ruleFileStream, fileLine);
  } while (fileLine.find_first_of("*") != 0);

}


bool 
RuleParser::CheckFileName (
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
   
   std::stringstream ss(RuleStr);
   std::istream_iterator<std::string> begin(ss);
   std::istream_iterator<std::string> end;
   std::vector<std::string> vecStr(begin, end);

   bool flag = false;
   for (size_t i = 0;i<vecStr.size();i++)
   {
      if (vecStr[i] == "=")
      {
         i++;
         flag = true;
         return std::atof(vecStr[i].c_str());
      }
   }
   if (!flag) throw std::runtime_error("Invalid RuleStr");
}


void 
MskStaticAnalyzer::SetParameters(
   LayoutData* Data,
   std::shared_ptr<MskRules> Rules)
{
   if (!Data) { return; }
   if (Data->fileFormat != LayoutFileFormat::MSK) { return; }
   MskData = Data;
   this->Rules = Rules;
}

MskStaticAnalyzer::MskStaticAnalyzer(
   LayoutData* Data,
   std::shared_ptr<MskRules> Rules) :MskStaticAnalyzer()
{
   SetParameters(Data, Rules);
}

MskStaticAnalyzer::~MskStaticAnalyzer()
{
   MskData = nullptr;
}

bool 
MskStaticAnalyzer::WriteAnalyzedFile(
   const std::string& OutFname)
{
   if (OutFname.empty()) { return false; }
   ResFname = OutFname;
   try
   {
      WriteEndian();
      File << L"______________________MSK_ANALYZER_RESULTS______________________\n";
      File.open(OutFname,std::ios::out | std::ios::ate);
      WriteCommonInfo();
      WriteCellInfo();
     
   }
   catch (std::exception& ex)
   {
      std::wcerr << L"\nSmth bad happened!" << ex.what();
   }
   if (File)
      File.close();
}


std::wstring
MskStaticAnalyzer::getTimeInfo()
{
   std::wstringstream timeInfo;
   time_t rawtime;
   tm rawTimeInfo;

   time(&rawtime);
   localtime_s(&rawTimeInfo,&rawtime);

   timeInfo << rawTimeInfo.tm_year + 1900 << L"-";

   if ((rawTimeInfo.tm_mon + 1) < 10) { timeInfo << L"0" << rawTimeInfo.tm_mon; }
   else { timeInfo << rawTimeInfo.tm_mon; }

   timeInfo << L"-" << rawTimeInfo.tm_mday << L"\t";

   if (rawTimeInfo.tm_hour + 1 < 10) { timeInfo << L"0" << rawTimeInfo.tm_hour; }
   else { timeInfo << rawTimeInfo.tm_hour; }
   timeInfo << L":";

   if (rawTimeInfo.tm_min+1 < 10) { timeInfo << L"0" << rawTimeInfo.tm_min; }
   else { timeInfo << rawTimeInfo.tm_min; }
   timeInfo << L":";

   if (rawTimeInfo.tm_sec < 10) { timeInfo << L"0" << rawTimeInfo.tm_sec; }
   else { timeInfo << rawTimeInfo.tm_sec; }

   return timeInfo.str();
}


inline
void
MskStaticAnalyzer::WriteEndian()
{
   File.open(ResFname, std::ios::out | std::ios::binary);
   if (!File.is_open()) throw std::runtime_error("File was not opened!");
   File << g_littleEndian;
   File.close();
}


template<class MapIter, class Value>
static
MapIter findVal(
   MapIter begin,
   MapIter end,
   Value v)
{
   while (begin != end && begin->second != v) ++begin;
   return begin;
}


inline
void
MskStaticAnalyzer::WriteCommonInfo()
{
   
   File << L"______________________SECTION_COMMON_INFORMATION______________________\n";
   File << L"Data&time information: " << getTimeInfo()<<L"\n\n\n\n";
   File << L"Layout file name : " << MskData->fileName << L"\n";
   File << L"Rule file name : " << std::wstring(Rules->RuleFileName.begin(), Rules->RuleFileName.end()) << L"\n";
   File << L"Layout information:\nlambda = " << std::to_wstring(Rules->Lamda)<<L"\n";
   File << L"Inside " << MskData->libraries[0]->elements[0]->geometries.size() << " geometries and "<<MskData->libraries[0]->layers.size() <<" layers\n";
   File << L"Geometry layer map:\nFormat: LayerName--->LayerNum\tGeometryCount\n";
   
   for (auto mapIter = MskData->libraries[0]->layers.begin(); mapIter != MskData->libraries[0]->layers.end(); mapIter++)
   {
      std::unordered_map<std::string, int16_t>::const_iterator valIter = findVal(g_layerMap.begin(), g_layerMap.end(), mapIter->layer);
      if (MskData->libraries[0]->layers.end() == mapIter) throw std::runtime_error("invalid layer number (processing common section information");
      File << std::wstring(valIter->first.begin(), valIter->first.end()) << "--->" << valIter->second << "\t" << mapIter->geometries.size() << "\n";
   }
   File << L"\n\n\n";
}

inline
void
MskStaticAnalyzer::WriteCellInfo()
{
   File << L"______________________SECTION_CELL_INFORMATION______________________\n";
   WriteNwellInfo();
}


std::vector<Layer>::const_iterator
MskStaticAnalyzer::FindLayer(
   const std::string& LayerName) const
{
   
   if (LayerName.empty()) { return MskData->libraries[0]->layers.end(); }

   const auto iterForSearch = g_layerMap.find(LayerName);
   if(g_layerMap.end() == iterForSearch) { return MskData->libraries[0]->layers.end(); }

   for (std::vector<Layer>::const_iterator iter = MskData->libraries[0]->layers.begin(); iter != MskData->libraries[0]->layers.end(); iter++)
   {
      if (iter->layer == iterForSearch->second)
      {
         return iter;
      }
   }
   return MskData->libraries[0]->layers.end();
}


inline
void 
MskStaticAnalyzer::WriteNwellInfo()
{
   File << L"Nwell info:\n";
   const auto nwellIter = FindLayer("NW");
   if (MskData->libraries[0]->layers.end() == nwellIter) throw std::runtime_error("Invalid layer number (processing Nwell section information)");

   File << L"LeftTop coordinates : {" << nwellIter->geometries[0]->coords[0].x << L"," << nwellIter->geometries[0]->coords[0].y << "}\n";
   File << L"RightBot coordinates : {" << nwellIter->geometries[0]->coords[2].x << L"," << nwellIter->geometries[0]->coords[2].y << "}\n";
   File << L"Height : " << nwellIter->geometries[0]->coords[0].y - nwellIter->geometries[0]->coords[2].y<<"\n";
}

inline
void 
MskStaticAnalyzer::WriteVddVssInfo()
{

}


void 
MskStaticAnalyzer::SortGeometries()
{
}


Geometry*
MskStaticAnalyzer::FindTitleIntersection(
   const std::string& TitleName,
   const std::string& LayerName)
{   
   if (TitleName.empty() || LayerName.empty()) { return nullptr; }

   Geometry* p_TitleNameObj = FindTitleByName(TitleName);
   const auto layerIter = FindLayer(LayerName);
   if (layerIter == MskData->libraries[0]->layers.end() || nullptr == p_TitleNameObj) { return nullptr; }

   Geometry* p_resObj = nullptr;
   for (size_t i = 0; i < layerIter->geometries.size(); i++)
   {
      if (IsIntersected(p_TitleNameObj, layerIter->geometries[i]))
      {
         p_resObj = layerIter->geometries[i];
         break;
      }
   }
   return p_resObj;
}


Geometry*
MskStaticAnalyzer::FindTitleByName(
   const std::string& Name)
{
   const static auto titleLayerIter = FindLayer("TITLE");

   Geometry* p_resObj = nullptr;
   for (size_t i = 0;i<titleLayerIter->geometries.size();i++)
   {
      if (Name == static_cast<Text*>(titleLayerIter->geometries[i])->stringValue)
      {
         p_resObj = titleLayerIter->geometries[i];
         break;
      }
   }
   return p_resObj;
}

bool 
MskStaticAnalyzer::IsIntersected(
   Geometry* First,
   Geometry* Second)
{
   if (!First || !Second) { return false; }

   if (First->min.x > Second->max.x || Second->max.x < First->min.x || First->min.y>Second->max.y || Second->min.y > First->max.y)
   {
      return false;
   }
   return true;
}