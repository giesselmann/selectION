// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <thread>

//-- private headers -----------------------------------------------------
#include "suffixArray.h"
#include "suffixComparator.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const ptrdiff_t IndexBlockSize = 1000000;		// local suffix indices buffer
const size_t BlockMergeLimit = 1024;			// merge blocks smaller than

//-- private types -------------------------------------------------------
typedef struct saBlockDefinition
{
	std::string begin;
	std::string end;
	uint32_t length;
}saBlockDefinition;


//-- private functions --------- declarations ----------------------------
bool prefixGreaterEqual(std::string::iterator firstIter, std::string::iterator secondIter,
						const std::string::iterator firstEnd, const std::string::iterator secondEnd);
static void kmerCombinations(std::string alphabet, std::string prefix,
							 std::vector<std::string>& result, int k);
bool isSingleChar(std::string str);

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
suffixArray::suffixArray
(
	std::string& S,
	uint32_t t
) :m_S(S)
{
	m_threads = t > 0 ? t : 1;
	m_currentSubBlock = m_currentBlock.begin();
	histogram(1);
	for (auto it = m_hist.begin(); it != m_hist.end(); ++it)
	{
		m_alphabet += (*it).first;
		m_alphabetCount.push_back((*it).second);
	}
}




// virtual destructor 
suffixArray::~suffixArray()
{

}




// get alphabet
std::string 
suffixArray::getAlphabet
(
	void
)
{
	return m_alphabet;
}




// get number of characters for each letter in alphabet
std::vector<uint32_t> 
suffixArray::getAlphabetCount
(
	void
)
{
	return m_alphabetCount;
}




// prepare for streaming suffix array
uint32_t 
suffixArray::prepareForStream
(
	uint32_t maxBlockSize
)
{
	auto counts = m_alphabetCount;
	auto countMax = *std::max_element(counts.begin(), counts.end());
	uint32_t k = 2;
	// determine kmer size to meet block size requirement
	while (countMax > maxBlockSize)
	{
		histogram(k++);
		auto countMax2 = (*std::max_element(m_hist.begin(), m_hist.end(),
			[](std::pair<std::string, uint32_t> lhs, std::pair<std::string, uint32_t> rhs) 
			-> bool {return lhs.second < rhs.second; })).second;
		if ((double)countMax2 / (double)countMax > 0.75)
		{
			countMax = countMax2;
			break;
		}
		else
			countMax = countMax2;
	}
	m_blockDefinitions.clear();
	// save start and ending kmers plus expected size of suffix array in between
	auto begin = *m_hist.begin();
	saBlockDefinition newSegment;
	newSegment.begin = begin.first;
	newSegment.end = newSegment.begin;
	newSegment.length = begin.second;
	m_blockDefinitions.push_back(newSegment);
	for (auto it = ++m_hist.begin(); it != m_hist.end(); ++it)
	{
		auto currentSegment = (m_blockDefinitions.rbegin());
		if ((*currentSegment).length + (*it).second < maxBlockSize)
		{
			(*currentSegment).end = (*it).first;
			(*currentSegment).length += (*it).second;
		}
		else
		{
			saBlockDefinition newSegment;
			newSegment.begin = (*it).first;
			newSegment.end = newSegment.begin;
			newSegment.length = (*it).second;
			m_blockDefinitions.push_back(newSegment);
		}
	}
	// integrate missing kmers from end of string
	for (size_t i = k - 2; i > 0; i--)
	{
		saBlockDefinition cmpVal;
		cmpVal.begin = m_S.substr(m_S.size() - i, i) + std::string(k - i - 1, *m_alphabet.begin());
		cmpVal.end = cmpVal.begin;
		auto lowInsert = std::lower_bound(m_blockDefinitions.begin(), m_blockDefinitions.end(),cmpVal,
			[](saBlockDefinition lhs, saBlockDefinition rhs) -> bool{return lhs.begin < rhs.begin; });
		if (lowInsert == m_blockDefinitions.begin())
		{
			(*lowInsert).length++;
			(*lowInsert).begin = cmpVal.begin;
		}
		else
		{
			if ((*lowInsert).begin > cmpVal.begin)
				lowInsert--;
			(*lowInsert).length++;
			(*lowInsert).end = std::max((*lowInsert).end, cmpVal.end);
			(*lowInsert).begin = std::min((*lowInsert).begin, cmpVal.begin);
		}
	}
	// reset suffix array structure
	m_currentBlock.clear();
	m_currentSubBlock = m_currentBlock.begin();
	// Debug output
	//uint32_t sum = 0;
	//std::cout << "After Integration" << std::endl;
	//for (auto it = m_blockDefinitions.begin(); it != m_blockDefinitions.end(); ++it)
	//{
	//	std::cout << (*it).begin << " to " << (*it).end << " with " << (*it).length << std::endl;
	//	sum += (*it).length;
	//}
	//std::cout << "Sum: " << sum << std::endl;
	// return actual max block size for pre-allocations
	return (*(std::max_element(m_blockDefinitions.begin(), m_blockDefinitions.end(),
		[](saBlockDefinition lhs, saBlockDefinition rhs)
		-> bool{return lhs.length < rhs.length; }))).length;
}




// get next Segment from suffix array stream
std::vector<uint32_t>
suffixArray::getNextSegment
(
	void
)
{
	// check for already prepared data
	if (m_currentSubBlock == m_currentBlock.end())
	{
		if (m_blockDefinitions.size() == 0)	
		{
			return std::vector<uint32_t>();
		}
		// get new set of indices
		auto blockDefinition = *m_blockDefinitions.begin();
		m_blockDefinitions.pop_front();
		suffixIndices(blockDefinition.begin, blockDefinition.end, blockDefinition.length);
		// sort segment
		sortCurrentBlock();
		// merge short segments
		if (m_currentBlock.size() > 1)
		{
			auto it = std::next(m_currentBlock.begin(), 1);
			while (it != m_currentBlock.end())
			{
				auto it2 = std::prev(it, 1);
				if ((*it).second.size() < BlockMergeLimit || (*it2).second.size() < BlockMergeLimit)
				{
					(*it2).second.insert((*it2).second.end(), (*it).second.begin(), (*it).second.end());
					it = m_currentBlock.erase(it);
				}
				else
					++it;
			}
		}
		m_currentSubBlock = m_currentBlock.begin();
	}
	auto segment = (*m_currentSubBlock).second;
	m_currentSubBlock++;
	return segment;
}




//-- private functions --------- definitions -----------------------------
// get histogram of chars in S
std::map<std::string, uint32_t>
suffixArray::histogram
(
	uint32_t kmerLength
)
{
	size_t segmentSize = (size_t)std::ceil((double)(m_S.size()) / m_threads);
	auto histParts = std::vector<std::map<std::string, uint32_t>>(m_threads);
	auto worker = std::vector<std::thread>();
	std::string::iterator segmentStart = m_S.begin();
	std::string::iterator segmentStop = std::distance(segmentStart + segmentSize, m_S.end() - kmerLength + 1) > 0 ?
										segmentStart + segmentSize : m_S.end() - kmerLength + 1;
	// fork into multiple working threads
	for (size_t i = 0; i < m_threads - 1; i++)
	{
		worker.push_back(std::thread(&suffixArray::histogramSegment, this, std::ref(histParts[i]),
						 segmentStart, segmentStop, kmerLength));
		segmentStart = segmentStop;
		segmentStop = std::distance(segmentStart + segmentSize, m_S.end() - kmerLength + 1) > 0 ?
			segmentStart + segmentSize : m_S.end() - kmerLength + 1;
	}
	if (segmentStart != segmentStop)
		histogramSegment(histParts[m_threads - 1], segmentStart, segmentStop, kmerLength);
	// join other working threads
	for (size_t i = 0; i < worker.size(); i++)
		worker[i].join();
	// merge results
	m_hist.clear();
	for (size_t i = 0; i < histParts.size(); i++)
	{
		auto tmp = histParts[i];
		for (auto it = tmp.begin(); it != tmp.end(); ++it)
			m_hist[(*it).first] += (*it).second;
	}
	return m_hist;
}




// histogram for segment
void 
suffixArray::histogramSegment
(
	std::map<std::string, uint32_t>& hist,
	std::string::iterator begin, 
	std::string::iterator end,
	const uint32_t l
)
{
	while (begin != end)
	{
		hist[std::string(begin, begin + l)]++;
		begin++;
	}		
}




// get indices of prefix
void
suffixArray::suffixIndices
(
	std::string pStart,
	std::string pStop,
	uint32_t length
)
{
	const uint32_t segmentSize = m_S.size() / m_threads;
	auto worker = std::vector<std::thread>();
	std::string::iterator segmentStart = m_S.begin();
	m_currentBlock.clear();
	m_currentSubBlock = m_currentBlock.begin();
	// fork into multiple working threads
	for (uint32_t i = 1; i < m_threads; i++)
	{
		worker.push_back(std::thread(&suffixArray::suffixIndicesSegment, this,
										pStart, pStop, segmentStart, segmentStart + segmentSize));
		segmentStart += segmentSize;
	}
	suffixIndicesSegment(pStart, pStop, segmentStart, m_S.end());
	// join other working threads
	for (size_t i = 0; i < worker.size(); i++)
		worker[i].join();
}




// get indices of prefix range
void 
suffixArray::suffixIndicesSegment
(
	std::string lowerBound, 
	std::string upperBound,
	std::string::iterator begin, 
	const std::string::iterator end
)
{
	std::map<std::string, std::vector<uint32_t>> tempBlock;
	size_t tempBlockItems = 0;
	size_t index = std::distance(m_S.begin(), begin);
	const auto lowBegin = lowerBound.begin();
	const auto upBegin = upperBound.begin();
	const auto lowEnd = lowerBound.end();
	const auto upEnd = upperBound.end();
	const auto globalEnd = m_S.end();
	const ptrdiff_t keyLengthExtension = 2;
	const ptrdiff_t keyLength = std::max(lowerBound.size(), upperBound.size()) + keyLengthExtension;
	std::string padding;
	if (std::distance(end, globalEnd) >= keyLength)
		padding = std::string(end - keyLength, end + keyLength);
	else
		padding = std::string(end - keyLength, end) + std::string(keyLength, *m_alphabet.begin());
	auto paddingBegin = padding.begin();
	const auto safeEnd = end - keyLength;
	// first part before window overlaps string end
	while (begin != safeEnd)
	{
		// check first character manually to avoid function call overhead
		if ((*begin) >= (*lowBegin) && (*begin) <= (*upBegin))
		{
			if (prefixGreaterEqual(begin, lowBegin, globalEnd, lowEnd) &&
				prefixGreaterEqual(upBegin, begin, upEnd, globalEnd))
			{
				tempBlock[std::string(begin, begin + keyLength)].push_back(index);
				tempBlockItems++;
				if (tempBlockItems == IndexBlockSize)
				{
					m_mutex.lock();
					for (auto it = tempBlock.begin(); it != tempBlock.end(); ++it)
						m_currentBlock[(*it).first].insert(m_currentBlock[(*it).first].end(), (*it).second.begin(), (*it).second.end());
					m_mutex.unlock();
					tempBlock.clear();
					tempBlockItems = 0;
				}
			}
		}
		begin++;
		index++;
	}
	// second part with padding of string
	while (paddingBegin != padding.end() - keyLength)
	{
		// check first character manually to avoid function call overhead
		if ((*paddingBegin) >= (*lowBegin) && (*paddingBegin) <= (*upBegin))
		{
			if (prefixGreaterEqual(paddingBegin, lowBegin, padding.end(), lowEnd) &&
				prefixGreaterEqual(upBegin, paddingBegin, upEnd, padding.end()))
			{
				tempBlock[std::string(paddingBegin, paddingBegin + keyLength)].push_back(index);
				tempBlockItems++;
			}
		}
		paddingBegin++;
		index++;
	}
	if (tempBlockItems > 0)
	{
		m_mutex.lock();
		for (auto it = tempBlock.begin(); it != tempBlock.end(); ++it)
			m_currentBlock[(*it).first].insert(m_currentBlock[(*it).first].end(), (*it).second.begin(), (*it).second.end());
		m_mutex.unlock();
	}
	tempBlock.clear();
}




// sort current block of suffix indices
void 
suffixArray::sortCurrentBlock
(
	void
)
{
	m_currentSubBlock = m_currentBlock.begin();
	size_t numWorker = std::min((size_t)m_threads, m_currentBlock.size());
	// fork into multiple working threads
	auto worker = std::vector<std::thread>();
	for (size_t i = 1; i < numWorker; i++)
		worker.push_back(std::thread(&suffixArray::sortCurrentBlockWorker, this, i));
	sortCurrentBlockWorker(numWorker);
	for (auto it = worker.begin(); it != worker.end(); ++it)
		(*it).join();
	m_currentSubBlock = m_currentBlock.begin();
}




// sorting worker function
void 
suffixArray::sortCurrentBlockWorker
(
	uint32_t id
)
{
	suffixComparator cmp(m_S);
	for (;;)
	{
		// concurrently sorting sub-blocks of suffix array
		m_mutex.lock();
		if (m_currentSubBlock == m_currentBlock.end())
		{
			m_mutex.unlock();
			break;
		}
		auto myBlock = m_currentSubBlock++;
		m_mutex.unlock();
		auto beforeSort((*myBlock).second);
		// improve sort performance on suffixes starting in homopolymers
		if (isSingleChar((*myBlock).first))
		{	
			std::sort((*myBlock).second.begin(), (*myBlock).second.end());
			std::vector<std::pair<uint32_t, uint32_t>> extendedSA;
			extendedSA.reserve((*myBlock).second.size());
			uint32_t previousSuffix = *(*myBlock).second.rbegin();
			uint32_t skip = 0;
			extendedSA.push_back(std::make_pair(previousSuffix, skip));
			// store suffix index and the number of preceding equal characters
			// these can be skipped during sorting process
			for (auto it = (*myBlock).second.rbegin() + 1; it != (*myBlock).second.rend(); ++it)
			{
				if ((*it) == previousSuffix - 1)
					extendedSA.push_back(std::make_pair((*it), ++skip));
				else
				{
					skip = 0;
					extendedSA.push_back(std::make_pair((*it), skip));
				}
				previousSuffix = *it;
			}
			// suffix comparator is overloaded to consider skipping information
			std::sort(extendedSA.begin(), extendedSA.end(), cmp);
			auto saIt = (*myBlock).second.begin();
			for (auto it = extendedSA.begin(); it != extendedSA.end(); ++it)
			{
				*saIt = (*it).first;
				saIt++;
			}	
		}
		// sort selected block using standard suffix comparator
		else
		{
			std::sort((*myBlock).second.begin(), (*myBlock).second.end(), cmp);
		}		
	}
	return;
}




// compare two strings for greater equal
bool
prefixGreaterEqual
(
	std::string::iterator firstIter,
	std::string::iterator secondIter,
	const std::string::iterator firstEnd,
	const std::string::iterator secondEnd
)
{
	while (firstIter != firstEnd && secondIter != secondEnd)
	{
		if ((*firstIter) > (*secondIter))
			return true;
		if ((*firstIter) < (*secondIter))
			return false;
		firstIter++;
		secondIter++;
	}
	return true;
}




// compute all combinations of alphabet with length k
static void
kmerCombinations
(
	std::string alphabet,
	std::string prefix,
	std::vector<std::string>& result,
	int k
)
{
	if (k == 0)
	{
		result.push_back(prefix);
		return;
	}
	for (size_t i = 0; i < alphabet.size(); ++i)
	{
		std::string newPrefix = prefix + alphabet[i];
		kmerCombinations(alphabet, newPrefix, result, k - 1);
	}
}




// check if string is build of single character
bool isSingleChar
(
	std::string str
)
{
	char previousChar;
	if (str.size() > 0)
		previousChar = *str.begin();
	else
		return false;
	for (auto it = str.begin(); it != str.end(); ++it)
	{
		if ((*it) != previousChar)
			return false;
		previousChar = (*it);
	}
	return true;
}
