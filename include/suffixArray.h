// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : suffix array construction
//
//  DESCRIPTION   :	methods to compute suffix array of input string
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
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <cstdint>

// -- forward declarations -----------------------------------------------
struct saBlockDefinition;

// -- exported constants, types, classes ---------------------------------
class suffixArray
{
public:
	// constructor
	suffixArray(std::string& S, uint32_t t);

	// virtual destructor
	virtual ~suffixArray();

	// get alphabet
	std::string getAlphabet(void);

	// get number of characters for each letter in alphabet
	std::vector<uint32_t> getAlphabetCount(void);

	// prepare for streaming suffix array
	uint32_t prepareForStream(uint32_t maxBlockSize);

	// get next Segment from suffix array stream
	std::vector<uint32_t> getNextSegment(void);

protected:

private:
	// methods
	// Copy constructor must not be used
	suffixArray(const suffixArray& object);

	// Assignment operator must not be used
	const suffixArray& operator=(const suffixArray& rhs);

	// get histogram of kmers in S
	std::map<std::string, uint32_t> histogram(uint32_t kmerLength);

	// histogram for segment
	void histogramSegment(std::map<std::string, uint32_t>& hist,
						  std::string::iterator begin, std::string::iterator end,
						  const uint32_t l);

	// get indices of prefix range
	void suffixIndices(std::string pStart, std::string pStop, uint32_t length);

	// get indices of prefix range
	void suffixIndicesSegment(std::string lowerBound, std::string upperBound, 
							  std::string::iterator begin,
							  const std::string::iterator end);

	// sort current block of suffix indices
	void sortCurrentBlock(void);

	// sorting worker function
	void sortCurrentBlockWorker(uint32_t id);

	// member
	uint32_t m_threads = 1;
	std::mutex m_mutex;
	std::string& m_S;
	std::string m_alphabet = "";
	std::vector<uint32_t> m_alphabetCount;
	std::map<std::string, uint32_t> m_hist;
	std::list<saBlockDefinition> m_blockDefinitions;
	std::map<std::string, std::vector<uint32_t>> m_currentBlock;
	std::map<std::string, std::vector<uint32_t>>::iterator m_currentSubBlock;
};
// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------