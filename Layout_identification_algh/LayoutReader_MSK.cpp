/*
 * LayoutReader_MSK.cpp
 *
 * uWind (MicroWind) MSK file format reader by Mikhail S. Kotlyarov
 * 02.10.2021
 */
#pragma warning(disable : 4996)
#include "LayoutReader_MSK.hpp"
#define _CRT_SECURE_NO_WARNINGS

#include <unordered_map>
#include <codecvt>

static std::unordered_map <std::string, int16_t> all_layers =
{
    {"TITLE",-6},
    {"BB",-5},
    {"VI",-4},
    {"VI",-1},
    {"NW",1},
    {"DN",16},
    {"DP",17},
    {"PO",13},
    {"CO",19},
    {"ME",23},
    {"M2",27},
    {"M3",34},
    {"M4",36},
    {"M5",53},
    {"M6",55},
};

constexpr int16_t undefinedValue = std::numeric_limits<int16_t>::min();


bool
LayoutReader_MSK::IsMyFormat(
   const std::wstring& fName)
{
   bool retVal = true;
   do {
      size_t comma_pos = fName.find_last_of(L".");
      if (comma_pos == std::string::npos) { retVal = false; break; }

      const std::wstring file_extention = fName.substr(comma_pos + 1, fName.length() - comma_pos);

      if (file_extention != L"MSK" && file_extention != L"msk") { retVal = false; break; }

      file.open(fName, std::ios::in);
      if (!file) { retVal = false; break; }
      fileName = fName;

      std::string line;
      std::getline(file, line);
      if (line.length() < 7) { retVal = false; break; }

      if (line.substr(0, 7) != "VERSION") { retVal = false; break; }
   } while (false);

   if (file) { file.close(); }
   
   return retVal;
}


int16_t	LayoutReader_MSK::ConvertMskLayerNum(const std::string& layer_name)
{
    auto it = all_layers.find(layer_name);
    if (it == all_layers.end()) { return undefinedValue; }

    return it->second;
}


int16_t LayoutReader_MSK::FindLayerNum(const std::vector <Layer>& all_layers, const uint16_t layer_num)
{
    for (int16_t i = 0; i < all_layers.size(); i++)
    {
        if (all_layers[i].layer == layer_num)
        {
            return i;
        }
    }

    return undefinedValue;
}


// URL: https://microeducate.tech/convert-wstring-to-string-encoded-in-utf-8/
std::string WcsStrToUtf8(const std::wstring& str)
{
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.to_bytes(str);
}


std::string LayoutReader_MSK::GetElemName()//elementName == fileName
{
    const std::wstring& temp_fName = fileName;
    const size_t last_comma_pos = temp_fName.find_last_of(L".");
    const size_t start_name_pos = temp_fName.find_last_of(L"/");
    if (start_name_pos == std::string::npos) { return WcsStrToUtf8(temp_fName.substr(0, last_comma_pos)); }
    else { return WcsStrToUtf8(temp_fName.substr(start_name_pos + 1, last_comma_pos - start_name_pos - 1)); }
}


bool LayoutReader_MSK::Read(LayoutData* layout)
{

   try {
      if (!layout) throw std::invalid_argument("Layout");

      file.open(fileName);
      if (!file.is_open()) throw std::runtime_error("File was not opened");
      p_data = layout;

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

      layout->fileName = fileName;
      layout->fileFormat = LayoutFileFormat::MSK;

      layout->libraries[0]->elements[0]->min = layout->libraries[0]->elements[0]->geometries[0]->min;
      layout->libraries[0]->elements[0]->max = layout->libraries[0]->elements[0]->geometries[0]->max;

      for (size_t i = 1; i < layout->libraries[0]->elements[0]->geometries.size(); ++i) {
         if (layout->libraries[0]->elements[0]->min.x > layout->libraries[0]->elements[0]->geometries[i]->min.x)
            layout->libraries[0]->elements[0]->min.x = layout->libraries[0]->elements[0]->geometries[i]->min.x;
         if (layout->libraries[0]->elements[0]->min.y > layout->libraries[0]->elements[0]->geometries[i]->min.y)
            layout->libraries[0]->elements[0]->min.y = layout->libraries[0]->elements[0]->geometries[i]->min.y;
         if (layout->libraries[0]->elements[0]->max.x < layout->libraries[0]->elements[0]->geometries[i]->max.x)
            layout->libraries[0]->elements[0]->max.x = layout->libraries[0]->elements[0]->geometries[i]->max.x;
         if (layout->libraries[0]->elements[0]->max.y < layout->libraries[0]->elements[0]->geometries[i]->max.y)
            layout->libraries[0]->elements[0]->max.y = layout->libraries[0]->elements[0]->geometries[i]->max.y;
      }
   }
   catch (const std::exception& ex)
   {
      std::wcerr << std::endl << ex.what();

      if (file.is_open()) { file.close(); }
      if (layout)
      {
         layout->fileFormat = LayoutFileFormat::undefined;
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


inline bool LayoutReader_MSK::ReadRecCoords(const std::string& line, Coord& left_bot, Coord& right_top, std::string& layer_name)
{

   char c_layer_name[8] = { '\0' };
   int32_t width = 0, height = 0;
   if (!sscanf_s(line.c_str(), "REC(%d,%d,%d,%d,%s)", &left_bot.x, &left_bot.y, &width, &height, c_layer_name, 4)) { return false; }

   layer_name = c_layer_name;
   right_top.x = left_bot.x + width;
   right_top.y = left_bot.y + height;
   return true;
}

void LayoutReader_MSK::FillBox(Geometry* filling_box, const Coord& left_bot, const Coord& right_top, const uint16_t layer_num)
{
    Coord currCoord;
    int32_t dx = calcDelta(left_bot.x, right_top.x);
    int32_t dy = calcDelta(left_bot.y, right_top.y);

    //Left top
    currCoord.x = right_top.x - dx;
    currCoord.y = right_top.y;
    filling_box->coords.push_back(currCoord);
    
    //Right top
    filling_box->coords.push_back(right_top);

    //Right bot
    currCoord.x = right_top.x;
    currCoord.y = right_top.y - dy;
    filling_box->coords.push_back(currCoord);

    //Left bot
    currCoord.x = right_top.x - dx;
    currCoord.y = right_top.y - dy;
    filling_box->coords.push_back(currCoord);

    //Left top
    currCoord.x = right_top.x - dx;
    currCoord.y = right_top.y;
    filling_box->coords.push_back(currCoord);

    filling_box->layer = layer_num;

    filling_box->min = filling_box->max = filling_box->coords[0];
    for (size_t i = 1; i < filling_box->coords.size(); ++i) {
      if (filling_box->min.x > filling_box->coords[i].x)
        filling_box->min.x = filling_box->coords[i].x;
      if (filling_box->min.y > filling_box->coords[i].y)
        filling_box->min.y = filling_box->coords[i].y;
      if (filling_box->max.x < filling_box->coords[i].x)
        filling_box->max.x = filling_box->coords[i].x;
      if (filling_box->max.y < filling_box->coords[i].y)
        filling_box->max.y = filling_box->coords[i].y;
    }
}


void LayoutReader_MSK::ReadRectangle(const std::string& FileLine)
{
   Geometry* currBox = nullptr;
   try {
      currBox = nullptr;
      Coord leftBot;
      Coord rightTop;
      std::string layerName;
      if (!ReadRecCoords(FileLine, leftBot, rightTop, layerName)) throw std::exception();

      const int16_t layer_num = ConvertMskLayerNum(layerName);
      if (layer_num == -1) throw std::exception();

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
      int16_t layerNum = all_layers.find("BB")->second;
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


void LayoutReader_MSK::ReadTitle(const std::string& FileLine)
{
   Geometry* text = nullptr;
   Text* p_text = nullptr;
   try
   {
      char buf[64] = { '\0' };
      Coord leftBot;

      if (!sscanf(FileLine.c_str(), "TITLE %d %d  #%s", &leftBot.x, &leftBot.y, buf)) throw std::exception();
      Geometry* text = new Text;

      Text* p_text = static_cast<Text*>(text);
      p_text->coords.push_back(leftBot);
      p_text->layer = all_layers.find("TITLE")->second;
      p_text->min = p_text->max =  leftBot;
      p_text->width = strlen(buf);
      p_text->stringValue = buf;
      p_text = nullptr;
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


inline int32_t LayoutReader_MSK::calcDelta(const int32_t first, const int32_t second)
{
   return second - first;
}
