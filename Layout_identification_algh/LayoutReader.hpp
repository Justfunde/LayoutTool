/*
 * LayoutReader.hpp
 *
 * Layout formats implementation:
 * GDSII Binary - Dmitry A. Bulakh
 *                see LayoutReader_GDSIIBin.hpp
 * MSK          - Mikhail S. Kotlyarov  
 *                see LayoutReader_MSK.hpp
 */
#pragma once

#include <string>
#include <fstream>

#include "LayoutData.hpp"

class LayoutReader {
protected:
  std::wstring  fileName;
  LayoutData   *p_data;
  std::ifstream file;
public:
  LayoutReader();
public:
  virtual bool IsMyFormat(const std::wstring &fName) = 0;
  virtual bool Read(LayoutData *layout) = 0;
};

LayoutReader*GetReader(const std::wstring &fName);
void FreeReader(LayoutReader*ptr);
