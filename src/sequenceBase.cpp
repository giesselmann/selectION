// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------

//-- private headers -----------------------------------------------------
#include "sequenceBase.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
sequenceBase::sequenceBase
(
	std::string name,
	std::string sequence
)
{
	m_name = name;
	m_sequence = sequence;
}




// constructor
sequenceBase::sequenceBase()
{

}




// virtual destructor
sequenceBase::~sequenceBase()
{

}




// get methods
size_t 
sequenceBase::size()
{
	return m_sequence.size();
}



std::string 
sequenceBase::getName(void)const
{
	return m_name;
}




std::string 
sequenceBase::getSequence(void)const
{
	return m_sequence;
}




// write record to stream 
void 
sequenceBase::write2Stream
(
	std::ostream& stream
)const
{
	stream << m_sequence << std::endl;
}




std::ostream& operator<< 
(
	std::ostream& outStream, 
	const sequenceBase* seq
)
{
	seq->write2Stream(outStream);
	return outStream;
}




//-- private functions --------- definitions -----------------------------