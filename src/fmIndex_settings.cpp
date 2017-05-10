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
#include "fmIndex_settings.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// default constructor
fmIndex_settings::fmIndex_settings()
{

}




// virtual destructor
fmIndex_settings::~fmIndex_settings()
{

}




// setter
void 
fmIndex_settings::set_m_threads(uint32_t value)
{
	m_threads = value > 0 ? value : 1;
}




void 
fmIndex_settings::set_m_tallyStepSize(uint32_t value)
{
	m_tallyStepSize = value > 0 ? value : 1;
}




void 
fmIndex_settings::set_m_saSampleStepSize(uint32_t value)
{
	m_saSampleStepSize = value > 0 ? value : 1;
}




void 
fmIndex_settings::set_m_MaxSuffixMemoryBlock(uint32_t value)
{
	m_MaxSuffixMemoryBlock = value > 10000 ? value : 10000;	// we need at least a bit of space to sort
}




// getter
uint32_t 
fmIndex_settings::get_m_threads(void)
{
	return m_threads;
}




uint32_t 
fmIndex_settings::get_m_tallyStepSize(void)
{
	return m_tallyStepSize;
}




uint32_t 
fmIndex_settings::get_m_saSampleStepSize(void)
{
	return m_saSampleStepSize;
}




uint32_t 
fmIndex_settings::get_m_MaxSuffixMemoryBlock(void)
{
	return m_MaxSuffixMemoryBlock;
}




//-- private functions --------- definitions -----------------------------