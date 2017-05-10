// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Interface sequenceFile
//
//  DESCRIPTION   :	Interface implemented by class providing DNA sequences
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
#include <memory>
#include <string>
#include "sequenceBase.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------

class ISequenceFile
{
public:
	// constructor
	ISequenceFile(){};

	// virtual destructor
	virtual ~ISequenceFile(){};

	// return true if no more sequences available
	virtual bool empty() = 0;

	// return next sequence in file, NULL if not available
	virtual std::shared_ptr<sequenceBase> getRecord() = 0;

	// return extension for output file writer
	virtual std::string extension() const = 0;

	// return read progress in range 0 - 1.0
	virtual float progress() = 0;

protected:

private:
	// methods
	// Copy constructor must not be used
	ISequenceFile(const ISequenceFile& object);

	// Assignment operator must not be used
	const ISequenceFile& operator=(const ISequenceFile& rhs);
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------