/*
 * LayoutReader.hpp
 *
 * Layout formats implementation:
 * GDSII Binary - Dmitry A. Bulakh
 * MSK          - Mikhail S. Kotlyarov 
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <numeric>
#include <iostream>
#include <exception>

#include "LayoutReader.hpp"
#include "LayoutData.hpp"

class LayoutReader_MSK:public LayoutReader
{
private:
	Library*			p_activeLibrary;
	Element*			p_activeElement;
	Geometry*      p_active_geometry_item;

public:

	LayoutReader_MSK() :p_activeLibrary(nullptr), p_activeElement(nullptr), p_active_geometry_item(nullptr) {}
	~LayoutReader_MSK() { p_activeElement = nullptr; p_activeLibrary = nullptr; p_active_geometry_item = nullptr; }
	bool            IsMyFormat(const std::wstring& fName) override final;
	bool            Read(LayoutData* layout) override final;

private:

	void ReadRectangle(const std::string& FileLine);
	void ReadBoundingBox(const std::string& FileLine);
	void ReadTitle(const std::string& FileLine);

	inline bool     ReadRecCoords(const std::string& line, Coord& left_bot, Coord& right_top, std::string& layer_name);
	void            FillBox(Geometry* filling_box, const Coord& right_top, const Coord& left_bot, const uint16_t layer_num);
	inline int32_t  CalcDelta(const int32_t first, const int32_t second);
	int16_t         ConvertMskLayerNum(const std::string& layer_name);
	inline int16_t  FindLayerNum(const std::vector <Layer>& all_layers, const int16_t layer_num);
	std::string     GetElemName();

};
