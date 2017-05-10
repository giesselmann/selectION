// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class fastaRecord
//
//  DESCRIPTION   :	Container class for fasta sequence record
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
#include <vector>
#include <istream>
#include "sequenceBase.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------

class sequenceFasta : public sequenceBase
{
public:
	// constructor
	sequenceFasta(std::string name, std::string sequence, std::vector<std::string> comments);
	sequenceFasta(std::istream& str);

	// virtual destructor
	virtual ~sequenceFasta();

	// get methods
	std::string getName(void)const;
	std::vector<std::string> getComments(void)const;

protected:
	// write record to stream 
	virtual void write2Stream(std::ostream& stream) const;

private:
	// methods
	// default constructor must not be used
	sequenceFasta();

	// member
	std::vector<std::string> m_comments;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------