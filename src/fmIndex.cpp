// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Class FM-Index
//
//  DESCRIPTION   :	FM-Index
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
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <climits>

//-- private headers -----------------------------------------------------
#include "fmIndex.h"
#include "suffixArray.h"
#include "fileFastx.h"
using namespace H5;

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------
const uint32_t BwtLastDiskChunkSize = 1024000;			// Size of compressed chunks in hdf5 file


const struct DatasetNames
{
	const std::string m_bwtFirst = "FirstColumn";
	const std::string m_bwtLast = "LastColumn";
	const std::string m_tally = "Tally";
	const std::string m_suffixArraySample = "SuffixArraySample";
	const std::string m_suffixArray = "SuffixArray";
}DatasetNames;


const struct GroupNames
{
	const std::string m_Index = "Index";
	const std::string m_Subsequences = "Subsequences";
}GroupNames;


//-- private types -------------------------------------------------------
typedef struct indexValuePair
{
	uint32_t index;
	uint32_t value;
}indexValuePair;


typedef struct IndexFileFirstColumn
{
	char chr;
	uint32_t count;
}IndexFileFirstColumn;


//-- private functions --------- declarations ----------------------------
void bwtFromSA(const std::string& S, std::vector<uint32_t> const& sa, std::string& bwt, size_t bwtOffset);
herr_t getGroupDatasetNames(hid_t loc_id, const char* name, const H5L_info_t *linfo, void* opdata);

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
// constructor loading index from disk
fmIndex::fmIndex
(
	fmIndex_settings& settings,
	std::string indexFileName	
) : m_settings(settings)
{
	init();
	load(indexFileName);
}




// virtual destructor
fmIndex::~fmIndex()
{
	this->clearDataStructures();
}




// allocate memory of input string
void 
fmIndex::allocateMemory
(
	std::string& str, 
	uint32_t expectedSize
)
{
	// reserve memory for string and appending $ at the end
	str.reserve(expectedSize + 1);
}




// build index
void 
fmIndex::build
(
	fmIndex_settings& settings,
	std::string& str,
	std::string outputFilename
)
{
	fmIndex workingIndex;
	str.append("$");
	workingIndex.m_settings = settings;
	workingIndex.buildIndex(str, outputFilename);
}




// build index using chapters
void 
fmIndex::build
(
	fmIndex_settings& settings,
	std::string& str, 
	std::string outputFilename, 
	std::map<uint32_t, std::string> chapters
)
{
	fmIndex workingIndex;
	str.append("$");
	workingIndex.m_settings = settings;
	workingIndex.m_nameOffset = chapters;
	workingIndex.buildIndex(str, outputFilename);
}




// load existing index from disk
void 
fmIndex::load
(
	std::string indexFile
)
{
	clearDataStructures();
	m_nameOffset.clear();
	Exception::dontPrint();
	try
	{
		// open file
		H5File file(indexFile, H5F_ACC_RDONLY);
		auto grpSubsequences = file.openGroup(GroupNames.m_Subsequences);
		std::vector<std::string> subsequenceNames;
		H5Literate(grpSubsequences.getId(),
					H5_INDEX_NAME, H5_ITER_INC, NULL,
					getGroupDatasetNames, &subsequenceNames);

		// read settings from root attributes
		uint32_t attr_data[1];
		Attribute attr = file.openAttribute("suffixSample");
		attr.read(PredType::NATIVE_UINT32, &attr_data);
		m_settings.set_m_saSampleStepSize(attr_data[0]);
		attr = file.openAttribute("tallyStep");
		attr.read(PredType::NATIVE_UINT32, &attr_data);
		m_settings.set_m_tallyStepSize(attr_data[0]);

		// read names and offsets of subsequences
		for (auto it = subsequenceNames.begin(); it != subsequenceNames.end(); ++it)
		{
			auto dst = grpSubsequences.openDataSet(*it);
			uint32_t offset;
			hsize_t dims[] = { 1 };
			DataSpace dataspace(1, dims);
			dst.read(&offset, PredType::NATIVE_UINT32, dataspace);
			m_nameOffset[offset] = *it;
		}

		// read bwtFirst
		auto grpIndex = file.openGroup(GroupNames.m_Index);
		auto dataset = grpIndex.openDataSet(DatasetNames.m_bwtFirst);
		auto dataspace = H5Dget_space(dataset.getId());
		hsize_t dim;
		H5Sget_simple_extent_dims(dataspace, &dim, NULL);
		auto bwtFirstData = std::vector<IndexFileFirstColumn>(dim);
		dataset.read((void*)&*bwtFirstData.begin(), m_fileDataTypes[DatasetNames.m_bwtFirst]);
		uint8_t charIndex = 0;
		for (auto it = bwtFirstData.begin(); it != bwtFirstData.end(); ++it)
		{
			m_alphabet.push_back((*it).chr);
			m_charIndex[(*it).chr] = charIndex++;
			m_bwtFirst.push_back((*it).count);
		}

		// read bwtLast
		dataset = grpIndex.openDataSet(DatasetNames.m_bwtLast);
		dataspace = H5Dget_space(dataset.getId());
		H5Sget_simple_extent_dims(dataspace, &dim, NULL);
		m_bwtLast.resize(dim);				// may throw bad_alloc
		dataset.read((void*)&*m_bwtLast.begin(), m_fileDataTypes[DatasetNames.m_bwtLast]);
		m_N = dim;

		// read suffix array sample
		dataset = grpIndex.openDataSet(DatasetNames.m_suffixArraySample);
		dataspace = H5Dget_space(dataset.getId());
		H5Sget_simple_extent_dims(dataspace, &dim, NULL);
		m_suffixArraySample.resize(dim);	// may throw bad_alloc
		dataset.read((void*)&*m_suffixArraySample.begin(), m_fileDataTypes[DatasetNames.m_suffixArraySample]);

		// read suffix array
		//dataset = grpIndex.openDataSet(DatasetNames.m_suffixArray);
		//dataspace = H5Dget_space(dataset.getId());
		//H5Sget_simple_extent_dims(dataspace, &dim, NULL);
		//m_suffixArray.resize(dim);	// may throw bad_alloc
		//dataset.read((void*)&*m_suffixArray.begin(), m_fileDataTypes[DatasetNames.m_suffixArray]);
		
		// read tally
		initTallyDatatype(m_alphabet);
		m_tally.resize(m_alphabet.size());
		dataset = grpIndex.openDataSet(DatasetNames.m_tally);
		dataspace = H5Dget_space(dataset.getId());
		H5Sget_simple_extent_dims(dataspace, &dim, NULL);
		auto tallyIter = m_tally.begin();
		auto alphaIter = m_alphabet.begin();
		size_t columnOffset = 0;
		for (; tallyIter != m_tally.end() && alphaIter != m_alphabet.end(); ++tallyIter, ++alphaIter)
		{
			CompType colSubType(sizeof(uint32_t));
			colSubType.insertMember(std::string(1, *alphaIter), 0, PredType::NATIVE_UINT32);
			(*tallyIter).resize(dim);
			dataset.read(&*(*tallyIter).begin(), colSubType);
			columnOffset += sizeof(uint32_t);
		}
	}
	catch (Exception& ex)
	{
		if (ex.getDetailMsg().find("H5Fopen") != std::string::npos)
			throw std::invalid_argument("Index " + indexFile + " not found");
		else
			throw std::invalid_argument("Index " + indexFile + " corrupted");
	}
	// memory allocation exceptions are left untouched
}




// return complete index sequence
std::string
fmIndex::getIndexSequence(void)
{
	std::string ref;
	ref.reserve(m_N);
	char currentChar = '$';
	uint32_t currentRow = 0;
	for (uint32_t i = 0; i < m_N; i++)
	{
		currentChar = m_bwtLast[currentRow];
		ref.append(1,currentChar);
		uint32_t currentCharCount = getCount(currentChar, currentRow);
		currentRow = getRowFromRank(currentChar, currentCharCount - 1);
	}
	std::reverse(ref.begin(), ref.end());
	return ref;
}




// get substr of index sequence
std::string
fmIndex::getIndexSequence
(
	uint32_t startIdx, 
	uint32_t length
)
{
	if (startIdx + length >= m_N)
		throw std::out_of_range("Requested string out of index range");
	// suffix array sample is sorted by index, 
	// have to find checkpoint the hard way
	uint32_t lokkupStartIdx = startIdx + length;
	uint32_t checkPointIdx;
	checkPointIdx = lokkupStartIdx + (m_settings.m_saSampleStepSize - lokkupStartIdx % m_settings.m_saSampleStepSize);
	uint32_t currentRow = 0;
	if (checkPointIdx < m_N)
	{
		auto it = m_suffixArraySample.cbegin();
		while (it != m_suffixArraySample.cend() && (*it).value != checkPointIdx)
			it++;
		currentRow = (*it).index;
	}
	// reconstruct ref string from bwt
	std::string index;
	index.reserve(checkPointIdx - startIdx + 1);
	for (uint32_t i = checkPointIdx - 1; i > startIdx; i--)
	{
		char currentChar = m_bwtLast[currentRow];
		index.append(1, currentChar);
		uint32_t currentCharCount = getCount(currentChar, currentRow);
		currentRow = getRowFromRank(currentChar, currentCharCount - 1);	
	}
	std::reverse(index.begin(), index.end());
	return index.substr(0, length);
}




// return vector containing all occurences of pattern in index
std::vector<uint32_t> 
fmIndex::getMatchingPositions
(
	std::string const & pattern
)
{
	return getMatchingPositions(pattern, UINT32_MAX);
}




// limit matching positions for performance reasons
std::vector<uint32_t>
fmIndex::getMatchingPositions
(
	std::string const & pattern, 
	const uint32_t maxResults
)
{
	// range for last character in pattern
	int64_t suffixStart = pattern.size() - 1;
	uint32_t rowStart = getRowFromRank(pattern[suffixStart], 0);
	uint32_t rowStop = rowStart + m_bwtFirst[m_charIndex[pattern[suffixStart]]];
	suffixStart--;
	// range for following characters
	while (suffixStart >= 0 && rowStop > rowStart)
	{
		const char currentChar = pattern[suffixStart];
		uint32_t startCount = 0;
		if (rowStart > 0)
			startCount = getCount(currentChar, rowStart - 1);
		uint32_t stopCount = getCount(currentChar, rowStop - 1);
		if (stopCount > startCount)
		{
			uint32_t startRank = startCount;
			uint32_t stopRank = stopCount;
			rowStart = getRowFromRank(currentChar, startRank);
			rowStop = getRowFromRank(currentChar, stopRank);
			suffixStart--;
		}
		else
			break;
	}
	// get positions of matches
	std::vector<uint32_t> results;
	if (maxResults >= rowStop - rowStart)
	{
		results.reserve(rowStop - rowStart);
		for (auto i = rowStart; i < rowStop; i++)
		{
			auto position = getPositionFromRow(i);
			if (position >= 0)
				results.push_back(position);
		}
		std::sort(results.begin(), results.end());
	}
	return results;
}




// get position of longest common subsequence (lcs)
std::vector<lcsDefinition>
fmIndex::getLongestCommonSubsequence
(
	std::string const & pattern
)
{
	int64_t prefixEnd = pattern.size() - 1;
	int64_t lcsLength = 0;
	int64_t lcsReadPos = 0;
	int64_t lcsStartRow = 0, lcsStopRow = 0;
	// ckeck for each prefix of str
	while (prefixEnd > 0 && prefixEnd > lcsLength)
	{
		// range for last character in pattern
		int64_t suffixStart = prefixEnd;
		uint32_t rowStart = getRowFromRank(pattern[suffixStart], 0);
		uint32_t rowStop = rowStart + m_bwtFirst[m_charIndex[pattern[suffixStart]]];
		suffixStart--;
		// range for following characters
		while (suffixStart >= 0 && rowStop > rowStart)
		{
			const char currentChar = pattern[suffixStart];
			uint32_t startCount = 0;
			if (rowStart > 0)
				startCount = getCount(currentChar, rowStart - 1);
			uint32_t stopCount = getCount(currentChar, rowStop - 1);
			if (stopCount > startCount)
			{
				uint32_t startRank = startCount;
				uint32_t stopRank = stopCount;
				rowStart = getRowFromRank(currentChar, startRank);
				rowStop = getRowFromRank(currentChar, stopRank);
				suffixStart--;
			}
			else
				break;
		}
		if (prefixEnd - suffixStart > lcsLength && rowStart < rowStop)
		{
			lcsLength = prefixEnd - suffixStart;
			lcsStartRow = rowStart;
			lcsStopRow = rowStop;
			lcsReadPos = suffixStart + 1;
		}
		prefixEnd--;
	}
	std::vector<lcsDefinition> results;
	for (auto i = lcsStartRow; i < lcsStopRow; i++)
	{
		auto indexStart = getPositionFromRow(i);
		if (indexStart >= 0)
		{
			lcsDefinition result;
			result.indexStart = getPositionFromRow(i);
			result.strStart = lcsReadPos;
			result.lcsLength = lcsLength;
			results.push_back(result);
		}
	}
	return results;
}




// return name and relative position in chapter
std::pair<std::string, uint32_t> 
fmIndex::getRelativePosition
(
	uint32_t absolutPosition
)
{
	auto it = m_nameOffset.lower_bound(absolutPosition);
	if (it != m_nameOffset.begin() && (*it).first != absolutPosition)
		it = std::prev(it);	
	return std::make_pair((*it).second, absolutPosition - (*it).first);
}




// optionally return names and lengths of chapters in order
std::vector<std::pair<std::string, uint32_t>> 
fmIndex::getChapters
(
	void
)
{
	std::vector<std::pair<std::string, uint32_t>> chapters;
	size_t totalLength = m_bwtLast.size();
	for (auto it = m_nameOffset.begin(); it != m_nameOffset.end(); ++it)
	{
		auto it_next = std::next(it);
		if (it_next != m_nameOffset.end())
			chapters.push_back(std::make_pair((*it).second, (*it_next).first - (*it).first));
		else
			chapters.push_back(std::make_pair((*it).second, totalLength - (*it).first));
	}
	return chapters;
}




//-- private functions --------- definitions -----------------------------
// constructor
fmIndex::fmIndex
() : m_charIndex(256, 0)
{
	init();
}




// init
void
fmIndex::init
(

)
{
	// init static file datatypes
	// m_bwtLast
	m_fileDataTypes[DatasetNames.m_bwtLast] = PredType::NATIVE_CHAR;
	// m_bwtFirst
	CompType memType(sizeof(IndexFileFirstColumn));
	memType.insertMember("char", HOFFSET(IndexFileFirstColumn, chr), PredType::NATIVE_UINT8);
	memType.insertMember("count", HOFFSET(IndexFileFirstColumn, count), PredType::NATIVE_UINT32);
	m_fileDataTypes[DatasetNames.m_bwtFirst] = memType;
	// m_suffixArraySample
	memType = CompType(sizeof(indexValuePair));
	memType.insertMember("index", HOFFSET(indexValuePair, index), PredType::NATIVE_UINT32);
	memType.insertMember("value", HOFFSET(indexValuePair, value), PredType::NATIVE_UINT32);
	m_fileDataTypes[DatasetNames.m_suffixArraySample] = memType;
	// m_suffixArray
	m_fileDataTypes[DatasetNames.m_suffixArray] = PredType::NATIVE_UINT32;
	// add default chapter
	m_nameOffset[0] = "default";
}




// build index
void 
fmIndex::buildIndex
(
	std::string& str, 
	std::string outputFilename
)
{
	m_N = str.size();
	suffixArray sa(str, m_settings.m_threads);
	clearDataStructures();
	// compute first column of bwt matrix and init index lookup
	auto alphabet = sa.getAlphabet();
	auto alphabetCount = sa.getAlphabetCount();
	uint8_t alphabetSize = 0;
	auto it1 = alphabet.begin();
	auto it2 = alphabetCount.begin();
	for (; it1 != alphabet.end() && it2 != alphabetCount.end(); ++it1, ++it2)
	{
		if ((*it1) != '$')
		{
			m_charIndex[*it1] = alphabetSize++;
			m_bwtFirst.push_back(*it2);
			m_alphabet.push_back(*it1);
		}
	}

	// clear old ranks
	m_tally.resize(alphabetSize);

	// start output file writer
	m_writeActive = true;
	m_writeBlock = false;
	auto writeThread = std::thread(&fmIndex::fileWriter, this, outputFilename);

	// prepare suffix array segment stream
	auto blockSize = sa.prepareForStream(m_settings.m_MaxSuffixMemoryBlock);
	m_settings.logging().log(e_logInfo, "Suffix-array stream prepared, max block is " + std::to_string(blockSize));

	// reserve memory
	m_bwtLast.reserve(blockSize);
	m_suffixArraySample.reserve(blockSize / m_settings.m_saSampleStepSize + 1);
	for (auto it = m_tally.begin(); it != m_tally.end(); ++it)
		(*it).reserve(blockSize / m_settings.m_tallyStepSize + 1);

	// stream FM-Index to disk
	size_t complete = 0;
	size_t lastCompletePrint = 0;
	for (;;)
	{
		auto const & sfx = sa.getNextSegment();
		if (sfx.size() == 0)
			break;
		// wait for pending write operations
		m_writeBlock = false;
		std::unique_lock<std::mutex> lock(m_writeMutex);

		// process chunk of suffix array
		processChunk(str, sfx, complete);
		// m_suffixArray.insert(m_suffixArray.end(), sfx.begin(), sfx.end());
		// notify file writer thread
		m_writeBlock = true;
		lock.unlock();
		m_writeCondition.notify_one();
		complete += sfx.size();
		if (complete > lastCompletePrint + m_N / 100)
		{
			m_settings.logging().log(e_logInfo, "Completed " + std::to_string(complete) + " / " + std::to_string(m_N));
			lastCompletePrint = complete;
		}		
	}
	if (lastCompletePrint != complete)
		m_settings.logging().log(e_logInfo, "Completed " + std::to_string(complete) + " / " + std::to_string(m_N));

	// finalize output file
	m_writeBlock = true;
	m_writeActive = false;
	m_writeCondition.notify_one();
	writeThread.join();

	// clear names for the case the class is reused
	m_nameOffset.clear();
	m_nameOffset[0] = "default";
}




// output file writer
void 
fmIndex::fileWriter
(
	std::string outputFilename
)
{
	// create new file, overwrite if existent
	H5File file(outputFilename, H5F_ACC_TRUNC);

	// save settings as attributes in root
	hsize_t attr_dims[1] = {1};
	uint32_t attr_data[1];
	DataSpace attr_dataspace = DataSpace(1, attr_dims);
	Attribute attr = file.createAttribute("suffixSample", PredType::NATIVE_UINT32, attr_dataspace);
	attr_data[0] = m_settings.get_m_saSampleStepSize();
	attr.write(PredType::NATIVE_UINT32, attr_data);
	attr = file.createAttribute("tallyStep", PredType::NATIVE_UINT32, attr_dataspace);
	attr_data[0] = m_settings.get_m_tallyStepSize();
	attr.write(PredType::NATIVE_UINT32, attr_data);

	// init compound datatype for tally
	initTallyDatatype(m_alphabet);

	// write name and offset of sequences
	auto grpSubsequences = file.createGroup(GroupNames.m_Subsequences);
	for (auto it = m_nameOffset.begin(); it != m_nameOffset.end(); ++it)
	{
		hsize_t dims[1]{1};
		DataSpace dataspace(1, dims);
		DataSet dataset = grpSubsequences.createDataSet((*it).second, PredType::NATIVE_UINT32, dataspace);
		dataset.write((void*)&(*it).first, PredType::NATIVE_UINT32);
	}

	// write first column of bwt matrix
	auto bwtFirstData = std::vector<IndexFileFirstColumn>();
	auto it1 = m_bwtFirst.begin();
	auto it2 = m_alphabet.begin();
	for (; it1 != m_bwtFirst.end() && it2 != m_alphabet.end(); ++it1, ++it2)
	{
		IndexFileFirstColumn item;
		item.chr = *it2;
		item.count = *it1;
		bwtFirstData.push_back(item);
	}
	hsize_t dims[] = { m_bwtFirst.size() };
	hsize_t chunkDims[] = { BwtLastDiskChunkSize < m_N / 10 ? BwtLastDiskChunkSize : m_N };
	DataSpace dataspace(1, dims);
	auto grpIndex = file.createGroup(GroupNames.m_Index);
	DataSet dst_bwtFirst = grpIndex.createDataSet(DatasetNames.m_bwtFirst, 
												  m_fileDataTypes[DatasetNames.m_bwtFirst], 
												  dataspace);
	dst_bwtFirst.write((void*)&*(bwtFirstData.begin()), m_fileDataTypes[DatasetNames.m_bwtFirst]);

	// init datasets for stream writing
	dims[0] = m_N;
	dataspace = DataSpace(1, dims);
	DSetCreatPropList properties;
	properties.setChunk(1, chunkDims);
	properties.setDeflate(3);
	DataSet dst_bwtLast = grpIndex.createDataSet(DatasetNames.m_bwtLast, 
												 m_fileDataTypes[DatasetNames.m_bwtLast], 
												 dataspace, properties);
	m_N % m_settings.m_saSampleStepSize == 0 ? dims[0] = m_N / m_settings.m_saSampleStepSize : 
											   dims[0] = m_N / m_settings.m_saSampleStepSize + 1;
	dataspace = DataSpace(1, dims);
	DataSet dst_suffixArraySample = grpIndex.createDataSet(DatasetNames.m_suffixArraySample, 
														   m_fileDataTypes[DatasetNames.m_suffixArraySample], 
														   dataspace);
	m_N % m_settings.m_tallyStepSize == 0 ? dims[0] = m_N / m_settings.m_tallyStepSize : 
											dims[0] = m_N / m_settings.m_tallyStepSize + 1;
	dataspace = DataSpace(1, dims);
	DataSet dst_tally = grpIndex.createDataSet(DatasetNames.m_tally,
											   m_fileDataTypes[DatasetNames.m_tally], 
											   dataspace);
	size_t tallyPosition = 0;
	size_t suffixArraySamplePosition = 0;
	size_t bwtLastPosition = 0;
	hsize_t offset[1], count[1], stride[1], block[1];
	stride[0] = 1;
	block[0] = 1;
	DataSpace memspace;
	while (m_writeActive)
	{
		// wait for signal
		{
			std::unique_lock<std::mutex> lock(m_writeMutex);
			m_writeCondition.wait(lock);
		}
		if (m_writeBlock) // secure against spurious wakeup
		{
			volatile std::unique_lock<std::mutex> lock(m_writeMutex);
			// write bwt_Last
			offset[0] = bwtLastPosition;
			count[0] = m_bwtLast.size();
			dims[0] = m_bwtLast.size();
			memspace = DataSpace(1, dims, NULL);
			dataspace = dst_bwtLast.getSpace();
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
			dst_bwtLast.write((void*)&*m_bwtLast.begin(), 
							  m_fileDataTypes[DatasetNames.m_bwtLast], 
							  memspace, dataspace);
			bwtLastPosition += m_bwtLast.size();
			m_bwtLast.clear();
			// write suffix array sample
			offset[0] = suffixArraySamplePosition;
			count[0] = m_suffixArraySample.size();
			dims[0] = m_suffixArraySample.size();
			memspace = DataSpace(1, dims, NULL);
			dataspace = dst_suffixArraySample.getSpace();
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
			dst_suffixArraySample.write((void*)&*m_suffixArraySample.begin(), 
										m_fileDataTypes[DatasetNames.m_suffixArraySample], 
										memspace, dataspace);
			suffixArraySamplePosition += m_suffixArraySample.size();
			m_suffixArraySample.clear();
			// write tally
			const size_t tallyLength = (*m_tally.begin()).size();
			offset[0] = tallyPosition;
			count[0] = tallyLength;
			dims[0] = tallyLength;
			memspace = DataSpace(1, dims, NULL);
			dataspace = dst_tally.getSpace();
			dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
			std::vector<uint32_t> continiousBuffer(tallyLength * m_alphabet.size());
			for (uint32_t i = 0; i < m_alphabet.size(); i++)
			{
				auto bufIter = continiousBuffer.begin() + i;
				for (auto it = m_tally[i].begin(); it != m_tally[i].end(); ++it)
				{
					*bufIter = *it;
					bufIter += m_alphabet.size();
				}				
			}
			dst_tally.write((void*)&*continiousBuffer.begin(), 
							m_fileDataTypes[DatasetNames.m_tally], 
							memspace, dataspace);
			tallyPosition += tallyLength;
			for (auto it = m_tally.begin(); it != m_tally.end(); ++it)
				(*it).clear();
		}
	}
	// debug store complete suffixArray
	 //dims[0] = m_N;
	 //dataspace = DataSpace(1, dims);
	 //properties.setChunk(1, chunkDims);
	 //properties.setDeflate(3);
	 //DataSet dst_suffixArray = grpIndex.createDataSet(DatasetNames.m_suffixArray, 
		//										  m_fileDataTypes[DatasetNames.m_suffixArray], 
		//										  dataspace, properties);
	 //memspace = DataSpace(1, dims, NULL);
	 //dst_suffixArray.write((void*)&*m_suffixArray.begin(),
		//				    m_fileDataTypes[DatasetNames.m_suffixArray],
		//				    memspace, dataspace);
	// end of debug
	dst_bwtFirst.close();
	dst_bwtLast.close();
	dst_suffixArraySample.close();
	// dst_suffixArray.close();
	dst_tally.close();
	m_settings.logging().log(e_logInfo, "Finished writing index to disk.");
	// done, file is closed by destructor
}




// get position from suffix array sample
int64_t 
fmIndex::getPositionFromRow
(
	uint32_t row
)
{
	indexValuePair cmpVal;
	cmpVal.index = row;
	auto it = std::lower_bound(m_suffixArraySample.begin(),
		m_suffixArraySample.end(),
		cmpVal,
		[](indexValuePair lhs, indexValuePair rhs) -> bool{return lhs.index < rhs.index; });
	const uint32_t SaSampleStepSize = m_settings.m_saSampleStepSize;
	uint32_t steps = 0;
	while ((*it).index != cmpVal.index)
	{
		const char currentChar = m_bwtLast[cmpVal.index];
		uint32_t currentCount = getCount(currentChar, cmpVal.index);
		uint32_t currentRank = currentCount - 1;		// count will always be > 0
		cmpVal.index = getRowFromRank(currentChar, currentRank);
		it = std::lower_bound(m_suffixArraySample.begin(),
			m_suffixArraySample.end(),
			cmpVal,
			[](indexValuePair lhs, indexValuePair rhs) -> bool{return lhs.index < rhs.index; });
		if (it == m_suffixArraySample.end())
		{
			it--;
		}
		steps++;
		if (steps > SaSampleStepSize + 1)
		{
			// should not happen, ...
			m_settings.logging().log(e_logError, "Error while looking up suffix array sample. Please contact development.");
			return -1;
		}
	}
	return (*it).value + steps;
}




// process chunk of suffix array
void 
fmIndex::processChunk
(
	std::string const & s,
	std::vector<uint32_t> const & sfx, 
	const uint32_t offset
)
{
	// expand bwt
	auto bwtOffset = m_bwtLast.size();
	m_bwtLast.resize(m_bwtLast.size() + sfx.size());
	bwtFromSA(s, sfx, m_bwtLast, bwtOffset);

	// expand suffix array sample
	auto index = offset;
	auto SaSampleStepSize = m_settings.m_saSampleStepSize;
	for (auto it = sfx.begin(); it != sfx.end(); ++it)
	{
		if ((*it) % SaSampleStepSize == 0)
		{
			indexValuePair p;
			p.index = index;
			p.value = *it;
			m_suffixArraySample.push_back(p);
		}
		index++;
	}

	// init tally on first call
	static std::vector<uint32_t> tallyLine;
	if (offset == 0)
	{
		tallyLine = std::vector<uint32_t>(m_alphabet.size(), 0);
	}
		
	// expand tally
	index = offset;
	const uint32_t TallyStepSize = m_settings.m_tallyStepSize;
	for (auto it = m_bwtLast.begin() + bwtOffset; it != m_bwtLast.end(); ++it)
	{
		if ((*it) != '$')
			tallyLine[m_charIndex[*it]]++;
		if (index % TallyStepSize == 0)
		{
			for (uint32_t j = 0; j < m_alphabet.size(); j++)
				m_tally[j].push_back(tallyLine[j]);
		}
		index++;
	}
}




// init file datatypes
void 
fmIndex::initTallyDatatype
(
	 std::string alphabet
)
{
	if (m_fileDataTypes.find(DatasetNames.m_tally) != m_fileDataTypes.end())
		m_fileDataTypes.erase(DatasetNames.m_tally);
	// m_tally
	CompType memType(sizeof(uint32_t) * alphabet.size());
	for (size_t i = 0; i < alphabet.size(); i++)
		memType.insertMember(alphabet.substr(i, 1), i * sizeof(uint32_t), PredType::NATIVE_UINT32);
	m_fileDataTypes[DatasetNames.m_tally] = memType;
}




// clear all members
void 
fmIndex::clearDataStructures
(
	void
)
{
	this->m_charIndex = std::vector<uint8_t>(256, 255);
	this->m_alphabet.clear();
	this->m_bwtFirst.clear();
	this->m_bwtLast.clear();
	this->m_tally.clear();
	this->m_suffixArraySample.clear();
}




void
bwtFromSA
(
	const std::string& S,
	std::vector<uint32_t> const& sa,
	std::string& bwt,
	size_t bwtOffset
)
{
	for (size_t i = 0; i < sa.size(); i++)
	{
		if (sa[i] == 0)
			bwt[bwtOffset + i] = *(S.rbegin());
		else
			bwt[bwtOffset + i] = S[sa[i] - 1];
	}
}




herr_t 
getGroupDatasetNames
(
	hid_t loc_id, 
	const char* name, 
	const H5L_info_t *linfo, 
	void* opdata
)
{
	auto dsNames = reinterpret_cast<std::vector<std::string>*>(opdata);
	hid_t ds = H5Dopen2(loc_id, name, H5P_DEFAULT);
	if (ds > 0)
		dsNames->push_back(name);
	H5Dclose(ds);
	return 0;
}
