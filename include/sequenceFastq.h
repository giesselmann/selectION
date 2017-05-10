// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class fastqRecord
//
//  DESCRIPTION   :	Container class for fastq sequence record
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
#include <istream>
#include "sequenceBase.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------

class sequenceFastq : public sequenceBase
{
public:
	// constructor
	sequenceFastq(std::string name, std::string sequence, std::string description, std::string quality);
	sequenceFastq(std::istream& str);

	// virtual destructor
	virtual ~sequenceFastq();

	// get methods
	std::string getDescription(void)const;
	std::string getQuality(void)const;

protected:
	// write record to stream 
	virtual void write2Stream(std::ostream& stream) const;

private:
	// methods
	// default constructor must not be used
	sequenceFastq();

	// member
	std::string m_description;
	std::string m_quality;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------
