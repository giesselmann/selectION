// \HEADER\---------------------------------------------------------------
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
//  All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------
#pragma once
// -- required headers ---------------------------------------------------
#include <cstdint>
#include <string>
#include "settingsBase.h"
#include "fmIndex_settings.h"
#include "pseudoAligner_settings.h"

// -- forward declarations -----------------------------------------------
class selectION;

// -- exported constants, types, classes ---------------------------------
class selection_settings : public settingsBase
{
friend class selectION;	// allow direct access to settings
public:
	// default constructor
	selection_settings();

	// virtual destructor
	virtual ~selection_settings();

	// getter
	uint32_t get_m_threads(void);
	std::string get_m_sam(void);
	uint32_t get_m_qualityThreshold(void);
	fmIndex_settings& get_m_fmIndex_settings(void);
	pseudoAligner_settings& get_m_pseudoAligner_settings(void);

	// setter
	void set_m_threads(uint32_t value);
	void set_m_sam(std::string value);
	void set_m_cmd(std::string value);
	void set_m_qualityThreshold(uint32_t value);
	void set_m_fmIndex_settings(fmIndex_settings value);
	void set_m_pseudoAligner_settings(pseudoAligner_settings value);

protected:

private:
	// methods
	// default copy constructor and assignment operator work

	// member
	uint32_t m_threads = 1;
	std::string m_sam = "";
	std::string m_cmd = "";
	uint32_t m_qualityThreshold = 20;

	// FM-Index settings
	fmIndex_settings m_fmIndex_settings;

	// Pseudo aligner settings
	pseudoAligner_settings m_pseudoAligner_settings;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------