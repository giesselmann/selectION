// \HEADER\---------------------------------------------------------------
//
//  CONTENTS      : Class selectION
//
//  DESCRIPTION   :	Use pseudo-alignment to filter reads from fastq files
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
#include <mutex>
#include "selection_settings.h"
#include "fmIndex.h"
#include "pseudoAligner.h"
#include "fileSAM.h"
#include "ISequenceFile.h"
#include "positionFilter.h"

// -- forward declarations -----------------------------------------------

// -- exported constants, types, classes ---------------------------------
class selectION
{
public:
	// construct using existing index on disk
	selectION(selection_settings& settings, std::string dbPrefix);

	// virtual destructor
	virtual ~selectION();

	// build from fasta or fastq
	static void buildFromFastx(selection_settings& settings, std::string fileName);
	static void buildFromFastx(selection_settings& settings, std::string fileName, std::string dbPrefix);

	// scan sequence file for matches, output to one file per selector
	void select(ISequenceFile& seqFile, positionFilter& filter, std::string outputPath);

protected:

private:
	// methods
	// default constructor
	selectION();

	// Copy constructor must not be used
	selectION(const selectION& object);

	// Assignment operator must not be used
	const selectION& operator=(const selectION& rhs);

	// worker function reading records from disk, aligning and writing back
	void selectionWorker(ISequenceFile& seqs, positionFilter& filter);

	// worker function writing selected records to disk or stream
	void writeWorker(std::string outputPath, std::string fileExtension);

	// member
	selection_settings m_settings;
	std::mutex m_mutex;
	std::condition_variable m_writeCondition;
	bool m_writeActive;
	fmIndex* m_index = NULL;
	pseudoAligner* m_aligner = NULL;
	std::map<std::string, std::vector<std::shared_ptr<sequenceBase>>> m_selected;
	bool m_samOutput = false;
	std::vector<samRecord> m_samRecords;
};


// -- exported functions - declarations ----------------------------------

// -- exported global variables - declarations (should be empty)----------