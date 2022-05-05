#include "LayoutReader.hpp"

#include "LayoutReader_MSK.hpp"


LayoutReader*
GetReader(
   const std::wstring &fName)
{
  LayoutReader* p_reader = nullptr;

  p_reader = new LayoutReader_MSK;
  if (p_reader->IsMyFormat(fName))
    return p_reader;

  delete p_reader;
  p_reader = nullptr;

  return p_reader;
}

void
FreeReader(
   LayoutReader *reader) 
{
  if (!reader)
    return;
  delete reader;
  reader = nullptr;
}
