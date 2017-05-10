// \HEADER\---------------------------------------------------------------
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
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include <string>
#include "pseudoAligner_settings.h"
#include "fileSAM.h"
#include "fmIndex.h"

// -- forward declarations -----------------------------------------------
struct seedType;

// -- exported constants, types, classes ---------------------------------
class pseudoAligner
{
public:
	// constructor
	pseudoAligner(pseudoAligner_settings& settings, fmIndex& index);

	// virtual destructor
	virtual ~pseudoAligner();

	// get most likely position of flawed string
	samRecord estimatePosition(const std::string str);

	// get alignment at distributed seeds
	samRecord seedAlign(const std::string str);

protected:

private:
	// methods
	// default constructor must not be used
	pseudoAligner();

	// Copy constructor must not be used
	pseudoAligner(const pseudoAligner& object);

	// Assignment operator must not be used
	const pseudoAligner& operator=(const pseudoAligner& rhs);

	// get positions for seeds
	std::vector<seedType> getSeedPositions(std::vector<std::string>& seeds, uint32_t seedDistance);

	// member
	pseudoAligner_settings m_settings;
	fmIndex& m_index;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------