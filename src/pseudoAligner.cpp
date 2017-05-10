// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class pseudoAligner
//
//  DESCRIPTION   :	Estimated position of sequence based on kmer matches
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <random>
#include <vector>
#include <map>
#include <list>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <climits>
#include <iostream>

//-- private headers -----------------------------------------------------
#include "pseudoAligner.h"
#include "fileSAM.h"
#include "bio.h"
#include "fmIndex.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const uint32_t WindowSize = 100;				// lcs window
const uint32_t WindowShift = 80;				// lcs window shift
const uint32_t maxSeedsPerRow = 100;

//-- private types -------------------------------------------------------
struct seedType
{
	uint32_t row = 0;				// offset in read
	uint32_t col = 0;				// offset in reference
	uint32_t length = 0;			// length of exact match
	int64_t magnitude;				// distance to origin
};

//-- private functions --------- declarations ----------------------------
// check if seed is homopolymer
bool isHomoPolymer(std::string& str);
// project seeds to main diagonal of dotplot matrix
void orthogonalProject2Line(std::vector<seedType>& seeds);
// project seeds to line specified by row/column direction vector
void orthogonalProject2Line(std::vector<seedType>& seeds, const std::pair<int64_t, int64_t> direction);
// cluster seeds to find best region
std::list<seedType> clusterProjectedSeeds(std::vector<seedType>& seeds);
// revise for overlapping seeds and invalid paths
void reviseSeedPath(std::list<seedType>& path);
// construct alignment cigar from path
std::string path2alignmentCigar(std::list<seedType>const & path, uint32_t readLength);

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
pseudoAligner::pseudoAligner
(
	pseudoAligner_settings& settings,
	fmIndex& index
) : m_settings(settings),
	m_index(index)
{

}




// virtual destructor
pseudoAligner::~pseudoAligner()
{

}




// get most likely position of flawed string
samRecord 
pseudoAligner::estimatePosition
(
	const std::string str
)
{
	std::vector<seedType> forwardMatches, reverseMatches;
	const uint32_t scanMax = str.size() >= m_settings.m_scanPrefix ? m_settings.m_scanPrefix : static_cast<uint32_t>(str.size());
	auto fStr = std::string(str.begin(), str.begin() + scanMax);
	auto rStr = complement(std::string(str.rbegin(), str.rbegin() + scanMax));
	auto strBegin = fStr.begin();
	auto rStrBegin = rStr.begin();
	size_t windowStart = 0;
	// get longest common substrings of overlapping parts of the read and the reference in the index
	// store organized like main diagonals of a dot-plot matrix (diagonal, offset, length of lcs)
	while (windowStart + WindowSize < scanMax)
	{
		auto const & fMatch = m_index.getLongestCommonSubsequence(std::string(strBegin, strBegin + WindowSize));
		auto const & rMatch = m_index.getLongestCommonSubsequence(std::string(rStrBegin, rStrBegin + WindowSize));
		for (auto it = fMatch.begin(); it != fMatch.end(); ++it)
		{
			seedType match;
			match.col = (*it).indexStart;
			match.row = (*it).strStart + windowStart;
			match.length = (*it).lcsLength;
			forwardMatches.push_back(match);
		}
		for (auto it = rMatch.begin(); it != rMatch.end(); ++it)
		{
			seedType match;
			match.col = (*it).indexStart;
			match.row = (*it).strStart + windowStart;
			match.length = (*it).lcsLength;
			reverseMatches.push_back(match);
		}
		strBegin += WindowShift;
		rStrBegin += WindowShift;
		windowStart += WindowShift;
	}
	// group exact matches
	// const std::pair<int64_t, int64_t> projection = std::make_pair(-1, 1);
	orthogonalProject2Line(forwardMatches);
	orthogonalProject2Line(reverseMatches);
	auto fwdPath = clusterProjectedSeeds(forwardMatches);
	auto bwdPath = clusterProjectedSeeds(reverseMatches);
	// compute coverage of path
	uint32_t fwdHits = 0, bwdHits = 0;
	for (auto it = fwdPath.begin(); it != fwdPath.end(); ++it)
		fwdHits += (*it).length;
	for (auto it = bwdPath.begin(); it != bwdPath.end(); ++it)
		bwdHits += (*it).length;
	// return position on strand with more exact matches
	samRecord result;
	if (fwdHits > bwdHits)
	{
		double matchRatio = static_cast<double>(fwdHits) / bwdHits;
		result.FLAG = 0;
		auto relativePosition = m_index.getRelativePosition((*fwdPath.begin()).col);
		result.RNAME = relativePosition.first;
		result.POS = relativePosition.second + 1;
		result.MAPQ = std::round(10 * std::log2(matchRatio));
	}
	else
	{
		double matchRatio = static_cast<double>(bwdHits) / fwdHits;
		result.FLAG = samFlag::e_reverseComplement;		
		auto relativePosition = m_index.getRelativePosition((*bwdPath.begin()).col);
		result.RNAME = relativePosition.first;
		result.POS = relativePosition.second + 1;
		result.MAPQ = std::round(10 * std::log2(matchRatio));
	}
	return result;
}




// get alignment at distributed seeds
samRecord 
pseudoAligner::seedAlign
(
	const std::string str
)
{
	const uint32_t seedLength = m_settings.m_seedLength;
	const uint32_t seedDistance = m_settings.m_seedDistance;
	samRecord result;
	if(str.size() < seedLength)
	{
		result.FLAG = samFlag::e_unmapped;
		return result;
	}
	// extract seeds from read sequence
	std::vector<std::string> seedStrings;
	for (uint32_t i = 0; i < str.size() - seedLength; i += seedDistance)
	{
		std::string seed = std::string(str.begin() + i, str.begin() + i + seedLength);
		if (!isHomoPolymer(seed))
			seedStrings.push_back(seed);
	}
		
	// compute most likely path for template read
	const std::pair<int64_t, int64_t> projection = std::make_pair(-1, 1);
	auto seeds = getSeedPositions(seedStrings, seedDistance);
	orthogonalProject2Line(seeds, projection);
	auto fwdPath = clusterProjectedSeeds(seeds);
	reviseSeedPath(fwdPath);
	
	// compute most likely path for complement read
	std::reverse(seedStrings.begin(), seedStrings.end());
	for (auto it = seedStrings.begin(); it != seedStrings.end(); ++it)
		(*it) = reverseComplement(*it);
	seeds = getSeedPositions(seedStrings, seedDistance);
	orthogonalProject2Line(seeds, projection);
	auto bwdPath = clusterProjectedSeeds(seeds);
	reviseSeedPath(bwdPath);
	
	// concatenate result
	if (fwdPath.size() > bwdPath.size())
	{
		result.FLAG = 0;
		result.CIGAR = path2alignmentCigar(fwdPath, str.size());
		std::tie(result.RNAME, result.POS) = m_index.getRelativePosition((*fwdPath.begin()).col);
		result.POS++;
		result.MAPQ = std::round(-10 * std::log2(static_cast<double>(fwdPath.size()) / seedStrings.size()));
		result.SEQ = str;
	}
	else
	{
		result.FLAG = samFlag::e_reverseComplement;
		result.CIGAR = path2alignmentCigar(bwdPath, str.size());
		std::tie(result.RNAME, result.POS) = m_index.getRelativePosition((*bwdPath.begin()).col);
		result.POS++;
		result.MAPQ = std::round(-10 * std::log2(static_cast<double>(bwdPath.size()) / seedStrings.size()));
		result.SEQ = reverseComplement(str);
	}
	return result;
}




//-- private functions --------- definitions -----------------------------
// get positions for seeds
std::vector<seedType> 
pseudoAligner::getSeedPositions
(
	std::vector<std::string>& seeds,
	uint32_t seedDistance
)
{
	std::vector<seedType> positions;
	uint32_t seedRow = 0;
	for (auto it = seeds.cbegin(); it != seeds.cend(); ++it)
	{
		auto const & pos = m_index.getMatchingPositions((*it), maxSeedsPerRow);
		positions.reserve(positions.size() + pos.size());
		for (auto it2 = pos.cbegin(); it2 != pos.cend(); ++it2)
		{
			seedType seed;
			seed.col = *it2;
			seed.row = seedRow;
			seed.length = (*it).size();
			positions.push_back(seed);
		}
		seedRow += seedDistance;
	}
	return positions;
}




// check if seed is homopolymer
bool isHomoPolymer
(
	std::string& str
)
{
	if (str.size() > 1)
	{
		const char first = *str.begin();
		for (auto it = str.begin() + 1; it != str.end(); ++it)
			if ((*it) != first)
				return false;
	}
	return true;
}




// project seeds to main diagonal of dotplot matrix
void 
orthogonalProject2Line
(
	std::vector<seedType>& seeds
)
{
	for (auto it = seeds.begin(); it != seeds.end(); ++it)
	{
		(*it).magnitude = static_cast<int64_t>((*it).col - (*it).row);
	}
}




// project seeds to line specified by row/column direction vector
void 
orthogonalProject2Line
(
	std::vector<seedType>& seeds,
	const std::pair<int64_t, int64_t> direction
)
{
	const int64_t scalar_direction = direction.first * direction.first + direction.second * direction.second;
	if (scalar_direction == 0)
		throw std::invalid_argument("Projection to 0");
	for (auto it = seeds.begin(); it != seeds.end(); ++it)
	{
		int64_t scalar_seed = (*it).row * direction.first + (*it).col * direction.second;
		double scale = static_cast<double>(scalar_seed) / scalar_direction;
		auto row = scale * direction.first;
		auto col = scale * direction.second;
		if (row <= 0)
			(*it).magnitude = static_cast<int64_t>(std::round(std::sqrt(row*row + col*col)));
		else
			(*it).magnitude = static_cast<int64_t>(std::round(-std::sqrt(row*row + col*col)));
	}
}




// cluster seeds to find best path
std::list<seedType>
clusterProjectedSeeds
(
	std::vector<seedType>& seeds
)
{
	if (seeds.size() < 2)
		return std::list<seedType>();
	// sort seeds by distance to origin
	std::sort(seeds.begin(), seeds.end(),
		[](seedType lhs, seedType rhs) -> bool {return lhs.magnitude < rhs.magnitude; });

	// sliding window to find most dense region of seeds
	const int64_t clusterSize = 1000;
	auto lowerBound = seeds.begin();
	auto upperBound = std::upper_bound(seeds.begin(), seeds.end(), (*lowerBound).magnitude + clusterSize,
		[](int64_t lhs, seedType rhs) -> bool {return lhs < rhs.magnitude; });

	typedef std::pair<std::vector<seedType>::iterator, int64_t> clusterType;
	std::vector<clusterType> cluster;
	cluster.push_back(std::make_pair(lowerBound, std::distance(lowerBound, upperBound)));
	while (upperBound != seeds.end())
	{
		// step forward
		lowerBound++;
		while (upperBound != seeds.end() && (*upperBound).magnitude <= (*lowerBound).magnitude + clusterSize)
			upperBound++;
		cluster.push_back(std::make_pair(lowerBound, std::distance(lowerBound, upperBound)));
	}

	// get seeds from highest scoring window
	std::list<seedType> path;
	std::sort(cluster.begin(), cluster.end(), 
		[](clusterType lhs, clusterType rhs) -> bool {return lhs.second < rhs.second; });
	auto maxCluster = *cluster.rbegin();
	for (int64_t i = 0; i < maxCluster.second; ++i)
		path.push_back(*maxCluster.first++);
	
	return path;
}




// revise for overlapping seeds and invalid paths
void 
reviseSeedPath
(
	std::list<seedType>& path
)
{
	// check for overlapping seeds
	if (path.size() < 2)
		return;
	path.sort([](seedType lhs, seedType rhs) -> bool { return lhs.magnitude < rhs.magnitude; });
	auto pathMid = path.begin();
	std::advance(pathMid, path.size() / 2);
	const auto medianMagnitude = (*pathMid).magnitude;
	// reset magnitude to distance from read begin
	for (auto it = path.begin(); it != path.end(); ++it)
		(*it).magnitude = (*it).row + (*it).magnitude - medianMagnitude;
	path.sort([](seedType lhs, seedType rhs) -> bool { return lhs.magnitude < rhs.magnitude; });
	// TODO do not overwrite magnitude for later comparison
}




// construct alignment cigar from path
std::string 
path2alignmentCigar
(
	std::list<seedType> const & path,
	uint32_t readLength
)
{
	if (path.size() == 0)
		return std::to_string(readLength) + "S";
	std::string cigar;
	if ((*path.begin()).row > 0)
	{
		cigar += std::to_string((*path.begin()).row) + "S";
	}
	for (auto it = path.cbegin(); it != path.cend(); ++it)
	{
		auto it_next = std::next(it);
		if (it_next != path.cend())
		{
			const uint32_t rows = (*it_next).row - (*it).row;
			const uint32_t cols = (*it_next).col - (*it).col;
			if (rows > cols)
			{
				// match + insertion
				cigar += std::to_string(cols) + "M";
				cigar += std::to_string(rows - cols) + "I";
			}
			else if (cols > rows)
			{
				// match + deletion
				cigar += std::to_string(rows) + "M";
				cigar += std::to_string(cols - rows) + "D";
			}
			else
			{
				// match
				cigar += std::to_string(rows) + "M";
			}
		}
		else
		{
			if (static_cast<int64_t>(readLength)-(*it).row > 0)
				cigar += std::to_string(readLength - (*it).row) + "M";
		}
	}
	return cigar;
}



