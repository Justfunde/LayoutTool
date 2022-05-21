#define _CRT_SECURE_NO_WARNINGS
//#include "Layout_comparator.h"
//#include "LayoutReader_MSK.hpp"
//#include "MskStaticAnalyzer.hpp"
//#include <iostream>

#include "NewPrettyTable.hpp"

int main(int argc, char* argv[]) {
  

   PrettyTable table;
   table.SetColumns(L"First", L"Second", L"Third",nullptr);

   std::wstring value = L"test";
   table << 1 << L"2"<<value;

   //MskStaticAnalyzer analyzer;
   //RuleParser parser("tests/cmos012.rul");
   //parser.ParseFile();
   //LayoutData  layoutFirst;
   // //freopen("before.txt", "w", stdout);
   // //std::wstring FileName = L"tests/inv.gds";
   // //std::wstring FileName = L"output.gds";
   // //std::wstring FileName = L"tests/nand2.gds";
   // //std::wstring FileName = L"tests/xor.gds";
   // //std::wstring FileName = L"tests/1Kpolyg.gds";
   // //std::wstring FileName = L"tests/testDesign.gds";
   // std::wstring fileNameFirst = L"tests/Kotlyarov_LR1.MSK";

   // LayoutReader* p_readerFirst = GetReader(fileNameFirst);
   // if (!p_readerFirst) {
   //     std::wcerr << "__err__ : Can't file appropriate reader for given file '" << fileNameFirst << "'." << std::endl;
   //     return EXIT_FAILURE;
   // }
   // if (!p_readerFirst->Read(&layoutFirst)) {
   //     std::wcerr << "__err__ : Can't read file '" << fileNameFirst << "'." << std::endl;
   //     FreeReader(p_readerFirst);
   //     return EXIT_FAILURE;
   // }
   // FreeReader(p_readerFirst);
   // analyzer.SetParameters(&layoutFirst, parser.GetRules());
   // analyzer.WriteAnalyzedFile("tests/testAnalyzerFile.txt");

   //// std::wcout << "Input file " << fileNameFirst << " has " << layoutFirst.libraries.size() << " library(ies)" << std::endl;
   ////
   //// for (size_t i = 0; i < layoutFirst.libraries.size(); ++i) {
   ////    std::cout << "  - Library [" << i << "] has name '" << layoutFirst.libraries[i]->name << "' and contains " << layoutFirst.libraries[i]->elements.size() << " elements:" << std::endl;
   ////    for (size_t j = 0; j < layoutFirst.libraries[i]->elements.size(); ++j)
   ////        std::cout << "      * " << layoutFirst.libraries[i]->elements[j]->name << " (contains " << layoutFirst.libraries[i]->elements[j]->geometries.size() << " geometries)" << std::endl;
   ////    std::cout << "    Library [" << i << "] also contains " << layoutFirst.libraries[i]->layers.size() << " layers (in order of appearance):" << std::endl;
   ////    std::cout << "      { ";
   ////    for (size_t j = 0; j < layoutFirst.libraries[i]->layers.size(); ++j)
   ////        std::cout << layoutFirst.libraries[i]->layers[j].layer << " ";
   ////    std::cout << " }" << std::endl;
   ////}



    return 0;
}