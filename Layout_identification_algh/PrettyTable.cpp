#include "PrettyTable.hpp"
#include <stdexcept>
#include <iomanip>
#include <stdexcept>

namespace bprinter {
 

   size_t TablePrinter::GetColumnCount() const { return Headers.size(); }


   size_t TablePrinter::GetTableWidth() const { return TableWidth; }


   void TablePrinter::SetSeparator(const std::wstring& Separator) { SeparatorSymbols = Separator; }


   void TablePrinter::SetFflushLeft() { FflushLeft = true; }


   void TablePrinter::SetFflushRigth() { FflushLeft = false; }


   void TablePrinter::AddColumn(const std::wstring& HeaderName, size_t ColumnWidth = 0) {
     
      if (HeaderName.size() > ColumnWidth) { ColumnWidth = HeaderName.size(); }

      Headers.push_back(HeaderName);
      ColumnWidthArr.push_back(ColumnWidth);
      TableWidth += ColumnWidth + SeparatorSymbols.size(); // for the separator  
   }


   void TablePrinter::PrintHorizontalLine() {
      OutStream << L"+"; // the left bar

      for (size_t i = 0; i < TableWidth - 1; i++)
      {
         OutStream << L"-";
      }

      OutStream << L"+"; // the right bar
      OutStream << L"\n";
   }


   void TablePrinter::PrintHeader() {
      PrintHorizontalLine();
      OutStream << L"|";

      for (size_t i = 0; i < Headers.size(); i++) 
      {

         if (FflushLeft) { OutStream << std::left; }
         else { OutStream << std::right; }

         OutStream << std::setw(ColumnWidthArr.at(i)) << Headers[i].substr(0, ColumnWidthArr.at(i));
         if (i != Headers.size() - 1) 
         {
            OutStream << SeparatorSymbols;
         }
      }

      OutStream << L"|\n";
      PrintHorizontalLine();
   }


   void TablePrinter::PrintFooter() { PrintHorizontalLine(); }

  
   std::wfstream& TablePrinter::operator<<(double Val)
   {
      OutputDecimalNumber<double>(Val);
      return OutStream;
   }
}
