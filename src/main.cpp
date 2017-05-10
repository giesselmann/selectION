// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Selection main function
//
//  DESCRIPTION   :	FM-Index based selection of a subset of reads
//					within specified region(s)
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
#include <string>
#include <cstdint>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

//-- private headers -----------------------------------------------------
#include "selection.h"
#include "fileFastx.h"

//-- private functions --------- declarations ----------------------------
void printUsage();

//-- exported functions -------- definitions -----------------------------
int main(int argc, char *argv[])
{
	namespace po = boost::program_options;
	try
	{
		po::options_description all("Command");
		all.add_options()
			("command", po::value<std::string>()->required(), "Command to execute")
			("options", po::value<std::vector<std::string>>(), "Options for command")
			;
		po::positional_options_description pos;
		pos.add("command", 1).
			add("options", -1);
		po::variables_map vm;	
		po::parsed_options parsed = po::command_line_parser(argc, argv).
			options(all).
			positional(pos).
			allow_unregistered().
			run();
		po::store(parsed, vm);
		// branch on selected command
		try
		{
			po::notify(vm);
			std::string command = vm["command"].as<std::string>();
			std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
			opts.erase(opts.begin());	// remove command option
			// build index for reference sequence
			if (command == "index")
			{
				selection_settings settings;
				po::options_description printOpt("Index building options");
				printOpt.add_options()
					("threads,t", po::value<uint32_t>()->default_value(settings.get_m_threads()), "Number of threads")
					("prefix,p", po::value<std::string>(), "Prefix of output database [Same as reference filename]")
					("tallyStep", po::value<uint32_t>()->default_value(settings.get_m_fmIndex_settings().get_m_tallyStepSize()), "Tally step size")
					("suffixSample", po::value<uint32_t>()->default_value(settings.get_m_fmIndex_settings().get_m_saSampleStepSize()), "Suffix-array sample step size")
					("sortMemory", po::value<uint32_t>()->default_value(settings.get_m_fmIndex_settings().get_m_MaxSuffixMemoryBlock()), "Memory for sorting suffix array")
					;
				po::options_description allOpt;
				allOpt.add(printOpt);
				allOpt.add_options()
					("reference,r", po::value<std::string>()->required(), "Input reference sequence")
					;
				po::positional_options_description pos;
				pos.add("reference", 1);
				try
				{
					po::store(po::command_line_parser(opts).
													  options(allOpt).
													  positional(pos).
													  run(), vm);
					po::notify(vm);
					std::string path2Reference = vm["reference"].as<std::string>();
					std::string dbPrefix = path2Reference;
					if (vm.count("prefix"))
						dbPrefix = vm["prefix"].as<std::string>();
					settings.set_m_threads(vm["threads"].as<uint32_t>());
					settings.get_m_fmIndex_settings().set_m_tallyStepSize(vm["tallyStep"].as<uint32_t>());
					settings.get_m_fmIndex_settings().set_m_saSampleStepSize(vm["suffixSample"].as<uint32_t>());
					settings.get_m_fmIndex_settings().set_m_MaxSuffixMemoryBlock(vm["sortMemory"].as<uint32_t>());
					selectION::buildFromFastx(settings, path2Reference, dbPrefix);
				}
				catch (po::error&)
				{
					std::cout << "Usage: selection index [options] <reference.fa>" << std::endl;
					std::cout << printOpt;
					return 0;
				}
			}
			// scan input files/directories for reads matching filter
			else if (command == "scan")
			{
				selection_settings settings;
				po::options_description printOpt("Read selection options");
				printOpt.add_options()			
					("filter,f", po::value<std::string>(), "Input selection filter")				
					("sam,s", po::value<std::string>()->default_value(settings.get_m_sam()), "Write pseudo alignment in sam format to file")
					("threads,t", po::value<uint32_t>()->default_value(settings.get_m_threads()), "Number of threads")
					("quality,q", po::value<uint32_t>()->default_value(settings.get_m_qualityThreshold()), "Quality threshold for filtered reads")
					("scanPrefix", po::value<uint32_t>()->default_value(settings.get_m_pseudoAligner_settings().get_m_scanPrefix()), "Prefix of read to use for alignment")
					;
				po::options_description allOpt;
				allOpt.add(printOpt);
				allOpt.add_options()
					("prefix,p", po::value<std::string>()->required(), "Prefix of database")
					("input,i", po::value<std::string>()->required(), "Input fastq filename")				
					("output,o", po::value<std::string>()->required(), "Output directory for selected reads")	
					;
				po::positional_options_description pos;
				pos.add("prefix", 1).add("input",1).add("output", 1);
				try
				{
					po::store(po::command_line_parser(opts).
													 options(allOpt).
													 positional(pos).
													 run(), vm);
					po::notify(vm);	
					std::string dbPrefix = vm["prefix"].as<std::string>();
					std::string path2Input = vm["input"].as<std::string>();					
					std::string outDir = vm["output"].as<std::string>();
					settings.set_m_sam(vm["sam"].as<std::string>());
					settings.set_m_threads(vm["threads"].as<uint32_t>());
					settings.set_m_qualityThreshold(vm["quality"].as<uint32_t>());
					settings.get_m_pseudoAligner_settings().set_m_scanPrefix(vm["scanPrefix"].as<uint32_t>());
					std::string cmd;
					for (int i = 0; i < argc; i++)
						cmd.append(std::string(argv[i]) + " ");
					settings.set_m_cmd(cmd);
					selectION sel(settings, dbPrefix);
					fileFastx seq(path2Input);
					positionFilter filter;
					if (vm.count("filter"))
					{
						std::string path2Filter = vm["filter"].as<std::string>();
						uint32_t n = filter.addSelectors(path2Filter);
						settings.logging().log(e_logInfo, "Successfully loaded " + std::to_string(n) + " selectors");
					}				
					sel.select(seq, filter, outDir);
				}
				catch (po::error&)
				{
					std::cout << "Usage: selection scan [options] <db.prefix> <input.fq> <outputDir>" << std::endl;
					std::cout << printOpt;
					return 0;
				}
			}
			else
			{
				printUsage();
				return 0;
			}
		}
		catch (po::error&)
		{
			// error specifying command
			printUsage();
			return 0;
		}
	}
	catch (std::exception& e)
	{
		std::cerr	<< "Exception in selectION: \n" 
					<< e.what() << "\n"
					<< "Application closed." << std::endl;
		return 1;
	}
	return 0;
}




//-- private functions --------- definitions -----------------------------
void 
printUsage()
{
	std::cout << "Program:\tSelectION" << std::endl
		<< "Usage:\t\tselection <command> [options]" << std::endl
		<< "Commands:\tindex : Build FM-Index for reference sequence" << std::endl
		<< "\t\tscan : Scan input for reads matching specified positions" << std::endl;
}