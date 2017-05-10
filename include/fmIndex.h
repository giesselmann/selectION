// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class FM-Index
//
//  DESCRIPTION   :	FM-Index
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
#include <vector>
#include <map>
#include <thread>
#include <condition_variable>
#include <H5Cpp.h>
#include "fmIndex_settings.h"

// -- forward declarations -----------------------------------------------
struct indexValuePair;

// -- exported constants, types, classes ---------------------------------
typedef struct lcsDefinition
{
	uint32_t indexStart = 0;		// offset of match in index
	uint32_t strStart = 0;			// offset of match in pattern
	uint32_t lcsLength = 0;			// length of match
	bool errorFlag = false;
}lcsDefinition;


class fmIndex
{
public:
	// constructor loading index from disk
	fmIndex(fmIndex_settings& settings, std::string indexFileName);

	// virtual destructor
	virtual ~fmIndex();

	// allocate memory of input string
	static void allocateMemory(std::string& str, uint32_t expectedSize);

	// build index
	static void build(fmIndex_settings& settings, std::string& str, std::string outputFilename);

	// build index using chapters
	static void build(fmIndex_settings& settings, std::string& str, std::string outputFilename, 
					  std::map<uint32_t, std::string> chapters);

	// load existing index from disk
	void load(std::string indexFile);

	// return complete index sequence
	std::string getIndexSequence(void);

	// get substr of index sequence
	std::string getIndexSequence(uint32_t start, uint32_t length);

	// return vector containing all occurences of pattern in index
	std::vector<uint32_t> getMatchingPositions(std::string const & pattern);

	// limit matching positions for performance reasons
	std::vector<uint32_t> getMatchingPositions(std::string const & pattern, const uint32_t maxResults);

	// return positions of longest common substring of pattern and index
	std::vector<lcsDefinition> getLongestCommonSubsequence(std::string const & pattern);

	// return name and relative position in chapter
	std::pair<std::string, uint32_t> getRelativePosition(uint32_t absolutPosition);

	// optionally return names and lengths of chapters in order
	std::vector<std::pair<std::string, uint32_t>> getChapters(void);

protected:

private:
	// methods
	// default constructor for private use only
	fmIndex();

	// Copy constructor must not be used
	fmIndex(const fmIndex& object);

	// Assignment operator must not be used
	const fmIndex& operator=(const fmIndex& rhs);

	// init
	void init();

	// build index
	void buildIndex(std::string& str, std::string outputFilename);

	// output file writer
	void fileWriter(std::string outputFilename);

	// return row index for character of given rank
	inline
	uint32_t getRowFromRank(const char chr, const uint32_t rank)
	{
		const uint8_t charIndex = m_charIndex[chr];
		uint32_t index = 1;
		for (uint32_t i = 0; i < charIndex; i++)
			index += m_bwtFirst[i];
		return index + rank;
	}

	// get count of character up to row
	inline
	uint32_t getCount(const char chr, const uint32_t row)
	{
		// determine closest checkpoint
		const uint32_t TallyStepSize = m_settings.m_tallyStepSize;
		const uint32_t tallyRow = row / TallyStepSize;
		std::vector<uint32_t> const & col = m_tally[m_charIndex[chr]];
		uint32_t rank = col[tallyRow];
		// loop over bwt segment for final count
		for (uint32_t i = row - row % TallyStepSize + 1; i <= row; ++i)
		{
			if (m_bwtLast[i] == chr)
				rank++;
		}
		return rank;
	}

	// get position from suffix array sample
	int64_t getPositionFromRow(uint32_t row);

	// process chunk of suffix array
	void processChunk(std::string const & s, std::vector<uint32_t> const & sfx, const uint32_t offset);

	// init file datatype for tally
	void initTallyDatatype(std::string alphabet);

	// clear all members
	void clearDataStructures();

	// member
	// settings
	fmIndex_settings m_settings;
	// write thread activation and flow control
	bool m_writeActive = false;
	bool m_writeBlock = false;
	std::mutex m_writeMutex;
	std::condition_variable m_writeCondition;
	// definitions of h5 datatypes
	std::map<std::string, H5::DataType> m_fileDataTypes;
	// length of input string
	uint32_t m_N = 0;
	// alphabet of input string
	std::string m_alphabet;
	// names and offsets of subsequences 
	std::map<uint32_t, std::string>  m_nameOffset;
	// index lookup for char in bwt
	std::vector<uint8_t> m_charIndex;
	// first column of bwt matrix (elements per character)
	std::vector<uint32_t> m_bwtFirst;
	// last column of bwt matrix
	std::string m_bwtLast;
	// ranks of characters in last column
	std::vector<std::vector<uint32_t>> m_tally;
	// suffix array sample
	std::vector<indexValuePair> m_suffixArraySample;
	// complete array for debugging
	std::vector<uint32_t> m_suffixArray;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------