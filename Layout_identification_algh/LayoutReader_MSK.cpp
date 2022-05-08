/*
 * LayoutReader_MSK.cpp
 *
 * uWind (MicroWind) MSK file format reader by Mikhail S. Kotlyarov
 * 02.10.2021
 */
#pragma warning(disable : 4996)
#include "LayoutReader_MSK.hpp"
#define _CRT_SECURE_NO_WARNINGS


#include <codecvt>



constexpr int16_t undefinedValue = std::numeric_limits<int16_t>::min();


bool
LayoutReader_MSK::IsMyFormat(
   const std::wstring& Fname)
{
   bool retVal = true;
   do {
      const size_t commaPos = Fname.find_last_of(L".");
      if (commaPos == std::string::npos) { retVal = false; break; }

      const std::wstring fileExt = Fname.substr(commaPos + 1, Fname.length() - commaPos);

      if (fileExt != L"MSK" && fileExt != L"msk") { retVal = false; break; }

      file.open(Fname, std::ios::in);
      if (!file) { retVal = false; break; }
      fileName = Fname;

      std::string line;
      std::getline(file, line);
      if (line.length() < 7) { retVal = false; break; }

      if (line.substr(0, 7) != "VERSION") { retVal = false; break; }
   } while (false);

   if (file) { file.close(); }
   
   return retVal;
}


int16_t
LayoutReader_MSK::ConvertMskLayerNum(
   const std::string& LayerName)
{
    auto it = g_layerMap.find(LayerName);
    if (it == g_layerMap.end()) { return undefinedValue; }

    return it->second;
}


int16_t
LayoutReader_MSK::FindLayerNum(
   const std::vector <Layer>& Layers,
   const int16_t LayerNum)
{
    for (int16_t i = 0; i < Layers.size(); i++)
    {
        if (Layers[i].layer == LayerNum)
        {
            return i;
        }
    }
    return undefinedValue;
}


// URL: https://microeducate.tech/convert-wstring-to-string-encoded-in-utf-8/
std::string
WcsStrToUtf8(
   const std::wstring& Str)
{
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.to_bytes(Str);
}


std::string
LayoutReader_MSK::GetElemName()//elementName == FileName
{
    const std::wstring& tempFileName = fileName;
    const size_t lastCommaPos = tempFileName.find_last_of(L".");
    const size_t beginNamePos = tempFileName.find_last_of(L"/");
    if (beginNamePos == std::string::npos) { return WcsStrToUtf8(tempFileName.substr(0, lastCommaPos)); }
    else { return WcsStrToUtf8(tempFileName.substr(beginNamePos + 1, lastCommaPos - beginNamePos - 1)); }
}


bool
LayoutReader_MSK::Read(
   LayoutData* Layout)
{

   try {
      if (!Layout) throw std::invalid_argument("Layout");

      file.open(fileName);
      if (!file.is_open()) throw std::runtime_error("File was not opened");
      p_data = Layout;

      p_activeLibrary = new Library;
      p_activeElement = new Element;

      p_data->fileName = this->fileName;
      p_activeElement->name = GetElemName();
      p_activeLibrary->name = WcsStrToUtf8(fileName);

      //Переменная для хранения одной строки из файла
      std::string fileLine;
      while (std::getline(file, fileLine))
      {
         if (fileLine.find("BB") != std::string::npos)  { ReadBoundingBox(fileLine); }
         if (fileLine.find("REC") != std::string::npos) { ReadRectangle(fileLine); }
         if (fileLine.find("TITLE") != std::string::npos) { ReadTitle(fileLine); }
      }
      p_activeLibrary->elements.push_back(p_activeElement);
      p_data->libraries.push_back(p_activeLibrary);

      Layout->fileName = fileName;
      Layout->fileFormat = LayoutFileFormat::MSK;

      Layout->libraries[0]->elements[0]->min = Layout->libraries[0]->elements[0]->geometries[0]->min;
      Layout->libraries[0]->elements[0]->max = Layout->libraries[0]->elements[0]->geometries[0]->max;

      for (size_t i = 1; i < Layout->libraries[0]->elements[0]->geometries.size(); ++i) {
         if (Layout->libraries[0]->elements[0]->min.x > Layout->libraries[0]->elements[0]->geometries[i]->min.x)
            Layout->libraries[0]->elements[0]->min.x = Layout->libraries[0]->elements[0]->geometries[i]->min.x;
         if (Layout->libraries[0]->elements[0]->min.y > Layout->libraries[0]->elements[0]->geometries[i]->min.y)
            Layout->libraries[0]->elements[0]->min.y = Layout->libraries[0]->elements[0]->geometries[i]->min.y;
         if (Layout->libraries[0]->elements[0]->max.x < Layout->libraries[0]->elements[0]->geometries[i]->max.x)
            Layout->libraries[0]->elements[0]->max.x = Layout->libraries[0]->elements[0]->geometries[i]->max.x;
         if (Layout->libraries[0]->elements[0]->max.y < Layout->libraries[0]->elements[0]->geometries[i]->max.y)
            Layout->libraries[0]->elements[0]->max.y = Layout->libraries[0]->elements[0]->geometries[i]->max.y;
      }
   }
   catch (const std::exception& ex)
   {
      std::wcerr << std::endl << ex.what();

      if (file.is_open()) { file.close(); }
      if (Layout)
      {
         Layout->fileFormat = LayoutFileFormat::undefined;
         if (p_activeElement)
         {
            delete p_activeElement;
            p_activeElement = nullptr;
         }
         if (p_activeLibrary)
         {
            delete p_activeLibrary;
            p_activeLibrary = nullptr;
         }
         
      }
      return false;
   }
   if (file.is_open()) { file.close(); }
    return true; 
}


inline 
bool
LayoutReader_MSK::ReadRecCoords(
   const std::string& Line,
   Coord& LeftBot,
   Coord& RightTop,
   std::string& LayerName)
{
   char ch_layerName[8] = { '\0' };

   int32_t width = 0, height = 0;
   if (!sscanf_s(Line.c_str(), "REC(%d,%d,%d,%d,%s)", &LeftBot.x, &LeftBot.y, &width, &height, ch_layerName, 4)) { return false; }
   ch_layerName[strlen(ch_layerName) - 1] = '\0';
   LayerName = ch_layerName;
   RightTop.x = LeftBot.x + width;
   RightTop.y = LeftBot.y + height;
   return true;
}

void
LayoutReader_MSK::FillBox(Geometry* FillingBox,
   const Coord& LeftBot,
   const Coord& RightTop,
   const uint16_t LayerNum)
{
    Coord currCoord;
    int32_t dx = CalcDelta(LeftBot.x, RightTop.x);
    int32_t dy = CalcDelta(LeftBot.y, RightTop.y);

    //Left top
    currCoord.x = RightTop.x - dx;
    currCoord.y = RightTop.y;
    FillingBox->coords.push_back(currCoord);
    
    //Right top
    FillingBox->coords.push_back(RightTop);

    //Right bot
    currCoord.x = RightTop.x;
    currCoord.y = RightTop.y - dy;
    FillingBox->coords.push_back(currCoord);

    //Left bot
    currCoord.x = RightTop.x - dx;
    currCoord.y = RightTop.y - dy;
    FillingBox->coords.push_back(currCoord);

    //Left top
    currCoord.x = RightTop.x - dx;
    currCoord.y = RightTop.y;
    FillingBox->coords.push_back(currCoord);

    FillingBox->layer = LayerNum;

    FillingBox->min = FillingBox->max = FillingBox->coords[0];
    for (size_t i = 1; i < FillingBox->coords.size(); ++i) {
      if (FillingBox->min.x > FillingBox->coords[i].x)
        FillingBox->min.x = FillingBox->coords[i].x;
      if (FillingBox->min.y > FillingBox->coords[i].y)
        FillingBox->min.y = FillingBox->coords[i].y;
      if (FillingBox->max.x < FillingBox->coords[i].x)
        FillingBox->max.x = FillingBox->coords[i].x;
      if (FillingBox->max.y < FillingBox->coords[i].y)
        FillingBox->max.y = FillingBox->coords[i].y;
    }
}


void
LayoutReader_MSK::ReadRectangle(
   const std::string& FileLine)
{
   Geometry* currBox = nullptr;
   try {
      currBox = nullptr;
      Coord leftBot;
      Coord rightTop;
      std::string layerName;
      if (!ReadRecCoords(FileLine, leftBot, rightTop, layerName)) throw std::exception();

      const int16_t layer_num = ConvertMskLayerNum(layerName);
      if (undefinedValue == layer_num) throw std::exception();

      currBox = new Rectangle;
      FillBox(currBox, leftBot, rightTop, layer_num);
      if (const int32_t layerIndex = FindLayerNum(p_activeLibrary->layers, layer_num); undefinedValue == layerIndex)
      {
         Layer current_layer;
         current_layer.layer = currBox->layer;
         current_layer.name = layerName;
         current_layer.geometries.push_back(currBox);
         p_activeLibrary->layers.push_back(current_layer);
      }
      else
      {
         p_activeLibrary->layers.at(layerIndex).geometries.push_back(currBox);
      }
      p_activeElement->geometries.push_back(currBox);



   }
   catch (const std::exception& ex)
   {
      if (currBox)
      {
         delete currBox;
         currBox = nullptr;
      }
      throw std::runtime_error("Error while reading section BB");
   }
}


void LayoutReader_MSK::ReadBoundingBox(const std::string& FileLine)
{
   Geometry* boundingBox = nullptr;
   try {
      Coord leftBot;
      Coord rightTop;
      if (!sscanf(FileLine.c_str(), "BB(%d,%d,%d,%d)", &leftBot.x, &leftBot.y, &rightTop.x, &rightTop.y)) throw std::exception();
      Layer boundingBoxLayer;
      const int16_t layerNum = g_layerMap.find("BB")->second;
      boundingBoxLayer.layer = layerNum;
      boundingBoxLayer.name = "BB";
     
      boundingBox = new Rectangle;
      FillBox(boundingBox, leftBot, rightTop, layerNum);

      boundingBoxLayer.geometries.push_back(boundingBox);
      p_activeLibrary->layers.push_back(boundingBoxLayer);
      p_activeElement->geometries.push_back(boundingBox);
   }
   catch (std::exception& ex)
   {
      if (boundingBox)
      {
         delete boundingBox;
         boundingBox = nullptr;
      }
      throw std::runtime_error("Error while reading section BB");
   }
}


void
LayoutReader_MSK::ReadTitle(
   const std::string& FileLine)
{
   Geometry* text = nullptr;
   Text* p_text = nullptr;
   try
   {
      char buf[64] = { '\0' };
      Coord leftBot;

      if (!sscanf(FileLine.c_str(), "TITLE %d %d  #%s", &leftBot.x, &leftBot.y, buf)) throw std::exception();
      Geometry* text = new Text;
      const int16_t layerNum = g_layerMap.find("TITLE")->second;
      Text* p_text = static_cast<Text*>(text);
      p_text->coords.push_back(leftBot);
      p_text->layer = layerNum;
      p_text->min = p_text->max =  leftBot;
      p_text->width = strlen(buf);
      p_text->stringValue = buf;
      p_text = nullptr;


      if (const int32_t layerIndex = FindLayerNum(p_activeLibrary->layers, layerNum); undefinedValue == layerIndex)
      {
         Layer current_layer;
         current_layer.layer = text->layer;
         current_layer.name = "TITLE";
         current_layer.geometries.push_back(text);
         p_activeLibrary->layers.push_back(current_layer);
      }
      else
      {
         p_activeLibrary->layers.at(layerIndex).geometries.push_back(text);
      }
      p_activeElement->geometries.push_back(text);
   }
   catch (std::exception& ex)
   {
      if (text)
      {
         delete text;
         text = nullptr;
      }
      p_text = nullptr;
      throw std::runtime_error("Error while reading section TITLE");
   }
}


inline
int32_t
LayoutReader_MSK::CalcDelta(
   const int32_t First,
   const int32_t Second)
{
   return abs(Second - First);
}
