#include "Matrix.h"
BitMatrix::BitMatrix(const size_t rows, const size_t cols):i_size(rows),j_size(cols)
{
	try{
	rows_count = rows;
	cols_count = (j_size % (sizeof(int8_t) * BITS)) ? (j_size / (sizeof(int8_t) * BITS) + 1) : (j_size / (sizeof(int8_t) * BITS));
	
	allocMem();
	bit_count = sizeof(bitmap[0][0]) * BITS;
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\nBitMatrix constructor error:" << exception.what();
		reset();
	}
}

BitMatrix::BitMatrix(const BitMatrix& matrix) :is_alloc(false)
{
	try {
		rows_count = matrix.rows_count;
		cols_count = matrix.cols_count;
		i_size = matrix.i_size;
		j_size = matrix.j_size;
		bit_count = sizeof(bitmap[0][0]) * BITS;
		if (matrix.is_alloc)
			allocMem();
		bit_count = sizeof(bitmap[0][0]) * BITS;
		if (!cpyBitmap(bitmap, matrix.bitmap, cols_count, rows_count)) throw std::runtime_error("Matrix copy error");
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\nBitMatrix constructor error:" << exception.what();
		reset();
	}
}

BitMatrix::BitMatrix(BitMatrix&& matrix) noexcept
{
	cols_count = matrix.cols_count;
	rows_count = matrix.rows_count;
	i_size = matrix.i_size;
	j_size = matrix.j_size;
	is_alloc = matrix.is_alloc;
	bitmap = matrix.bitmap;
	bit_count = matrix.bit_count;
	matrix.bitmap = nullptr;
}

BitMatrix::~BitMatrix()
{
	reset();
}

//overloaded operators
BitMatrix& BitMatrix::operator=(const BitMatrix& matrix)
{
	if (!matrix.is_alloc)
		return *this;
	if (&matrix == this)
		return *this;
	try {
		reset();
		rows_count = matrix.rows_count;
		cols_count = matrix.cols_count;
		i_size = matrix.i_size;
		j_size = matrix.j_size;
		bit_count = matrix.bit_count;
		allocMem();
		cpyBitmap(bitmap, matrix.bitmap, cols_count, rows_count);
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\nBitMatrix operator= error:" << exception.what();
		reset();
	}
	return *this;
}

BitMatrix& BitMatrix::operator=(BitMatrix&& matrix) noexcept
{
	reset();
	bitmap = matrix.bitmap;
	matrix.bitmap = nullptr;
	cols_count = matrix.cols_count;
	rows_count = matrix.rows_count;
	i_size = matrix.i_size;
	j_size = matrix.j_size;
	bit_count = matrix.bit_count;
	is_alloc = matrix.is_alloc;
	return *this;
}

const BitMatrix operator^(const BitMatrix& matr1, const BitMatrix& matr2)
{
	if (!matr1.is_alloc || !matr1.bitmap) throw std::runtime_error("First argument is not allocated!");
	if (!matr2.is_alloc || !matr2.bitmap) throw std::runtime_error("Second argument is not allocated!");
	if (matr1.j_size != matr2.j_size || matr1.i_size != matr2.i_size) throw std::runtime_error("Objects have uncomparable sizes!");

	BitMatrix result(matr1.i_size, matr1.j_size);
	if (!result) throw std::runtime_error("Result XOR object was not constructed");
	for (size_t i = 0; i < matr1.get_i_size(); i++)
		for (size_t j = 0; j < matr1.get_j_size(); j++)
		{
			if (matr1.get(i, j) != matr2.get(i, j))
			{
				result.set(i, j, 1);
			}
		}
	return result;
}

bool operator==(const BitMatrix& matr1, const BitMatrix& matr2) noexcept
{
	if (BitMatrix::compare(matr1, matr2) == 1)
		return true;
	return false;
}

bool BitMatrix::operator!() noexcept
{
	if (!is_alloc||!bitmap)
		return true;
	if (i_size == 0 || j_size == 0)
		return true;
	return false;
}

//Getters

bool BitMatrix::get(size_t i, size_t j) const 
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	if (i >= i_size || j >= j_size) throw std::runtime_error("Invalid index");
	//finding byte pos in arr
	const size_t i_byte_pos = i;
	const size_t j_byte_pos = j / bit_count;
	const size_t j_addit_bit_pos = bit_count - 1 - j % bit_count;
	switch ((bitmap[i_byte_pos][j_byte_pos] & BIT(j_addit_bit_pos)) >> j_addit_bit_pos)
	{
	case 0x0:return false;
	case 0x1:return true;
	}
	return 0xFF;
}

bool BitMatrix::unsafeGet(size_t i, size_t j) const
{
	return static_cast<bool>((bitmap[i][j / bit_count] & BIT(bit_count - 1 - j % bit_count)) >> (bit_count - 1 - j % bit_count));
}

bool BitMatrix::isAllocated() const 
{ 
	return is_alloc; 
}

size_t BitMatrix::get_i_size() const
{
	return i_size;
}

size_t BitMatrix::get_j_size() const
{
	return j_size;
}

//Setters

void BitMatrix::set(size_t i, size_t j, bool value)
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	if (i >= i_size || j >= j_size) throw std::runtime_error("Invalid index");
	
	//finding byte pos in arr
	const size_t i_byte_pos = i;
	const size_t j_byte_pos = j / bit_count;
	const size_t j_addit_bit_pos = bit_count - 1 - j % bit_count;

	switch (value)
	{
	case true:
		bitmap[i_byte_pos][j_byte_pos] |= BIT(j_addit_bit_pos);
		break;
	case false:
		bitmap[i_byte_pos][j_byte_pos] &= ~BIT(j_addit_bit_pos);
		break;
	}
	
}

void BitMatrix::unsafeSet(size_t i, size_t j, bool value)
{
	switch (value)
	{
	case true:
		bitmap[i][j / bit_count] |= BIT(bit_count - 1 - j % bit_count);
		break;
	case false:
		bitmap[i][j / bit_count] &= ~BIT(bit_count - 1 - j % bit_count);
		break;
	}
}

void  BitMatrix::unsafeSetByte(size_t i, size_t j, bool value)
{
	switch (value)
	{
	case true:
		bitmap[i][j] = 0xFF;
		break;
	case false:
		bitmap[i][j] = 0;
	}
}

void BitMatrix::setRange(size_t i_start, size_t j_start, size_t i_end, size_t j_end, bool value)
{
	if (!is_alloc||!bitmap) throw std::runtime_error("Allocation error");
	if (i_start > i_end || i_end > i_size) throw std::runtime_error("Invalid index");
	if (j_start > j_end || j_end > j_size) throw std::runtime_error("Invalid index");
	
	const int64_t j_start_byte_pos = j_start / bit_count;
	const int64_t j_end_byte_pos = j_end / bit_count;

	switch ((j_end_byte_pos - j_start_byte_pos - 1) > 0)//does column range contains full byte
	{
	case true:
		for (size_t i = i_start; i < i_end; i++)
			for (size_t j = j_start_byte_pos + 1; j < j_end_byte_pos; j++)
				unsafeSetByte(i, j, value);
	
		for (size_t i = i_start; i < i_end; i++)
			for (size_t j = j_start; j < (j_start_byte_pos + 1) * bit_count; j++)
				unsafeSet(i, j, value);
		
		for (size_t i = i_start; i < i_end; i++)
			for (size_t j = j_end_byte_pos * bit_count; j < j_end; j++)
				unsafeSet(i, j, value);
		break;

	case false:
		for (size_t i = i_start; i < i_end; i++)
			for (size_t j = j_start; j < j_end; j++)
				unsafeSet(i, j, value);
		break;
	}
}


bool BitMatrix::print() const noexcept
{
	if (!is_alloc || !bitmap)
		return false;
	for (size_t i = 0; i < i_size; i++)
	{
		for (size_t j = 0; j < j_size; j++)
		{
			std::cout <<  unsafeGet(i, j);
		}
		std::cout << "\n";
	}
	return true;
}

double BitMatrix::oneRatio() const noexcept
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	double unit = 0;
	try {
		for (size_t i = 0; i < i_size; i++)
		{
			for (size_t j = 0; j < j_size; j++)
			{
				if (unsafeGet(i, j) == true)
					unit++;
			}
		}
	}
	catch (const std::exception &exception)
	{
		std::cerr << "\nUnitRatio error:" << exception.what();
		return -1;
	}

	return unit / static_cast<double>(i_size * j_size);
}

double  BitMatrix::zeroRatio() const noexcept 
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	double unit = 0;
	try {
		for (size_t i = 0; i < i_size; i++)
		{
			for (size_t j = 0; j < j_size; j++)
			{
				if (unsafeGet(i, j) == false)
					unit++;
			}
		}
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\nUnitRatio error:" << exception.what();
		return -1;
	}

	return unit / static_cast<double>(i_size * j_size);
}

inline double BitMatrix::compare(const BitMatrix& matr1, const BitMatrix& matr2)
{
	double res = 0;
	try
	{
		res = (matr1 ^ matr2).oneRatio();
		if (res == -1)
			return res;
		return 1 - res;
	}
	catch (std::exception& exception)
	{
		std::cerr << "\nCompare error:" << exception.what();
		return -1;
	}
}

void BitMatrix::ones()
{
	setRange(0, 0, i_size, j_size, 1);
}

void BitMatrix::zeros()
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	for (size_t i = 0; i < rows_count; i++)
		for (size_t j = 0; j < cols_count; j++)
			bitmap[i][j] = 0;
}

void BitMatrix::randm()
{
	if (!is_alloc || !bitmap) throw std::runtime_error("Allocation error");
	std::mt19937 engine; 
	engine.seed(std::time(nullptr));
	for (size_t i = 0; i < i_size; i++)
	{
		for (size_t j = 0; j < j_size; j++)
		{
			bool val = (engine() % 2 == 0) ? true : false;
			unsafeSet(i, j, val);
		}
	}

}

bool BitMatrix::resize(size_t rows, size_t cols)
{
	if (rows <= 0 || cols <= 0) throw std::runtime_error("Matrix resize parameters error!");
	
	reset();
	rows_count = i_size = rows;
	j_size = cols;
	cols_count = (j_size % (sizeof(int8_t) * BITS)) ? (j_size / (sizeof(int8_t) * BITS) + 1) : (j_size / (sizeof(int8_t) * BITS));
	try {
		allocMem();
		bit_count = sizeof(bitmap[0][0]) * BITS;
	}
	catch (std::bad_alloc& exception)
	{
		std::cerr << "\nStandard error:" << exception.what();
		reset();
		return false;
	}

	return true;
}


std::string BitMatrix::getString() const 
{
	if (!is_alloc || !bitmap) return std::string();
	if (i_size == 0 || j_size == 0) return std::string();
	std::string str;
	str.reserve(cols_count * rows_count);
	for (size_t i = 0; i < rows_count; i++)
		for (size_t j = 0; j < cols_count; j++)
			str += bitmap[i][j];
	return str;
}

 int32_t BitMatrix::getMapTypeByteCount() const
{
	return sizeof(bitmap[0][0]);
}

//utility methods


bool BitMatrix::cpyBitmap(int8_t** dest, int8_t** source, const size_t cols, const size_t rows)
{
	if (!dest || !source)
		return false;
	for (size_t i = 0; i < rows; i++)
		for (size_t j = 0; j < cols; j++)
			dest[i][j] = source[i][j];
	return true;
}

void BitMatrix::allocMem()
{

	bitmap = new int8_t * [rows_count];
	for (size_t i = 0; i < rows_count; i++)
	{
		bitmap[i] = new int8_t[cols_count];
	}
	for (size_t i = 0; i < rows_count; i++)
		for (size_t j = 0; j < cols_count; j++)
		{
			bitmap[i][j] = 0;
		}
	is_alloc = true;
	
}

void BitMatrix::reset()
{
	if (bitmap)
	{
		for (size_t i = 0; i < rows_count; i++)
		{
			delete[] bitmap[i];
			bitmap[i] = nullptr;
		}
		delete[] bitmap;
		
	}
	bitmap = nullptr;
	is_alloc = false;
	i_size = 0;
	j_size = 0;
	cols_count = 0;
	rows_count = 0;
	bit_count = 0;
}