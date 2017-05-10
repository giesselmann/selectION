// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class positionFilter
//
//  DESCRIPTION   :	Decide if estimated position of read matches filter
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gie�elmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>

//-- private headers -----------------------------------------------------
#include "positionFilter.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------
typedef struct selector
{
public:
	uint32_t m_start = 0;
	uint32_t m_stop = 0;
	std::string m_description = "";
}selector;


//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
positionFilter::positionFilter()
{

}




// virtual destructor
positionFilter::~positionFilter()
{

}




// load selectors from stream of csv lines (ref;start[;stop])
uint32_t 
positionFilter::addSelectors
(
	std::string path2csv
)
{
	std::ifstream selectors(path2csv);
	if (selectors.good())
		return addSelectors(selectors);
	else
		throw std::invalid_argument("File " + path2csv + " not found");
}




uint32_t 
positionFilter::addSelectors
(
	std::istream& selectorLines
)
{
	std::string rawLine;
	uint32_t parsedSelectors = 0;
	while (std::getline(selectorLines, rawLine))
	{
		if (rawLine.size() > 0 && rawLine[0] != '#')
		{
			std::istringstream lineStream(rawLine);
			std::string cell;
			std::vector<std::string> cols;
			while (std::getline(lineStream, cell, ';'))
			{
				cols.push_back(cell);
			}
			selector sel;
			if (cols.size() == 2)		// ref and start
			{
				try
				{
					sel.m_start = std::stoi(cols[1]);
					sel.m_stop = sel.m_start;
					sel.m_description = cols[0] + "_" +
										std::to_string(sel.m_start) + "_" +
										std::to_string(sel.m_stop);
				}
				catch (std::exception e)
				{
					// no handling, just ignore line
					continue;
				}
				m_selectors[cols[0]].push_back(sel);
				parsedSelectors++;
			}
			else if (cols.size() == 3)	// ref, start and stop
			{
				try
				{
					sel.m_start = std::stoi(cols[1]);
					sel.m_stop = std::stoi(cols[2]);
					sel.m_description = cols[0] + "_" +
										std::to_string(sel.m_start) + "_" +
										std::to_string(sel.m_stop);
				}
				catch (std::exception e)
				{
					// no handling, just ignore line
					continue;
				}
				if (sel.m_start <= sel.m_stop)
				{
					m_selectors[cols[0]].push_back(sel);
					parsedSelectors++;
				}
			}
		}
	}
	// sort selector map entries by end position
	for (auto it = m_selectors.begin(); it != m_selectors.end(); ++it)
	{
		auto regions = (*it).second;
		std::sort(regions.begin(), regions.end(),
			[](selector lhs, selector rhs)
			-> bool {return lhs.m_stop < rhs.m_stop; });
	}
	return parsedSelectors;
}




// clear all selector definitions
void 
positionFilter::clearSelectors
(
	void
)
{
	m_selectors.clear();
}




// get number of active selectors
uint32_t 
positionFilter::getActiveSelectorCount(void)
{
	return m_selectors.size();
}




// check if position matches filter
// return list of match descriptions
std::vector<std::string> 
positionFilter::match
(
	std::string ref, 
	uint32_t pos
)
{
	return match(ref, pos, 0);
}




// check if position matches filter with respect to read length
// return list of match descriptions
std::vector<std::string> 
positionFilter::match
(
	std::string ref, 
	uint32_t pos, 
	uint32_t length
)
{
	std::vector<std::string> matches;
	if (m_selectors.count(ref))
	{
		auto regions = m_selectors[ref];
		auto lower = std::lower_bound(regions.begin(), regions.end(), pos,
			[](selector lhs, uint32_t rhs)
			-> bool {return lhs.m_stop < rhs; });
		while (lower != regions.end())
		{
			if (pos <= (*lower).m_stop && pos + length >= (*lower).m_start)
				matches.push_back((*lower).m_description);
			if (pos > (*lower).m_stop)
				break;
			lower++;
		}
		return matches;
	}
	else
		return matches;
}




//-- private functions --------- definitions -----------------------------