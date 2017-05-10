// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class fileFastx
//
//  DESCRIPTION   :	Fasta/ Fastq file reader
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
#include <queue>
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "fileBase.h"
#include "ISequenceFile.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class fileFastx : public ISequenceFile, public fileBase
{
public:
	// constructor
	fileFastx(std::string filePath);

	// virtual destructor
	virtual ~fileFastx();

	// return true if no more sequences available
	bool empty();

	// return next sequence in file, NULL if not available
	std::shared_ptr<sequenceBase> getRecord();

	// return extension for output file writer
	std::string extension() const;

	// return read progress in range 0 - 1.0
	float progress();

	// predicate required storage for all sequences
	size_t predicateMemoryRequirement(void);

protected:

private:
	// methods
	// default constructor
	fileFastx();

	// Copy constructor must not be used
	fileFastx(const fileFastx& object);

	// Assignment operator must not be used
	const fileFastx& operator=(const fileFastx& rhs);

	// read next fasta or fastq record from stream
	std::shared_ptr<sequenceBase> readNextRecord(std::ifstream& fileStream);

	// file reader function
	void fileReader();

	// member
	std::string m_fileExtension;
	std::ifstream m_fileStream;
	std::queue<std::shared_ptr<sequenceBase>> m_records;
	size_t m_bufferSize = 0;
	bool m_readActive = false;
	std::thread m_reader;
	std::mutex m_readMutex;
	std::condition_variable m_readCondition;
};

