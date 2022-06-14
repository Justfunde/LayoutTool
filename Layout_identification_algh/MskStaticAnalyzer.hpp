#pragma once
#include "LayoutReader_MSK.hpp"
#include <fstream>
#include <memory>
#include <sstream>
#include <ctime>

constexpr std::string_view g_RuleFileExtLowerCase = ".rul";
constexpr std::string_view g_RuleFileExtUpperCase = ".RUL";

constexpr uint16_t g_littleEndian = 0xFFFE;///<Первые два байта в файле при прямом порядке байт в UTF-16

struct MskRules
{
   std::string RuleFileName;
   double Lamda;
   uint32_t NumMetalLayers;
};

class RuleParser
{
private:
   std::string                 FileName;
   std::fstream                RuleFileStream;
   std::shared_ptr<MskRules>   Rules;
   bool                        IsParsed;
public:
   RuleParser():Rules(new MskRules),IsParsed(false) {};
   RuleParser(const std::string& FileName);
   ~RuleParser() {};

   void SetFileName(const std::string FileName);
   std::shared_ptr<MskRules> GetRules();

   bool ParseFile();
   void Reset();
private:
   void ReadSectionCommonParams();
   bool CheckFileName(const std::string& FileName);
   double GetNumVal(const std::string& RuleStr);
};

class MskStaticAnalyzer
{
private:
   std::string                ResFname;
   LayoutData*                MskData;
   std::shared_ptr<MskRules>  Rules;
   std::wfstream              File;
public:
   MskStaticAnalyzer() :MskData(nullptr) {}
   MskStaticAnalyzer(LayoutData* Data, std::shared_ptr<MskRules> Rules);
   ~MskStaticAnalyzer();

   void SetParameters(LayoutData* Data, std::shared_ptr<MskRules> Rules);
   bool WriteAnalyzedFile(const std::string& OutFname);
private:
   void Reset();
   std::wstring getTimeInfo();
   //main sections
   inline void WriteEndian();
   inline void WriteCommonInfo();
   inline void WriteCellInfo();

   //cell info
   inline void WriteNwellInfo();
   inline void WriteVddVssInfo();
   inline void WriteMpInfo();
   inline void WriteMnInfo();

   std::list<Geometry*> FindPolyGates(Geometry* Obj);


   std::vector<Layer>::const_iterator FindLayer(const std::string& LayerName) const;
   Geometry* FindTitleIntersection(const std::string& TitleName, const std::string& LayerName);
   bool IsIntersected( Geometry* First, Geometry* Second);
   Geometry* FindTitleByName(const std::string& Name);
};
