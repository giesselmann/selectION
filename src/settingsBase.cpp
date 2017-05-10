// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class settingsBase
//
//  DESCRIPTION   :	Base class for all other settings
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------

//-- private headers -----------------------------------------------------
#include "settingsBase.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
settingsBase::settingsBase
(

) : m_logger(logger())
{

}




// virtual destructor
settingsBase::~settingsBase()
{

}




//  set logger
void 
settingsBase::setLogger
(
	const logger& value
)
{
	m_logger = value;
}




logger& 
settingsBase::logging(void)
{
	return m_logger;
}




//-- private functions --------- definitions -----------------------------