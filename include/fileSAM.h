// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class fileSAM
//
//  DESCRIPTION   :	Basic implementation for SAM file format interactions
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
#include <ostream>
#include <cstdint>
#include "fileBase.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
struct samRecord
{
public:
	std::string QNAME = "*";
	uint32_t FLAG = 0;
	std::string RNAME = "*";
	uint32_t POS = 0;
	uint32_t MAPQ = 0;
	std::string CIGAR = "*";
	std::string RNEXT = "*";
	uint32_t PNEXT = 0;
	int32_t TLEN = 0;
	std::string SEQ = "*";
	std::string QUAL = "*";
};


enum samFlag
{
	e_multipleSegment = 0x1,
	e_properlyAligned = 0x2,
	e_unmapped = 0x4,
	e_nextSegTemplateUnmapped = 0x8,
	e_reverseComplement = 0x10,
	e_nextSegTemplateReverseComplement = 0x20,
	e_firstSegmentTemplate = 0x40,
	e_lastSegmentTemplate = 0x80,
	e_secondary = 0x100,
	e_filtered = 0x200,
	e_duplicate = 0x400,
	e_supplementary = 0x800
};


struct samHeaderProgram
{
	std::string ID = "none";
	std::string Name = "none";
	std::string Version = "0.0";
	std::string commandLine = "none";
};


struct samHeader
{
public:
	uint32_t majorFormat = 0;
	uint32_t minorFormat = 0;
	std::vector<std::pair<std::string, uint32_t>> sequences;
	samHeaderProgram program;
};


class fileSAM : public fileBase
{
public:
	// constructor
	fileSAM(std::string filePath);

	// virtual destructor
	virtual ~fileSAM();

	// write sam header
	void writeHeader(samHeader header);

	// append record to file
	void append(samRecord& record);
	void append(std::vector<samRecord>& records);

protected:

private:
	// methods 
	// constructor
	fileSAM();

	// Copy constructor must not be used
	fileSAM(const fileSAM& object);

	// Assignment operator must not be used
	const fileSAM& operator=(const fileSAM& rhs);

	// parse record to string
	std::string record2string(samRecord& record);

	// parse string to record
	samRecord string2record(std::string str);

	// member

};


// -- exported functions - declarations ----------------------------------
std::ostream& operator<< (std::ostream& outStream, const fileSAM& file);

// -- exported global variables - declarations (should be empty)----------