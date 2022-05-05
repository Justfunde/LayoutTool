#define _CRT_SECURE_NO_WARNINGS
#include "Layout_comparator.h"
#include "LayoutReader_MSK.hpp"
#include <iostream>


int main(int argc, char* argv[]) {
  
   std::cout << char(0);
    LayoutData  layoutFirst,layoutSecond;
    //freopen("before.txt", "w", stdout);
    //std::wstring fileName = L"tests/inv.gds";
    //std::wstring fileName = L"output.gds";
    //std::wstring fileName = L"tests/nand2.gds";
    //std::wstring fileName = L"tests/xor.gds";
    //std::wstring fileName = L"tests/1Kpolyg.gds";
    //std::wstring fileName = L"tests/testDesign.gds";
    std::wstring fileNameFirst = L"C:/microwind3 lite/carryCellFirst.MSK";

    LayoutReader* p_readerFirst = GetReader(fileNameFirst);
    if (!p_readerFirst) {
        std::wcerr << "__err__ : Can't file appropriate reader for given file '" << fileNameFirst << "'." << std::endl;
        return EXIT_FAILURE;
    }
    if (!p_readerFirst->Read(&layoutFirst)) {
        std::wcerr << "__err__ : Can't read file '" << fileNameFirst << "'." << std::endl;
        FreeReader(p_readerFirst);
        return EXIT_FAILURE;
    }
    FreeReader(p_readerFirst);
    

   // std::wcout << "Input file " << fileNameFirst << " has " << layoutFirst.libraries.size() << " library(ies)" << std::endl;
   //
   // for (size_t i = 0; i < layoutFirst.libraries.size(); ++i) {
   //    std::cout << "  - Library [" << i << "] has name '" << layoutFirst.libraries[i]->name << "' and contains " << layoutFirst.libraries[i]->elements.size() << " elements:" << std::endl;
   //    for (size_t j = 0; j < layoutFirst.libraries[i]->elements.size(); ++j)
   //        std::cout << "      * " << layoutFirst.libraries[i]->elements[j]->name << " (contains " << layoutFirst.libraries[i]->elements[j]->geometries.size() << " geometries)" << std::endl;
   //    std::cout << "    Library [" << i << "] also contains " << layoutFirst.libraries[i]->layers.size() << " layers (in order of appearance):" << std::endl;
   //    std::cout << "      { ";
   //    for (size_t j = 0; j < layoutFirst.libraries[i]->layers.size(); ++j)
   //        std::cout << layoutFirst.libraries[i]->layers[j].layer << " ";
   //    std::cout << " }" << std::endl;
   //}

      std::wstring fileNameSecond = L"C:/microwind3 lite/carryCellSecond.MSK";
      
      LayoutReader* p_readerSecond = GetReader(fileNameSecond);
      if (!p_readerSecond) {
         std::wcerr << "__err__ : Can't file appropriate reader for given file '" << fileNameSecond << "'." << std::endl;
         return EXIT_FAILURE;
      }
      if (!p_readerSecond->Read(&layoutSecond)) {
         std::wcerr << "__err__ : Can't read file '" << fileNameSecond << "'." << std::endl;
         FreeReader(p_readerSecond);
         return EXIT_FAILURE;
      }
      FreeReader(p_readerSecond);

    LayoutBitmapGenerator generatorFirst,generatorSecond;
    Coord leftTop{48,150}, rightBot{108,94};
    Coord min{ 48,94 }, max(105, 150);
    std::vector<int16_t> layers{23,27,34,36,53,55};
   
    generatorFirst.init(&layoutFirst, min, max, layers);
    generatorFirst.process(1028, 1028);
    std::cout << "\n\n\n";
    generatorSecond.init(&layoutSecond, min, max, layers);
    generatorSecond.process(1028, 1028);
    Layout_comparator comp;
    comp.setMatricies(&generatorFirst, &generatorSecond);
    std::cout << "\n\n\n";
    comp.compare();
    comp.writeDiffFile("D:\\mskDiff.txt");

    return 0;
}