#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <fstream>

namespace bprinter
{
   class endl {};

   class TablePrinter {
   private:
      std::wfstream& OutStream;
      std::vector<std::wstring>  Headers;
      std::vector<int32_t>       ColumnWidthArr;
      std::wstring               SeparatorSymbols;

      size_t Current_i;
      size_t Current_j;
      size_t TableWidth;
      bool FflushLeft;
   public:
      TablePrinter(std::wfstream& Stream, const std::wstring& SeparatorSymb = L"|") :OutStream(Stream), SeparatorSymbols(SeparatorSymb), Current_i(0), Current_j(0), TableWidth(0), FflushLeft(false) {}
      ~TablePrinter() { }

      size_t GetColumnCount() const;
      size_t GetTableWidth() const;
      void SetSeparator(const std::wstring& Separator);
      void SetFflushLeft();
      void SetFflushRigth();

      void AddColumn(const std::wstring& HeaderName, size_t ColumnWidth);
      void PrintHeader();
      void PrintFooter();

      TablePrinter& operator<<(endl input) {
         while (Current_j != 0) {
            *this << L"";
         }
         return *this;
      }

      std::wfstream& operator<<(double Val);

      template<typename T> TablePrinter& operator<<(T input) {
         if (Current_j == 0)
            OutStream << L"|";

         if (FflushLeft)
            OutStream << std::left;
         else
            OutStream << std::right;

         // Leave 3 extra space: One for negative sign, one for zero, one for decimal
         OutStream << std::setw(ColumnWidthArr.at(Current_j))
            << input;

         if (Current_j == GetColumnCount() - 1) {
            OutStream << L"|\n";
            Current_i = Current_i + 1;
            Current_j = 0;
         }
         else {
            OutStream << SeparatorSymbols;
            Current_j = Current_j + 1;
         }

         return *this;
      }
      template<typename T> void OutputDecimalNumber(T Value) {
         // If we cannot handle this number, indicate so
         if (Value < 10 * (ColumnWidthArr.at(Current_j) - 1) || Value > 10 * ColumnWidthArr.at(Current_j)) {
            std::wstringstream string_out;
            string_out << std::setiosflags(std::ios::fixed)
               << std::setprecision(ColumnWidthArr.at(Current_j))
               << std::setw(ColumnWidthArr.at(Current_j))
               << Value;

            std::wstring string_rep_of_number = string_out.str();

            string_rep_of_number[ColumnWidthArr.at(Current_j) - 1] = L'*';
            std::wstring string_to_print = string_rep_of_number.substr(0, ColumnWidthArr.at(Current_j));
            OutStream << string_to_print;
         }
         else {

            // determine what precision we need
            int precision = ColumnWidthArr.at(Current_j) - 1; // leave room for the decimal point
            if (Value < 0)
               --precision; // leave room for the minus sign

             // leave room for digits before the decimal?
            if (Value < -1 || Value > 1) {
               int num_digits_before_decimal = 1 + (int)log10(std::abs(Value));
               precision -= num_digits_before_decimal;
            }
            else
               precision--; // e.g. 0.12345 or -0.1234

            if (precision < 0)
               precision = 0; // don't go negative with precision

            OutStream << std::setiosflags(std::ios::fixed)
               << std::setprecision(precision)
               << std::setw(ColumnWidthArr.at(Current_j))
               << Value;
         }

         if (Current_j == GetColumnCount() - 1) {
            OutStream << L"|\n";
            Current_i = Current_i + 1;
            Current_j = 0;
         }
         else {
            OutStream << SeparatorSymbols;
            Current_j = Current_j + 1;
         }
      }
   private:
      void PrintHorizontalLine();

   };

}