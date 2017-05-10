// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <string>
#include <cstdint>
#include <cstddef>
#include <algorithm>

//-- private headers -----------------------------------------------------
#include "sequenceFasta.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const ptrdiff_t FastaLineWidth = 60;

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
sequenceFasta::sequenceFasta
(
	std::string name, 
	std::string sequence, 
	std::vector<std::string> comments
	) :sequenceBase(name, sequence)
{
	m_name = name;
	m_comments = comments;
}




// constructor
sequenceFasta::sequenceFasta
(
	std::istream& str
)
{
	if (str.peek() == '>')
	{
		std::string buffer;
		if (std::getline(str, buffer).good())
			m_name = buffer.substr(1);
		while (str.peek() == ';')
		{
			if (std::getline(str, buffer).good())
				m_comments.push_back(buffer.substr(1));
			else
				break;
		}
		while (str.good() && str.peek() != '>')
		{
			std::getline(str, buffer);
			m_sequence.append(buffer);
		}
	}
}




// virtual destructor
sequenceFasta::~sequenceFasta()
{

}




// get methods
std::string 
sequenceFasta::getName(void)const
{
	return m_name;
}




std::vector<std::string> 
sequenceFasta::getComments(void)const
{
	return m_comments;
}




// write record to stream 
void
sequenceFasta::write2Stream
(
	std::ostream& stream
)const
{
	stream << ">" << m_name << std::endl;
	for (auto it = m_comments.begin(); it != m_comments.end(); ++it)
		stream << ";" << *it << std::endl;
	auto const & seq =  getSequence();
	auto seqIter = seq.begin();
	while (seqIter != seq.end())
	{
		uint32_t line = std::min(FastaLineWidth, std::distance(seqIter, seq.end()));
		stream.write(&*seqIter, line) << std::endl;
		seqIter += line;
	}
}




//-- private functions --------- definitions -----------------------------