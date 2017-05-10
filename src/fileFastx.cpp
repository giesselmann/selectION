// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class sequenceRecordSource
//
//  DESCRIPTION   :	Container class for sequence records
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <iostream>
#include <thread>
#include <chrono>

//-- private headers -----------------------------------------------------
#include "fileFastx.h"
#include "sequenceFasta.h"
#include "sequenceFastq.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const size_t MaxBufferSize = 10000000;
const size_t MinBufferSize = 100000;

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------
std::ifstream::pos_type filesize(std::string filename);

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor
fileFastx::fileFastx
(
	std::string filePath
) :fileBase(filePath)
{
	m_fileStream = std::ifstream(normPath());
	if (m_fileStream.good())
	{
		const auto ext = extension();
		char recordStartIndicator = ' ';
		if (ext == ".fasta" || ext == ".fa")
		{
			m_fileExtension = ".fa";
			recordStartIndicator = '>';
		}			
		else if (ext == ".fastq" || ext == ".fq")
		{
			m_fileExtension = ".fq";
			recordStartIndicator = '@';
		}
		else
			m_fileExtension = "";
		// skip invalid lines
		std::string buffer;
		while (m_fileStream.good() && m_fileStream.peek() != recordStartIndicator)
			std::getline(m_fileStream, buffer);
		if (!m_fileStream.good())
			return;
		// read first sequence in constructor and start reader thread
		if (m_fileExtension == ".fa")
		{
			m_records.push(std::shared_ptr<sequenceBase>(new sequenceFasta(m_fileStream)));
		}
		else if (m_fileExtension == ".fq")
		{
			m_records.push(std::shared_ptr<sequenceBase>(new sequenceFastq(m_fileStream)));
		}
		m_bufferSize += m_records.front()->size();
		// check for additional valid records
		while (m_fileStream.good() && m_fileStream.peek() != recordStartIndicator)
			std::getline(m_fileStream, buffer);
		if (!m_fileStream.good())
			return;
		else
		{
			m_readActive = true;
			m_reader = std::thread(&fileFastx::fileReader, this);
		}
	}
	else
		throw std::invalid_argument("Can't read " + filePath);
}




// virtual destructor
fileFastx::~fileFastx()
{
	m_readActive = false;
	m_readCondition.notify_one();
	if (m_reader.joinable())
		m_reader.join();
	
}




// return true if no more sequences available
bool 
fileFastx::empty()
{
	std::unique_lock<std::mutex> lock(m_readMutex);
	if (m_records.empty())
		if (m_readActive)
			return false;
		else
			return true;
	else
		return false;
}




// return next sequence in file, NULL if not available
std::shared_ptr<sequenceBase> 
fileFastx::getRecord()
{
	if (this->empty())
		return std::shared_ptr<sequenceBase>();
	else
	{
		while (m_records.empty())
		{
			// block until next record available
			m_readCondition.notify_one();
			std::unique_lock<std::mutex> lock(m_readMutex);
			m_readCondition.wait(lock);
		}
		std::shared_ptr<sequenceBase> record;
		{
			std::unique_lock<std::mutex> lock(m_readMutex);
			record = m_records.front();
			m_records.pop();
			m_bufferSize -= (*record).size();
		}
		if (m_bufferSize <= MinBufferSize)
			m_readCondition.notify_one();
		return record;
	}
}




// return extension for output file writer
std::string 
fileFastx::extension() 
const
{
	return fileBase::extension();
}




// return read progress in range 0 - 1.0
float 
fileFastx::progress() 
{
	float p = 0;
	try
	{
		p = static_cast<float>(m_fileStream.tellg()) / fileBase::size();
	}
	catch (...)
	{

	}
	return p;
}




// predicate required storage for all sequences
size_t 
fileFastx::predicateMemoryRequirement
(
	void
)
{
	if (m_fileExtension == ".fq")
		return filesize(normPath()) / 2;
	else
		return filesize(normPath());
}




//-- private functions --------- definitions -----------------------------
// file reader function
void 
fileFastx::fileReader
(

)
{
	char recordStartIndicator;
	if (m_fileExtension == ".fa")
		recordStartIndicator = '>';
	else
		recordStartIndicator = '@';
	while (m_readActive)
	{
		// wait for signal
		{
			std::unique_lock<std::mutex> lock(m_readMutex);
			m_readCondition.wait(lock);
		}
		while (m_fileStream.good() && m_bufferSize <= MaxBufferSize)
		{
			// read next record from input file
			std::shared_ptr<sequenceBase> record;		
			if (recordStartIndicator == '>')
				record = std::shared_ptr<sequenceBase>(new sequenceFasta(m_fileStream));
			else
				record = std::shared_ptr<sequenceBase>(new sequenceFastq(m_fileStream));
			// skip lines beginning with invalid characters
			std::string buffer;
			while (m_fileStream.good() && m_fileStream.peek() != recordStartIndicator)
				std::getline(m_fileStream, buffer);
			{
				std::unique_lock<std::mutex> lock(m_readMutex);
				m_records.push(record);
				m_bufferSize += (*record).size();
				if (!m_fileStream.good())
					m_readActive = false;
			}// lock destroyed by closing block
			m_readCondition.notify_one();
		}
	}
}




std::ifstream::pos_type filesize(std::string filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}