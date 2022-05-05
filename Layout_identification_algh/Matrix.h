#pragma once
#include <vector>
#include <iostream>
#include <cassert>
#include <random>
#include <ctime>



#define BIT(n)         (1U << (n))
#define BITS           (8U)


class BitMatrix{
protected:
	int8_t**      	            bitmap;
	uint8_t                    bit_count;
	size_t                     j_size, i_size;
	size_t                     cols_count, rows_count;
	bool                       is_alloc;
public:
	BitMatrix() :bitmap(nullptr), cols_count(0), rows_count(0), j_size(0), i_size(0), is_alloc(false), bit_count(0) {}
	BitMatrix(const size_t rows, const size_t cols);
	BitMatrix(const BitMatrix& matrix);
	BitMatrix(BitMatrix&& matrix) noexcept;
	virtual ~BitMatrix();


	bool             get(size_t i, size_t j) const;
	bool             isAllocated() const;
	size_t           get_i_size() const;
	size_t           get_j_size() const;
	std::string      getString() const;
	inline int32_t   getMapTypeByteCount() const;


	void             set(size_t i, size_t j, bool value);
	void             setRange(size_t i_start, size_t j_start, size_t i_end, size_t j_end, bool value);


	void             ones();
	void             zeros();
	void             randm();
	bool             resize(size_t rows,size_t cols);
	bool             print() const noexcept;
	double           oneRatio() const noexcept;
	double           zeroRatio() const noexcept;
	


   BitMatrix&       operator=(const BitMatrix& matrix);
	BitMatrix&       operator=(BitMatrix&& matrix) noexcept;
	bool             operator!() noexcept;

	static inline double    compare(const BitMatrix& matr1, const BitMatrix& matr2);

protected:
	//utility methods
	inline void      unsafeSetByte(size_t i, size_t j, bool value);
	inline bool      unsafeGet(size_t i, size_t j) const;
	inline void      unsafeSet(size_t i, size_t j, bool value);
	bool             cpyBitmap(int8_t** dest, int8_t** source,const size_t cols,const size_t rows);
	void             allocMem();
	void             reset();

public:
	friend const BitMatrix  operator^(const BitMatrix& matr1, const BitMatrix& matr2);
	friend bool       operator==(const BitMatrix& matr1, const BitMatrix& matr2) noexcept;
	friend class LayoutMatrix;
};