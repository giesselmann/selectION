// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class fileBase
//
//  DESCRIPTION   :	Base class for data file input
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
#include <vector>

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class fileBase
{
public:
	// constructor
	fileBase(std::string filePath);

	// virtual destructor
	virtual ~fileBase();

	// check if file exists
	bool exists(void);

	// size in Byte
	size_t size(void);

	// get normalized path
	std::string normPath(void) const;

	// get file extension
	std::string extension(void) const;

protected:

private:
	// methods
	// default constructor
	fileBase();

	// Copy constructor must not be used
	fileBase(const fileBase& object);

	// Assignment operator must not be used
	const fileBase& operator=(const fileBase& rhs);

	// member
	std::string m_normPath;
	bool m_exists = false;
	size_t m_size;
};

