// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : suffix comparator class
//
//  DESCRIPTION   :	compare two suffixes of input string
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include <cstdint>
#include <string>
#include <map>
#include <algorithm>
#include <emmintrin.h>

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class suffixComparator
{
public:
	// constructor with string
	suffixComparator(const std::string& comparisonTarget);

	// default copy and assignment operator will do it

	// virtual destructor
	virtual ~suffixComparator();

	// comparison operator
	inline
	bool operator ()(const uint32_t a, const uint32_t b)
	{
		return cmpSfxSSE2(a, b);
	}

	// comparison for suffixes in homopolymer islands
	inline 
	bool operator ()(const std::pair<uint32_t, uint32_t> a, const std::pair<uint32_t, uint32_t> b)
	{
		const uint32_t skip = std::min(a.second, b.second);
		return cmpSfxSSE2(a.first + skip, b.first + skip);
	}

protected:

private:
	// methods
	// default constructor must not be used
	suffixComparator();

	// init
	void init();

	// compare two substrings for less-equal
	inline
	bool cmpSfx(const uint32_t a, const uint32_t b)
	{
		auto str_a = m_target.begin() + a;
		auto str_b = m_target.begin() + b;
		auto rearIter = a > b ? &str_a : &str_b;
		const ptrdiff_t maxIterations = std::distance(*rearIter, m_target.cend());
		for (ptrdiff_t i = 0; i < maxIterations; i++)
		{
			if ((*str_a) != (*str_b))
				break;
			str_a++;
			str_b++;
		}
		if ((*rearIter) == m_target.end())	// iterator reached end of string, prefer shorter suffix
		{
			if (a > b)
				return true;
			else
				return false;
		}
		if ((*str_a) < (*str_b))
			return true;
		else
			return false;
	}




	// compare two substrings using SSE2 intrinsics
	inline
	bool cmpSfxSSE2(const uint32_t a, const uint32_t b)
	{
		if (a > m_targetEndSSE || b > m_targetEndSSE)
			return cmpSfx(a, b);
		auto pa = (m_target.begin() + a);
		auto pb = (m_target.begin() + b);
		const uint32_t vectorIterations = (m_target.size() - std::max(a, b)) / 16;
		for (uint32_t i = 0; i < vectorIterations; i++)
		{
			// load 16 characters from each sequence (unaligned)
			__m128i va = _mm_loadu_si128((__m128i*)&*pa);
			__m128i vb = _mm_loadu_si128((__m128i*)&*pb);
			// compare for equality
			__m128i eq = _mm_cmpeq_epi8(va, vb);
			int eqMask = _mm_movemask_epi8(eq);
			if (eqMask == 0xFFFF)
			{
				pa += 16;
				pb += 16;
				continue;
			}
			// compare for less-equal
			__m128i lt = _mm_cmplt_epi8(va, vb);
			// one bit per comparison in lower half of masks
			int ltMask = _mm_movemask_epi8(lt);
			// check if least significant bit set in unequal mask
			// is set for less then (return true)
			// or set for greater then (return false)
			if ((ltMask & ~(ltMask - 1)) == (~eqMask & ~(~eqMask - 1)))
				return true;
			else
				return false;
		}
		return cmpSfx(a + vectorIterations * 16, b + vectorIterations * 16);
	}



	// member
	const std::string& m_target;
	uint32_t m_targetEndSSE = 0;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------