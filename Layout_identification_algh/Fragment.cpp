#include "Layout_comparator.h"
Fragment::~Fragment()
{
	p_matrix = nullptr;
	p_bitmapCoords = nullptr;
	//for (size_t i = 0; i < fragmentGeometries.size(); i++)
		//fragmentGeometries[i] = nullptr;
}

//public methods

void Fragment::initIndicies(size_t i_begin, size_t j_begin, size_t i_end, size_t  j_end )
{
	this->boundIndicies.iBegin = i_begin;
	this->boundIndicies.iEnd   = i_end;
	this->boundIndicies.jBegin = j_begin;
	this->boundIndicies.jEnd   = j_end;
}

void Fragment::fillMatrix()
{
	if (!p_matrix)
		return;
	dx = calcDelta(angleCoords.leftTop.x, angleCoords.rightBot.x, boundIndicies.jEnd - boundIndicies.jBegin + 1);
	dy = calcDelta(angleCoords.leftTop.y, angleCoords.rightBot.y, boundIndicies.iEnd - boundIndicies.iBegin + 1);
	//std::cout << "dx = " << dx << "\ndy = " << dy << std::endl;
	size_t i = 0;
	for (auto it = geometries.dataGeometries.begin(); it!=geometries.dataGeometries.end(); it++,i++)
	{
		//std::cout << "\nGeometry[" << i << "]:\n";
		switch ((*it)->type)
		{

		case GeometryType::polygon:
			break;
		case GeometryType::path:
			break;
		case GeometryType::rectangle:
			zondRectangle(it);
			break;
		case GeometryType::reference:
			break;
		default:
			break;
		}
	}
}

void Fragment::zondRectangle(std::list<Geometry*>::const_iterator rect)
{
	const Coord& leftTop = (*rect)->coords[0];
	const Coord& rightBot =(*rect)->coords[2];
	//printf("\ntype:rectangle\nleftTop = (%d,%d)\nrightBot = (%d,%d)\n", leftTop.x, leftTop.y, rightBot.x, rightBot.y);
	//Theoretical indicies
	double i_rect_begin = static_cast<double> (boundIndicies.iBegin);
	double i_rect_end = static_cast<double> (boundIndicies.iEnd);
	double j_rect_begin = static_cast<double> (boundIndicies.jBegin);
	double j_rect_end = static_cast<double> (boundIndicies.jEnd);
	//printf("Bound indicies:\nmin = [%d,%d]\tmax = [%d,%d]\n ",boundIndicies.iBegin, boundIndicies.jBegin, boundIndicies.iEnd, boundIndicies.jEnd);

	//checking if Rectangle lies on the fragment

	if (leftTop.x <= angleCoords.leftTop.x + dx / 2 - dx * eps && rightBot.x >= angleCoords.rightBot.x - dx / 2 + dx * eps)
		if (leftTop.y >= angleCoords.leftTop.y - dy / 2 + dy * eps && rightBot.y <= angleCoords.rightBot.y + dy / 2 - dy * eps)
		{
			//std::cout << "\nRectangle lies on the fragment\n";
			for (int32_t i = boundIndicies.iBegin; i <= boundIndicies.iEnd; i++)
				for (int32_t j = boundIndicies.jBegin; j <= boundIndicies.jEnd; j++)
				{
					try {
						p_matrix->set(i, j, 1);
					}
					catch (...)
					{
						std::cout << "err";
					}
				}
			p_matrix->print();
			return;
		}


		j_rect_begin = (leftTop.x - p_bitmapCoords->leftTop.x) / dx;

		j_rect_end = (rightBot.x - p_bitmapCoords->leftTop.x) / dx;

		i_rect_begin = (p_bitmapCoords->leftTop.y - leftTop.y) / dy;

		i_rect_end = (p_bitmapCoords->leftTop.y - rightBot.y) / dy;

	//std::cout << "\Indicies before normalization:\n";
	//printf("begin = [%.2f,%.2f]\t end = [%.2f,%.2f]\n", i_rect_begin, j_rect_begin, i_rect_end, j_rect_end);

	//std::cout << "Indicies after normalization\n";
	Indicies normalIndicies = Indicies::normIndicies(i_rect_begin, i_rect_end, dy, j_rect_begin, j_rect_end, dx, boundIndicies);
	//printf("begin = [%d,%d]\t end = [%d,%d]\n", normalIndicies.iBegin, normalIndicies.jBegin, normalIndicies.iEnd, normalIndicies.jEnd);
	
	for (size_t i = normalIndicies.iBegin; i <= normalIndicies.iEnd && i >= boundIndicies.iBegin && i <= boundIndicies.iEnd; i++)
		for (size_t j = normalIndicies.jBegin; j <= normalIndicies.jEnd && j >= boundIndicies.jBegin && j <= boundIndicies.jEnd; j++)
		{
			try {
				p_matrix->set(i, j, 1);
			}
			catch (...)
			{
				std::cout << "err";
			}
		}

	//std::cout << std::endl << std::endl;
	//p_matrix->print();
	//std::cout << std::endl << std::endl;



}