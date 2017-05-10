// \HEADER\---------------------------------------------------------------
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
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include <cstdint>
#include <istream>
#include <string>
#include <vector>
#include <map>

// -- forward declarations -----------------------------------------------
struct selector;

// -- exported constants, types, classes ---------------------------------
class positionFilter
{
public:
	// constructor
	positionFilter();

	// virtual destructor
	virtual ~positionFilter();

	// load selectors from stream of csv lines (ref;start[;stop])
	uint32_t addSelectors(std::string path2csv);
	uint32_t addSelectors(std::istream& selectorLines);

	// clear all selector definitions
	void clearSelectors(void);
	
	// get number of active selectors
	uint32_t getActiveSelectorCount(void);

	// check if position matches filter
	// return list of match descriptions
	std::vector<std::string> match(std::string ref, uint32_t pos);

	// check if position overlaps filter taking length into account
	// return list of match descriptions
	std::vector<std::string> match(std::string ref, uint32_t pos, uint32_t length);

protected:

private:
	// methods

	// member
	// selectors as [reference] [vector <  selector > ]
	std::map<std::string, std::vector<selector>> m_selectors;
};

