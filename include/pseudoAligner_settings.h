// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class pseudoAligner_settings
//
//  DESCRIPTION   :	Settings for pseudo aligner
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
#include <string>
#include "settingsBase.h"

// -- forward declarations -----------------------------------------------
class pseudoAligner;

// -- exported constants, types, classes ---------------------------------
class pseudoAligner_settings : public settingsBase
{
friend class pseudoAligner;	// allow direct access to settings
public:
	// constructor
	pseudoAligner_settings();

	// virtual destructor
	virtual ~pseudoAligner_settings();

	// setter
	void set_m_scanPrefix(uint32_t value);
	void set_m_seedLength(uint32_t value);

	// getter
	uint32_t get_m_scanPrefix(void);
	uint32_t get_m_seedLength(void);

protected:

private:
	// methods
	// default copy constructor and assignment operator work

	// member
	uint32_t m_scanPrefix = 1000;		// prefix of sequence to use for alignment
	uint32_t m_seedLength = 15;			// length of seeds for distributed alignment
	uint32_t m_seedDistance = 100;		// distance of seeds in read sequence
};

// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------