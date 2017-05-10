// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class sequenceRecord
//
//  DESCRIPTION   :	Container base class for sequence records
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
#include <string>
#include <ostream>

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class sequenceBase
{
public:
	// constructor
	sequenceBase(std::string name, std::string sequence);

	// virtual destructor
	virtual ~sequenceBase();

	// get methods
	size_t size();
	std::string getName(void)const;
	std::string getSequence(void) const;

protected:
	// default constructor
	sequenceBase();

	// write record to stream 
	virtual void write2Stream(std::ostream& stream) const;
	friend std::ostream& operator<< (std::ostream& outStream, const sequenceBase* seq);

	// member
	std::string m_name;
	std::string m_sequence;

private:
};


// -- exported functions - declarations ----------------------------------
std::ostream& operator<< (std::ostream& outStream, const sequenceBase* seq);


// -- exported global variables - declarations (should be empty)----------