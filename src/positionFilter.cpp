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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
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
	uint32_t m_stop = 0xFFFFFFFF;
	std::string m_description = "";
}selector;


//-- private functions --------- declarations ----------------------------
bool is_number(const std::string& s);

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
            if (cols.size() < 1 || cols.size() > 4)
                continue;
			selector sel;
            std::string key = cols[0];
            try
            {
                if (cols.size() > 1)
                {
                    if (is_number(cols[1]))
                        sel.m_start = std::stoi(cols[1]);
                    else
                        sel.m_description = cols[1];
                }
                if (cols.size() > 2)
                {
                    if (is_number(cols[2]))
                        sel.m_stop = std::stoi(cols[2]);
                    else
                    {
                        sel.m_stop = sel.m_start;
                        sel.m_description = cols[2];
                    }
                }
            }
            catch(...)
            {
                // no handling, ignore line
                continue;
            }
            if (cols.size() > 3)
                sel.m_description = cols[3];
            else if (sel.m_description == "")
                sel.m_description = key + "_" +
										std::to_string(sel.m_start) + "_" +
										std::to_string(sel.m_stop);
            m_selectors[key].push_back(sel);
            parsedSelectors++;
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
bool 
is_number
(
    const std::string& s
)
{
    // http://ideone.com/OjVJWh
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}



