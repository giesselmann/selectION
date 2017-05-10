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
#include <iostream>
#include <iomanip>
#include <vector>

//-- private headers -----------------------------------------------------
#include "logger.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
static const std::vector<std::string> 
LogSeverityLevelMessage = 
{"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};


//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// default constructor
logger::logger
(

) : m_stream(std::cout)
{

}




// construct using output stream
logger::logger
(
	std::ostream& outStream
) : m_stream(outStream)
{

}




// virtual destructor
logger::~logger()
{
	m_stream.flush();
}




// Copy constructor
logger::logger
(
	const logger& object
) : m_stream(object.m_stream)
{

}




// Assignment operator
const logger& 
logger::operator=
(
	const logger& rhs
)
{
	m_stream.flush();
	m_stream.rdbuf(rhs.m_stream.rdbuf());	
	return *this;
}




// append entry to log
void 
logger::log
(
	const LogSeverityLevel lvl, 
	std::string message
)
{
	// We dont't know target of m_stream, better be thread safe
	std::lock_guard<std::mutex> guard(m_mutex);
	m_stream << ""
			 << LogSeverityLevelMessage[lvl] 
			 << " | " << message << std::endl;

}



//-- private functions --------- definitions -----------------------------