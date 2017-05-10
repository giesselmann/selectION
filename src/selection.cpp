// \MODULE\---------------------------------------------------------------
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
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <cstdint>
#include <map>
#include <cmath>
#include <boost/filesystem.hpp>

//-- private headers -----------------------------------------------------
#include "fileFastx.h"
#include "selection.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const size_t SeqWriteBufferSize = 0;
const size_t SamWriteBufferSize = 499;

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// construct using existing index on disk
selectION::selectION
(
	selection_settings& settings,
	std::string dbPrefix
) : m_settings(settings)
{
	m_index = new fmIndex(settings.get_m_fmIndex_settings(), dbPrefix + ".h5");
	m_aligner = new pseudoAligner(settings.get_m_pseudoAligner_settings(), *m_index);
	m_settings.logging().log(e_logInfo, "SelectION instance created");	
}




// virtual destructor
selectION::~selectION()
{
	delete m_aligner;
	delete m_index;
}




// build from fasta/ fastq
void
selectION::buildFromFastx
(
	selection_settings& settings,
	std::string filename
)
{
	buildFromFastx(settings, filename, filename);
}




// build from fasta/ fastq
void
selectION::buildFromFastx
(
	selection_settings& settings,
	std::string fileName,
	std::string dbPrefix
)
{
	// get size of file (ignore header and newline overhead)
	std::string refSequence;
	fileFastx reader(fileName);
	try
	{
		fmIndex::allocateMemory(refSequence, reader.predicateMemoryRequirement());
	}
	catch (std::exception e)
	{
		settings.logging().log(e_logFatal, "Failed to allocate memory for reference sequence");
		return;
	}	
	// names and offsets of chapters
	std::map<uint32_t, std::string> nameOffset;
	uint32_t offset = 0;
	settings.logging().log(e_logInfo, "Loaded reference " + fileName);
	while (!reader.empty())
	{
		auto record = reader.getRecord();
		if (record == NULL)
			break;
		std::string name;
		size_t pos = record->getName().find(' ', 1);
		if (pos != std::string::npos)
			name = record->getName().substr(0, pos);
		else
			name = record->getName().substr(0);
		nameOffset[offset] = name;
		offset += static_cast<uint32_t>(record->size());
		refSequence.append(record->getSequence());
	}

	// build index
	if (refSequence.size() > 0)
	{
		settings.logging().log(e_logInfo, "Building Index for " +
										  std::to_string(nameOffset.size()) +
										  " segments (" +
										  std::to_string(offset) + " Bp)");
		fmIndex::build(settings.get_m_fmIndex_settings(), refSequence, dbPrefix + ".h5", nameOffset);
	}
	else
		settings.logging().log(e_logError, "No reference sequence found. Specify valid fastq or fasta input file");
}




// scan sequence file for matches
void 
selectION::select
(
	ISequenceFile& seqFile, 
	positionFilter& filter,
	std::string outputPath
)
{
	if (!boost::filesystem::is_directory(outputPath))
	{
		m_settings.logging().log(e_logError, "Output path is not a directory");
		return;
	}
	if (m_settings.m_sam != "" && boost::filesystem::is_directory(m_settings.m_sam))
	{
		m_settings.logging().log(e_logError, "SAM Output path is a directory");
		return;
	}
	if (m_settings.m_sam != "")
		m_samOutput = true;
	m_writeActive = true;
	auto writer = std::thread(&selectION::writeWorker, this, outputPath, seqFile.extension());
	std::vector<std::thread> worker;
	for (uint32_t i = 1; i < m_settings.m_threads; i++)
		worker.push_back(std::thread(&selectION::selectionWorker, this, std::ref(seqFile), std::ref(filter)));
	selectionWorker(seqFile, filter);
	for (auto it = worker.begin(); it != worker.end(); ++it)
		(*it).join();
	m_writeActive = false;
	m_writeCondition.notify_all();
	writer.join();
}





//-- private functions --------- definitions -----------------------------
// worker function reading records from disk, aligning and writing back
void 
selectION::selectionWorker
(
	ISequenceFile& seqs,
	positionFilter& filter
)
{
	const auto qualityThreshold = m_settings.m_qualityThreshold;
	const auto activeSelectors = filter.getActiveSelectorCount();
	for (;;)
	{
		std::shared_ptr<sequenceBase> record = NULL;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if (!seqs.empty())
				record = seqs.getRecord();
		}
		if (record == NULL)
			break;
		auto position = m_aligner->estimatePosition(record->getSequence());
		auto recordName = record->getName();
		position.QNAME = recordName.substr(0, recordName.find(' '));;
		bool notify = false;
		// write pseudo alignment to sam output file
		if (m_samOutput)
		{
			position.TLEN = record->size();
			if (position.MAPQ < qualityThreshold)
				position.FLAG |= samFlag::e_unmapped;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_samRecords.push_back(position);
			}
			if (m_samRecords.size() > SamWriteBufferSize)
				notify = true;
		}		
		// write matched records to output directory
		if (activeSelectors > 0 && position.MAPQ >= qualityThreshold)
		{
			auto matches = filter.match(position.RNAME, position.POS, static_cast<uint32_t>(record->size()));
			if (matches.size() == 0)
				continue;
			std::lock_guard<std::mutex> lock(m_mutex);
			for (auto it = matches.begin(); it != matches.end(); ++it)
			{
				m_selected[(*it)].push_back(record);
			}
			if (m_selected.size() > SeqWriteBufferSize)
				notify = true;
		}
		// notify file writer thread
		if (notify == true)
			m_writeCondition.notify_one();
	}
	m_writeCondition.notify_one();
}




// worker function writing selected records to stream or disk
void 
selectION::writeWorker
(
	std::string outputPath,
	std::string fileExtension
)
{
	boost::filesystem::path dir(outputPath);
	boost::filesystem::path samFile(m_settings.m_sam);
	fileSAM samOut(samFile.generic_string());
	samHeaderProgram hp;
	hp.ID = "SelectION";
	hp.Name = "SelectION";
	hp.Version = "1.0";
	hp.commandLine = m_settings.m_cmd;
	auto chapters = m_index->getChapters();
	std::sort(chapters.begin(), chapters.end(),
				[](std::pair<std::string, uint32_t> a, std::pair<std::string, uint32_t> b)
				{ return a.second > b.second; });
	samHeader h;
	h.program = hp;
	h.majorFormat = 1;
	h.minorFormat = 0;
	h.sequences = chapters;
	samOut.writeHeader(h);
	while (m_writeActive)
	{
		std::map<std::string, std::vector<std::shared_ptr<sequenceBase>>> recordBuffer;
		std::vector<samRecord> samBuffer;
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			// pause thread
			m_writeCondition.wait(lock);
			// lock re-aquired, save to copy
			recordBuffer = std::move(m_selected);
			samBuffer = std::move(m_samRecords);
		}
		// write selected records if existent
		if (recordBuffer.size())
		{
			uint32_t recordsComplete = 0;
			bool success = true;
			for (auto it = recordBuffer.begin(); it != recordBuffer.end(); ++it)
			{
				try
				{
					boost::filesystem::path file = (*it).first + fileExtension;				
					std::ofstream outputStream((dir / file).generic_string(), std::ofstream::app);
					for (auto it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
					{
						outputStream << (*it2);
						recordsComplete++;
					}			
				}
				catch (std::exception&)
				{
					success = false;
					// TODO handle exceptions on output
				}
			}
			if (success)
				m_settings.logging().log(e_logInfo, "Successfully wrote " + 
					std::to_string(recordsComplete) + 
					" record to disk.");
			else
				m_settings.logging().log(e_logError, "Error writing sequence record");
		}	
		// write sam records if existent
		if (samBuffer.size())
		{
			try
			{
				samOut.append(samBuffer);
				m_settings.logging().log(e_logInfo, "Successfully wrote " +
					std::to_string(samBuffer.size()) +
					" records to sam.");
			}
			catch (std::exception&)
			{
				m_settings.logging().log(e_logError, "Error writing sam record");
				// TODO handle exceptions on output
			}
		}
	}
}