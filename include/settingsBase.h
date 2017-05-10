// \HEADER\---------------------------------------------------------------
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
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include "logger.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class settingsBase
{
public:
	// default constructor
	settingsBase();

	// virtual destructor
	virtual ~settingsBase();

	//  logging
	void setLogger(const logger& value);
	logger& logging(void);

protected:

private:
	// methods

	// member
	logger m_logger;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------