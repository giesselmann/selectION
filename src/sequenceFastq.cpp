// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------

//-- private headers -----------------------------------------------------
#include "sequenceFastq.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
sequenceFastq::sequenceFastq
(
	std::string name, 
	std::string sequence, 
	std::string description, 
	std::string quality
	) :sequenceBase(name, sequence)
{
	m_name = name;
	m_description = description;
	m_quality = quality;
}




sequenceFastq::sequenceFastq
(
	std::istream& str
)
{
	if (str.peek() == '@')
	{
		std::string buffer;
		if (std::getline(str, buffer).good())
			m_name = buffer.substr(1);
		if (std::getline(str, buffer).good())
			m_sequence = buffer;
		if (std::getline(str, buffer).good())
			m_description = buffer.substr(1);
		std::getline(str, buffer);
		m_quality = buffer;
	}
}




sequenceFastq::~sequenceFastq()
{

}




// get methods
std::string 
sequenceFastq::getDescription(void)const
{
	return m_description;
}




std::string 
sequenceFastq::getQuality(void)const
{
	return m_quality;
}




// write record to stream 
void
sequenceFastq::write2Stream
(
	std::ostream& stream
)const
{
	stream << "@" << m_name << std::endl;
	stream << getSequence() << std::endl;
	stream << "+" << m_description << std::endl;
	stream << m_quality << std::endl;
}




//-- private functions --------- definitions -----------------------------