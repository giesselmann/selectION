// \HEADER\---------------------------------------------------------------
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
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include <cstdint>
#include <string>
#include "settingsBase.h"

// -- forward declarations -----------------------------------------------
class fmIndex;

// -- exported constants, types, classes ---------------------------------
class fmIndex_settings : public settingsBase
{
friend class fmIndex;	// allow direct access to settings
public:
	// default constructor
	fmIndex_settings();

	// virtual destructor
	virtual ~fmIndex_settings();

	// setter
	void set_m_threads(uint32_t value);
	void set_m_tallyStepSize(uint32_t value);
	void set_m_saSampleStepSize(uint32_t value);
	void set_m_MaxSuffixMemoryBlock(uint32_t value);

	// getter
	uint32_t get_m_threads(void);
	uint32_t get_m_tallyStepSize(void);
	uint32_t get_m_saSampleStepSize(void);
	uint32_t get_m_MaxSuffixMemoryBlock(void);

protected:

private:
	// methods
	// default copy constructor and assignment operator work

	// member
	uint32_t m_threads = 1;							// threads to use in construction phase
	uint32_t m_tallyStepSize = 128;					// store every 128th sample of tally
	uint32_t m_saSampleStepSize = 64;				// store every 64th sample of suffix array
	uint32_t m_MaxSuffixMemoryBlock = 200000000;	// 200 MB for sorting suffixes
};

// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------