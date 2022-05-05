#include "LayoutMatrix.h"

//CoordinateWorkspace

bool WorkspaceCoords::setAngleCoords(const Coord& leftTop, const Coord& rightBot)
{
	if (leftTop.x > rightBot.x || leftTop.y < rightBot.y)
		return false;
	this->leftTop = leftTop;
	this->rightBot = rightBot;
	return true;
}

bool WorkspaceCoords::isInWorkspace(int32_t x, int32_t y)
{
	if (x >leftTop.x && x < rightBot.x)
		if (y < leftTop.y && y > rightBot.y)
			return true;
	return false;
}


//LayoutMatrix

//Constrictors

LayoutMatrix::LayoutMatrix(const std::string& in_hash)
{
	try {
		hash = in_hash;
		RLE_decode(Base64_decode(in_hash));
	}
	catch (const std::exception& exception)
	{
		std::cerr << "\nLayoutMatrix constructor error:" << exception.what();
	}
}

LayoutMatrix::LayoutMatrix(const LayoutMatrix& matrix) :BitMatrix(matrix)
{
	hash = matrix.hash;
}

LayoutMatrix::LayoutMatrix(LayoutMatrix&& matrix) noexcept
{
	cols_count = matrix.cols_count;
	rows_count = matrix.rows_count;
	i_size = matrix.i_size;
	j_size = matrix.j_size;
	is_alloc = matrix.is_alloc;
	bitmap = matrix.bitmap;
	bit_count = matrix.bit_count;
	matrix.bitmap = nullptr;
	hash = std::move(matrix.hash);
}

LayoutMatrix::LayoutMatrix(BitMatrix&& matrix) noexcept
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


//Operators

LayoutMatrix& LayoutMatrix::operator=(const BitMatrix& matrix)
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
	catch (std::exception& exception)
	{
		std::cerr << "\nLayoutMatrix operator= error:" << exception.what();
		reset();
	}
	return *this;
}

LayoutMatrix& LayoutMatrix::operator=(const LayoutMatrix& matrix)
{
	if (!matrix.is_alloc)
		return *this;
	if (&matrix == this)
		return *this;
	try {
	reset();
	hash = matrix.hash;
	rows_count = matrix.rows_count;
	cols_count = matrix.cols_count;
	i_size = matrix.i_size;
	j_size = matrix.j_size;
	bit_count = matrix.bit_count;
	
		allocMem();
		cpyBitmap(bitmap, matrix.bitmap, cols_count, rows_count);
	}
	catch (std::exception& exception)
	{
		std::cerr << "\nLayoutMatrix operator= error:" << exception.what();
		reset();
		hash.clear();
	}
	return *this;
}
	
LayoutMatrix& LayoutMatrix::operator=(BitMatrix&& matrix) noexcept
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

LayoutMatrix& LayoutMatrix::operator=(LayoutMatrix&& matrix) noexcept
{
	reset();
	hash = std::move(matrix.hash);
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


BitMatrix LayoutMatrix::getMatrix() const
{
	return static_cast<BitMatrix>(*this);
}

std::string LayoutMatrix::getHash() const
{
	return hash;
}

void LayoutMatrix::setMatrix(const BitMatrix& matrix)
{
	*this = matrix;
	hash.clear();
}

void LayoutMatrix::setHash(const std::string& hash)
{
	this->hash = hash;
	reset();
}



std::string LayoutMatrix::encodeHash()
{
	auto begin = std::chrono::steady_clock::now();
	if (!(*this)) return std::string();
	std::pair <std::string, double> RLE = RLE_encode();

	if (RLE.second < 1) hash = Base64_encode(getString(), 0);
	else hash = Base64_encode(RLE.first, 1);
	auto end = std::chrono::steady_clock::now();
	std::cout << "\nENCODING TIME:" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "microsecs" << std::endl;
	return hash;
}

BitMatrix LayoutMatrix::decodeHash()
{
	auto begin = std::chrono::steady_clock::now();
	RLE_decode(Base64_decode(hash));
	auto end = std::chrono::steady_clock::now();
	std::cout << "\nDECODING TIME:" << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "microsecs" << std::endl;
	return static_cast<BitMatrix>(*this);
}

std::pair<std::string,double> LayoutMatrix::RLE_encode()
{
	assert(i_size != 0 && j_size != 0);
	std::string RLE_encoded;
	bool curr_value = get(0, 0);
	char key = 0;
	for (size_t i = 0; i < i_size; i++)
	{
		for (size_t j = 0; j < j_size; j++)
		{
			if (curr_value == get(i, j))
			{
				key++;
			}
			else {
				RLE_encoded += unsafeWriteLastBit(key, curr_value);
				key = 1;
				curr_value = !curr_value;
			}
			if (key == 0b01111111)
			{
				RLE_encoded += unsafeWriteLastBit(key, curr_value);
				key = 0;
			}
		}
	}

	RLE_encoded += unsafeWriteLastBit(key, curr_value);
	double compression_coeff = static_cast<double>(cols_count * rows_count) / static_cast<double>(RLE_encoded.length()) ;
	return std::make_pair(RLE_encoded, compression_coeff);
}

std::string LayoutMatrix::Base64_encode(const std::string& str,bool RLE)
{
	if (str.empty()) throw std::runtime_error("String for Base64_encode is empty!");
	std::string is_RLE;
	if (RLE)
		is_RLE = "RLE";
	return base64_encode(str + std::string(file_section_param) + std::string(file_i_size_param) + std::to_string(i_size) + std::string(file_j_size_param) + std::to_string(j_size) + std::string(file_encode_param) + is_RLE);
}



char LayoutMatrix::unsafeWriteLastBit(char byte, bool value)
{
	switch (value)
	{
	case true:
		return byte | BIT(7);
	case false:
		return byte & ~BIT(7);
	}
}

std::string LayoutMatrix::Base64_decode(const std::string& str)
{
	std::string encoded_str = base64_decode(str);
	if(encoded_str.empty()) throw std::runtime_error("String for Base64_decode is empty!");
	const std::string_view parameters = std::string_view(encoded_str).substr(encoded_str.find(file_section_param), encoded_str.length() - encoded_str.find(file_section_param));
	const std::string_view i_size = parameters.substr(parameters.find(file_i_size_param) + file_i_size_param.length(), parameters.find(file_j_size_param)- parameters.find(file_i_size_param) - file_i_size_param.length());
	const std::string_view j_size = parameters.substr(parameters.find(file_j_size_param) + file_j_size_param.length(), parameters.find(file_encode_param)- parameters.find(file_j_size_param) - file_j_size_param.length());

	resize(std::stoull(std::string(i_size)), std::stoull(std::string(j_size)));

	if (parameters.find("RLE") == std::string::npos)
	{
		const std::string_view str_matr = std::string_view(encoded_str).substr(0, encoded_str.find(file_i_size_param));
		std::string_view::const_iterator it = str_matr.begin();
		for (size_t i = 0; i < rows_count; i++)
			for (size_t j = 0; j < cols_count; j++)
			{
				bitmap[i][j] = *it;
				it++;
			}
		return std::string();
	}
	std::string tmp = encoded_str.substr(0, encoded_str.find(parameters));
	return tmp;
}

void LayoutMatrix::RLE_decode(const std::string& str)
{
	if (str.empty())
		return;
	size_t j = 0, k = 0;
	for (size_t i = 0; i < str.length(); i++)
	{
		char tmp = str[i];
		bool value = (tmp & BIT(7)) >> 7;
		tmp = unsafeWriteLastBit(tmp, 0);

		while (tmp != 0)
		{
			set(k, j, value);
			j++;
			if (j == j_size)
			{
				k++;
				j = 0;
			}
			tmp--;
		}
	}
}