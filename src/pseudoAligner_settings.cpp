// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class fmIndex_settings
//
//  DESCRIPTION   :	Settings for fmIndex
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gießelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------

//-- private headers -----------------------------------------------------
#include "pseudoAligner_settings.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
pseudoAligner_settings::pseudoAligner_settings()
{

}




// virtual destructor
pseudoAligner_settings::~pseudoAligner_settings()
{

}




// setter
void 
pseudoAligner_settings::set_m_scanPrefix(uint32_t value)
{
	value > 0 ? m_scanPrefix = value : m_scanPrefix = 1;
}




void
pseudoAligner_settings::set_m_seedLength(uint32_t value)
{
	value > 1 ? m_seedLength = value : m_seedLength = 2;
}




// getter
uint32_t 
pseudoAligner_settings::get_m_scanPrefix(void)
{
	return m_scanPrefix;
}




uint32_t
pseudoAligner_settings::get_m_seedLength(void)
{
	return m_seedLength;
}
//-- private functions --------- definitions -----------------------------