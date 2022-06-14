#include "MskStaticAnalyzer.hpp"
#include "PrettyTable.hpp"

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
   if (RuleFileStream) { RuleFileStream.close(); }
   if (Rules.get()) { Rules.reset(); }
   IsParsed = false;
}


std::shared_ptr<MskRules>
RuleParser::GetRules()
{
   if (!IsParsed)
   {
      if (!ParseFile()) throw std::runtime_error("File was not parsed!");
   }
   return Rules;
}


void 
RuleParser::SetFileName(
   const std::string FileName)
{
   if (!CheckFileName(FileName)) throw std::invalid_argument("Invalid file name!");

   if(IsParsed) 
   {
      Reset();
   }
   this->FileName = FileName;
}


bool 
RuleParser::ParseFile()
{
   if (FileName.empty()) { return false; }
   if (IsParsed) { return true; }

   if (IsParsed) { Reset(); }

   RuleFileStream.open(FileName, std::ios::in);
   if (!RuleFileStream.is_open()) throw std::runtime_error("Rule file was not opened!");

   std::string fileLine;
   while (!RuleFileStream.eof())
   {
      std::getline(RuleFileStream, fileLine);
      if (fileLine.find("NAME") != std::string::npos)
      {
         ReadSectionCommonParams();
         //TODO:delete this break
         break;
      }

   }

   if (size_t lastSepIndex = FileName.find_last_of("/"); lastSepIndex != std::string::npos)
   {
      Rules->RuleFileName = std::string(FileName.begin() + lastSepIndex + 1, FileName.end());
   }
   else
   {
      Rules->RuleFileName = FileName;
   }
   IsParsed = true;
   return true;
}


void 
RuleParser::ReadSectionCommonParams()
{ 
  std::string fileLine;
  do { std::getline(RuleFileStream, fileLine); } while (fileLine.find_first_of("*") == 0);

  do
  {
     if (fileLine.find("lambda") == 0)
     {
        Rules->Lamda = GetNumVal(fileLine);
     }
     else if (fileLine.find("metalLayers") == 0)
     {
        Rules->NumMetalLayers = GetNumVal(fileLine);
     }
     std::getline(RuleFileStream, fileLine);
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

   for (size_t i = 0; i < vecStr.size(); i++)
   {
      if (vecStr[i] == "=")
      {
         i++;
         return std::atof(vecStr[i].c_str());
      }
   }
   throw std::runtime_error("Invalid RuleStr");
}


void 
MskStaticAnalyzer::SetParameters(
   LayoutData* Data,
   std::shared_ptr<MskRules> Rules)
{
   if (!Data) throw std::invalid_argument("LayoutData is nullptr");
   if (Data->fileFormat != LayoutFileFormat::MSK) throw std::invalid_argument("Layout file format is not \".msk\"");
   MskData = Data;
   this->Rules = Rules;
}


MskStaticAnalyzer::MskStaticAnalyzer(
   LayoutData* Data,
   std::shared_ptr<MskRules> Rules) :MskStaticAnalyzer()
{
   try
   {
      SetParameters(Data, Rules);
   }
   catch (...)
   {
      Reset();
   }
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
      File.open(OutFname,std::ios::out | std::ios::ate);
      File << L"______________________MSK_ANALYZER_RESULTS______________________\n";
      WriteCommonInfo();
      WriteCellInfo();
     
   }
   catch (std::exception& ex)
   {
      if (File) { File.close(); }
      throw;
   }
   if (File) { File.close(); }
   return true;
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
   File << L"Data&time information: " << getTimeInfo()<<L"\n\n";
   File << L"Layout file name : " << MskData->fileName << L"\n";
   File << L"Rule file name : " << std::wstring(Rules->RuleFileName.begin(), Rules->RuleFileName.end()) << L"\n";
   File << L"Layout information:\nlambda = " << std::to_wstring(Rules->Lamda)<<L"\n";
   File << L"Inside " << MskData->libraries[0]->elements[0]->geometries.size() << " geometries and "<<MskData->libraries[0]->layers.size() <<" layers\n";


   bprinter::TablePrinter tp(File);
   tp.AddColumn(L"Layer name", 11);
   tp.AddColumn(L"Layer number", 12);
   tp.AddColumn(L"GeometryCount", 15);
   tp.PrintHeader();

   for (auto mapIter = MskData->libraries[0]->layers.begin(); mapIter != MskData->libraries[0]->layers.end(); mapIter++)
   {
      std::unordered_map<std::string, int16_t>::const_iterator valIter = findVal(g_layerMap.begin(), g_layerMap.end(), mapIter->layer);
      if (MskData->libraries[0]->layers.end() == mapIter) throw std::runtime_error("invalid layer number (processing common section information");
      tp << std::wstring(valIter->first.begin(), valIter->first.end()) << valIter->second << mapIter->geometries.size();
   }
   tp.PrintFooter();
   File << L"\n\n\n";
}


inline
void
MskStaticAnalyzer::WriteCellInfo()
{
   File << L"______________________SECTION_CELL_INFORMATION______________________\n";
   WriteNwellInfo();
   WriteVddVssInfo();
   WriteMnInfo();
   WriteMpInfo();
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
   File << L"Height : " << nwellIter->geometries[0]->coords[0].y - nwellIter->geometries[0]->coords[2].y<<"\n\n";
}


inline
void 
MskStaticAnalyzer::WriteVddVssInfo()
{
   Geometry* Vdd = nullptr;
   Geometry* Vss = nullptr;

   File << L"VDD and VSS info:\n";
   File << L"VDD:\n";

   Vdd = FindTitleIntersection("Vdd", "ME");
   if (!Vdd)
   {
      File << "Power rail has no title! Information may not be accurate!\n";
      const auto layerIter = FindLayer("ME");
      Coord coordToCheck = { std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min() };
      for (size_t i = 0; i < layerIter->geometries.size(); i++)
      {
         if (layerIter->geometries[i]->coords[0].x < coordToCheck.x && layerIter->geometries[i]->coords[0].y > coordToCheck.y)
         {
            coordToCheck.x = layerIter->geometries[i]->coords[0].x;
            coordToCheck.y = layerIter->geometries[i]->coords[0].y;
            Vdd = layerIter->geometries[i];
         }
      }
   }

   const auto nwellIter = FindLayer("NW");
   if (Vdd)
   {
      File << L"LeftTop coordinates : {" << Vdd->coords[0].x << L"," << Vdd->coords[0].y << "}\n";
      File << L"RightBot coordinates : {" << Vdd->coords[2].x << L"," << Vdd->coords[2].y << "}\n";
      File << L"Height : " << Vdd->coords[0].y - Vdd->coords[2].y << "\n";
      File << L"Weight : " << Vdd->coords[2].x - Vdd->coords[0].x << "\n";
      File << L"Overlap channel and bus:\nTheoretical : " << 2 * Rules->Lamda << "\nPractical : " << nwellIter->geometries[0]->max.y - Vdd->max.y << L"\n\n";
   }

   File << "VSS:\n";
   Vss = FindTitleIntersection("Vss", "ME");
   if (!Vss)
   {
      File << "Ground rail has no title! Information may not be accurate!\n";
      const auto layerIter = FindLayer("ME");
      Coord coordToCheck = { std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max() };
      for (size_t i = 0; i < layerIter->geometries.size(); i++)
      {
         if (layerIter->geometries[i]->coords[2].x > coordToCheck.x && layerIter->geometries[i]->coords[2].y < coordToCheck.y)
         {
            coordToCheck.x = layerIter->geometries[i]->coords[2].x;
            coordToCheck.y = layerIter->geometries[i]->coords[2].y;
            Vss = layerIter->geometries[i];
         }
      }
   }

   if (Vss)
   {
      File << L"LeftTop coordinates : {" << Vss->coords[0].x << L"," << Vss->coords[0].y << "}\n";
      File << L"RightBot coordinates : {" << Vss->coords[2].x << L"," << Vss->coords[2].y << "}\n";
      File << L"Height : " << Vss->coords[0].y - Vss->coords[2].y << "\n";
      File << L"Weight : " << Vss->coords[2].x - Vss->coords[0].x << "\n\n";
   }
   File << L"Cell Height : " << Vdd->max.y - Vss->min.y<<"\n\n\n\n";
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


std::list<Geometry*>
MskStaticAnalyzer::FindPolyGates(
   Geometry* Obj)
{
   if (nullptr == Obj) {
      throw std::runtime_error("Nullptr in FindPolyGates");}

   auto polyLayer = FindLayer("PO");
   if (MskData->libraries[0]->layers.end() == polyLayer) { throw std::runtime_error("Layout has no poly gates!"); }

   std::list<Geometry*> intersectedPolyGeometries;
   for (auto it = polyLayer->geometries.begin(); it != polyLayer->geometries.end(); it++)
   {
      if (IsIntersected(Obj, *it)) 
      {
         intersectedPolyGeometries.push_back(*it);
      }
   }
   return intersectedPolyGeometries;
}


inline 
void 
MskStaticAnalyzer::WriteMpInfo()
{
   File << L"Pmos transistors info:\n";


   bprinter::TablePrinter tp(File);
   tp.AddColumn(L"Num", 4);
   tp.AddColumn(L"Lp", 10);
   tp.AddColumn(L"Mp", 10);
   tp.PrintHeader();

   auto pDiffusions = FindLayer("DP");
   if (MskData->libraries[0]->layers.end() == pDiffusions) { throw std::runtime_error("There is no pDiffusions in layout!\n"); }

   uint32_t prev = 0;
   uint32_t curr = 1;
   for (size_t i = 0,cnt = 0;i< pDiffusions->geometries.size();i++)
   {
      auto polyGates = FindPolyGates(pDiffusions->geometries[i]);
      if (polyGates.size() == 0) { continue; }

      curr = pDiffusions->geometries[i]->max.y - pDiffusions->geometries[i]->min.y;
      if (prev == curr) { continue; }
      else { prev = curr; }
      tp << ++cnt << polyGates.front()->max.x - polyGates.front()->min.x << curr;
   }
   tp.PrintFooter();
   File << L"\n\n\n";
}

inline
void
MskStaticAnalyzer::WriteMnInfo()
{
   File << L"Nmos transistors info:\n";


   bprinter::TablePrinter tp(File);
   tp.AddColumn(L"Num", 4);
   tp.AddColumn(L"Ln", 10);
   tp.AddColumn(L"Mn", 10);
   tp.PrintHeader();

   auto nDiffusions = FindLayer("DN");
   if (MskData->libraries[0]->layers.end() == nDiffusions) { throw std::runtime_error("There is no nDiffusions in layout!\n"); }

   uint32_t prev = 0;
   uint32_t curr = 1;
   for (size_t i = 0,cnt = 0; i < nDiffusions->geometries.size(); i++)
   {
      auto polyGates = FindPolyGates(nDiffusions->geometries[i]);
      if (polyGates.size() == 0) { continue; }

      curr = nDiffusions->geometries[i]->max.y - nDiffusions->geometries[i]->min.y;
      if (prev == curr) { continue; }
      else { prev = curr; }

      tp << ++cnt << polyGates.front()->max.x - polyGates.front()->min.x << curr;
   }
   tp.PrintFooter();
   File << L"\n\n\n";
}


Geometry*
MskStaticAnalyzer::FindTitleByName(
   const std::string& Name)
{
   const static auto titleLayerIter = FindLayer("TITLE");

   Geometry* p_resObj = nullptr;
   for (size_t i = 0; i < titleLayerIter->geometries.size(); i++)
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

   if (First->min.x > Second->max.x || Second->min.x > First->max.x || First->min.y>Second->max.y || Second->min.y > First->max.y)
   {
      return false;
   }
   return true;
}


void 
MskStaticAnalyzer::Reset()
{
   if (File) { File.close(); }
   MskData = nullptr;
   Rules.reset();
}