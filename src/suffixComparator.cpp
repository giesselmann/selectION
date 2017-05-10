// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <cstddef>

//-- private headers -----------------------------------------------------
#include "suffixComparator.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor with string
suffixComparator::suffixComparator
(
	const std::string& comparisonTarget
) : m_target(comparisonTarget)
{
	init();
}




// virtual destructor
suffixComparator::~suffixComparator()
{

}




//-- private functions --------- definitions -----------------------------
// init
void 
suffixComparator::init()
{
	if (m_target.size() >= 16)
		m_targetEndSSE = static_cast<uint32_t>(m_target.size()) - 16;
}
