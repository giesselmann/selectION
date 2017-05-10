// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <fstream>
#include <iostream>
#include <sstream>
#include <boost/filesystem.hpp>

//-- private headers -----------------------------------------------------
#include "fileSAM.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
fileSAM::fileSAM
(
	std::string filePath
) :fileBase(filePath)
{

}




// virtual destructor
fileSAM::~fileSAM()
{

}




// write sam header
void 
fileSAM::writeHeader
(
	samHeader header
)
{
	boost::filesystem::path file(normPath());
	std::ofstream outputStream((file).generic_string(), std::ofstream::app);
	// header line
	outputStream << "@HD" << "\t" << "VN:" + std::to_string(header.majorFormat) + "." + std::to_string(header.minorFormat) << std::endl;
	// sequence lines
	for (auto it = header.sequences.begin(); it != header.sequences.end(); ++it)
		outputStream << "@SQ" << "\t" << "SN:" << (*it).first << "\t" << "LN:" << std::to_string((*it).second) << std::endl;
	// program line
	auto prog = header.program;
	if (prog.ID != "default")
		outputStream << "@PG" << "\t" <<
						"ID:" << prog.ID << "\t" <<
						"PN:" << prog.Name << "\t" <<
						"VN:" << prog.Version << "\t" <<
						"CL:" << prog.commandLine << std::endl;
}




// append record to file
void
fileSAM::append
(
	samRecord& record
)
{
	std::string parsed = record2string(record);
	boost::filesystem::path file(normPath());
	std::ofstream outputStream((file).generic_string(), std::ofstream::app);
	outputStream << parsed << std::endl;
	outputStream.flush();
}




void 
fileSAM::append
(
	std::vector<samRecord>& records
)
{
	boost::filesystem::path file(normPath());
	std::ofstream outputStream((file).generic_string(), std::ofstream::app);
	for (auto it = records.begin(); it != records.end(); ++it)
	{
		std::string parsed = record2string(*it);
		outputStream << parsed << std::endl;
	}
	outputStream.flush();
}




//-- private functions --------- definitions -----------------------------
// record to string
std::string 
fileSAM::record2string(samRecord& record)
{
	std::string parsed = "";
	parsed += record.QNAME + "\t" + 
			  std::to_string(record.FLAG) + "\t" + 
			  record.RNAME + "\t" + 
			  std::to_string(record.POS) + "\t" + 
			  std::to_string(record.MAPQ) + "\t" + 
			  record.CIGAR + "\t" + 
			  record.RNEXT + "\t" +
			  std::to_string(record.PNEXT) + "\t" +
			  std::to_string(record.TLEN) + "\t" +
			  record.SEQ + "\t" +
			  record.QUAL;
	return parsed;
}




// parse string to record
samRecord 
fileSAM::string2record
(
	std::string str
)
{
	std::istringstream iss(str);
	std::string token;
	std::vector<std::string> fields;
	while (std::getline(iss, token, '\t'))
		fields.push_back(token);
	samRecord parsed;
	if (fields.size() >= 11)
	{
		parsed.QNAME = fields[0];
		parsed.RNAME = fields[2];
		parsed.CIGAR = fields[5];
		parsed.RNEXT = fields[6];
		parsed.SEQ = fields[9];
		parsed.QUAL = fields[10];
		try
		{
			parsed.FLAG = std::stoi(fields[1]);
			parsed.POS = std::stoi(fields[3]);
			parsed.MAPQ = std::stoi(fields[4]);
			parsed.PNEXT = std::stoi(fields[7]);
			parsed.TLEN = std::stoi(fields[8]);
		}
		catch (std::exception&)
		{
			// no handling, return default field value
		}
	}
	return parsed;
}