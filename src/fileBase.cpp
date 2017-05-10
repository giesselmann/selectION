// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <boost/filesystem.hpp>

//-- private headers -----------------------------------------------------
#include "fileBase.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
fileBase::fileBase
(
	std::string filePath
)
{
	boost::filesystem::path p(filePath);
	m_normPath = p.generic_string();
	m_exists = boost::filesystem::exists(p);
	if (m_exists)
		m_size = boost::filesystem::file_size(m_normPath);
}




// virtual destructor
fileBase::~fileBase()
{

}




// check if file exists
bool 
fileBase::exists(void)
{
	return m_exists;
}




// size in Byte
size_t 
fileBase::size(void)
{
	return m_size;
}




// get normalized path
std::string
fileBase::normPath(void)const
{
	return m_normPath;
}




// get file extension
std::string 
fileBase::extension(void)const
{
	return boost::filesystem::extension(m_normPath);
}




//-- private functions --------- definitions -----------------------------