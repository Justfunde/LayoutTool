#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <list>
#include <memory>
#include <fstream>
#include "LayoutData.hpp"
#include "LayoutMatrix.h"
#include <stdio.h>
#include <sstream>
#include <chrono>


constexpr double eps = 1e-10;

struct Indicies {
	size_t iBegin    = 0;
	size_t iEnd      = 0;
	size_t jBegin    = 0;
	size_t jEnd      = 0;

	static Indicies normIndicies(double iBegin, double iEnd, double dy, double jBegin, double jEnd, double dx, const Indicies& boundIndicies);
};

struct LayoutGeometries {

	enum class LineOrientation {
		undefined = 0,
		horizontal,
		vertical
	};

	enum class PathElemPos
	{
		undefuned = 0,
		begin,
		mid,
		end
	};

	std::list <Geometry*>                    dataGeometries;
	std::list <std::shared_ptr<Geometry>>    selfGeneratedGeometries;

	LayoutGeometries() {};
	~LayoutGeometries() { reset(); }
	
	inline void pushSelfGeneratedGeometry(const std::shared_ptr<Geometry>& item)
	{
		selfGeneratedGeometries.push_back(item);
	}

	inline void pushPrealoadedGeometry(Geometry* item)
	{
		if (!item)
			return;
		dataGeometries.push_back(item);
	}

	inline auto deleteDataGeometries(std::list<Geometry*>::iterator it)
	{
		*it = nullptr;
		return dataGeometries.erase(it);
	}

	inline auto deletesSelfGenGeometry(std::list<std::shared_ptr<Geometry>>::iterator it)
	{
		return selfGeneratedGeometries.erase(it);
	}

	static LineOrientation getOrientation(const Coord& first, const Coord& second);

	static std::pair<Coord, Coord> getRectAngles(const Coord& first, const Coord& second, const int32_t addWidth, PathElemPos& pos);
	
	static void addCoordValue(std::shared_ptr<Geometry> geom, const Coord& value);

	static std::list <std::shared_ptr<Geometry>> splitPath(Path* path);
	static std::list <std::shared_ptr<Geometry>> splitReference(Reference* reference,Coord min);

	void reset();
};

struct Fragment
{
	LayoutMatrix                   *p_matrix;
	WorkspaceCoords				    angleCoords;
	WorkspaceCoords                *p_bitmapCoords;
	LayoutGeometries    		    geometries;
	Indicies                        boundIndicies;
	double                          dx, dy;

public:
	Fragment() :p_matrix(nullptr), p_bitmapCoords(nullptr), dx(0), dy(0) {}
	~Fragment();

	void fillMatrix();
	void initIndicies(size_t i_begin, size_t i_end, size_t j_begin, size_t j_end);

	// Zonding geometry elements
	void zondRectangle(std::list<Geometry*>::const_iterator rect);
};



class LayoutBitmapGenerator {
private:

	//Must be preloaded
	LayoutData*				data;
	WorkspaceCoords	    	bitmapCoords;
	std::vector<int16_t>    layers;

	//Calculating inside generator
	LayoutMatrix            bitmap;
	Fragment**				   fragments;
	size_t	    			   fragmentsSize;
	LayoutGeometries        geometries;
	double  				      dx,dy;
	bool                    isCorrect;

	
public:
	LayoutBitmapGenerator() :data(nullptr), fragments(nullptr), fragmentsSize(2), dx(0), dy(0), isCorrect(false) {}
	~LayoutBitmapGenerator();
	
	bool init(LayoutData* data,const Coord& leftTop, const Coord& rightBot, const std::vector <int16_t>& layers);
	bool process(size_t iSize,size_t jSize);
	LayoutMatrix getMatrix() const;

private:
	void firstMatrixInit();

	//Zonding
	void zondRectangle(Rectangle* rect);
	
	//Fragment initialization
	void initFragmentsWorkspaces();
	void distributeGeometries();
	void initFragmentsIndicies();

	//Init vector of elements inside workspace
	void getLayerItems();

	void processGeometries (const std::vector<Geometry*>& source);
	inline bool GeometryWorkspaceIntersection(Geometry* item);

	//Pushing items into fragments
	bool pushRectangle(std::list<Geometry*>::const_iterator rect);
	bool pushRectangle(std::list<std::shared_ptr<Geometry>>::const_iterator rect);
	//bool push

	//utility methods
	void allocFragments();
	void reset();
	friend class Layout_comparator;
};




class Layout_comparator {
private:
	LayoutBitmapGenerator* first;
	LayoutBitmapGenerator* second;
	BitMatrix comparationResult;

public:
	Layout_comparator() :first(nullptr), second(nullptr) {};

	bool setMatricies(LayoutBitmapGenerator* first, LayoutBitmapGenerator* second)
	{
		if (!first || !second) return false;
		this->first = first;
		this->second = second;
	}
	bool writeDiffFile(const std::string& fileName)
	{
		std::ofstream fout(fileName);
		if (!fout)
			return false;

		const WorkspaceCoords& workspaceCoords = first->bitmapCoords;
		const int32_t dx = ceil(first->dx);
		const int32_t dy = ceil(first->dy);
		int32_t yError = 0;
		int32_t xError = 0;

		for (size_t i = 0; i < comparationResult.get_i_size(); i++)
		{
			for (size_t j = 0; j < comparationResult.get_j_size(); j++)
			{
				if (comparationResult.get(i, j))
				{
					Coord leftBot = { workspaceCoords.leftTop.x + first->dx * j, workspaceCoords.leftTop.y - (i+1)*first->dy };
					xError = (j) *(dx - first->dx);
					yError = (i)*(dy - first->dy);
					std::stringstream fileLine;
					fileLine << "REC(" << leftBot.x << ',' << leftBot.y << ',' << dx << ',' << dy << ',' << "OP,0" << ")" << '\n';
					fout << fileLine.str();
				}
			}
		}
		fout.close();

	}

	void compare()
	{
		if (!first || !second)
		{
			comparationResult = BitMatrix();
			return;
		}
		comparationResult = first->getMatrix() ^ second->getMatrix();
		comparationResult.print();


		LayoutMatrix firstMatr = first->getMatrix();
		firstMatr.encodeHash();
		std::string firstHash = firstMatr.getHash();

		LayoutMatrix secondMatr = second->getMatrix();
		secondMatr.encodeHash();
		std::string secondHash = secondMatr.getHash();

		std::cout << "\n\nFirst layout hash: " << firstHash<< std::endl;
		std::cout << "\nSecond layout hash: " << secondHash << std::endl;
		std::cout << "\nCompatibility chance = " << comparationResult.zeroRatio()*100<<"%";
	}

private:
	
};





inline double calcDelta(const int32_t n1, const int32_t n2, const uint32_t split_count)
{
	return fabs(n2 - n1) / split_count;
}