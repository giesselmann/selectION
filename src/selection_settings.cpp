// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class selectION_settings
//
//  DESCRIPTION   :	Settings for selectION
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
#include "selection_settings.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
selection_settings::selection_settings()
{

}




// virtual destructor
selection_settings::~selection_settings()
{

}




// getter
uint32_t
selection_settings::get_m_threads(void)
{
	return m_threads;
}




std::string
selection_settings::get_m_sam
(
	void
)
{
	return m_sam;
}




uint32_t
selection_settings::get_m_qualityThreshold(void)
{
	return m_qualityThreshold;
}




fmIndex_settings&
selection_settings::get_m_fmIndex_settings
(
	void
)
{
	return m_fmIndex_settings;
}




pseudoAligner_settings&
selection_settings::get_m_pseudoAligner_settings
(
void
)
{
	return m_pseudoAligner_settings;
}




// setter
void 
selection_settings::set_m_threads
(
	uint32_t value
)
{
	if (value > 0)
	{
		m_threads = value;
		m_fmIndex_settings.set_m_threads(value);
	}		
}




void 
selection_settings::set_m_sam
(
	std::string value
)
{
	m_sam = value;
}




void 
selection_settings::set_m_cmd
(
	std::string value
)
{
	m_cmd = value;
}




void 
selection_settings::set_m_qualityThreshold
(
	uint32_t value
)
{
	m_qualityThreshold = value;
}




void 
selection_settings::set_m_fmIndex_settings
(
	fmIndex_settings value
)
{
	m_fmIndex_settings = value;
}




void
selection_settings::set_m_pseudoAligner_settings
(
	pseudoAligner_settings value
)
{
	m_pseudoAligner_settings = value;
}




//-- private functions --------- definitions -----------------------------