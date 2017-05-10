// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class logger
//
//  DESCRIPTION   :	Log any program output, redirect to file/ std::cout
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
#include <ostream>
#include <sstream>
#include <string>
#include <mutex>
#include <cstdint>

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
enum LogSeverityLevel
{
	e_logFatal = 0,
	e_logError = 1,
	e_logWarning = 2,
	e_logInfo = 3,
	e_logDebug = 4,
	e_logTrace = 5
};


class logger
{
public:
	// default constructor using std::cout
	logger();

	// construct using output stream
	logger(std::ostream& outStream);

	// virtual destructor
	virtual ~logger();

	// Copy constructor
	logger(const logger& object);

	// Assignment operator
	const logger& operator=(const logger& rhs);

	// append entry to log
	void log(const LogSeverityLevel lvl, std::string message);

protected:

private:
	// methods

	// member
	std::mutex m_mutex;
	std::ostream& m_stream;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------