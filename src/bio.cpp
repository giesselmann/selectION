// \MODULE\---------------------------------------------------------------
//
//  CONTENTS      : Biological helper functions
//
//  DESCRIPTION   :	none
//
//  RESTRICTIONS  : none
//
//  REQUIRES      : none
//
// -----------------------------------------------------------------------
// All rights reserved to Pay Gieﬂelmann, Germany
// -----------------------------------------------------------------------

//-- standard headers ----------------------------------------------------
#include <algorithm>
#include <stdexcept>

//-- private headers -----------------------------------------------------
#include "bio.h"

//-- source control system ID (if needed)---------------------------------

//-- exported global variables - definitions (should be empty) -----------

//-- private constants ---------------------------------------------------

//-- private types -------------------------------------------------------

//-- private functions --------- declarations ----------------------------

//-- private global variables -- definitions (should be empty) -----------

//-- exported functions -------- definitions -----------------------------
std::string 
reverseComplement
(
	std::string templateString
)
{
	std::string cmp(templateString);
	std::reverse(cmp.begin(), cmp.end());
	std::transform(cmp.cbegin(), cmp.cend(), cmp.begin(), 
		[](const char c) -> char
		{
			switch (c)
			{
			case 'A':
				return 'T';
			case 'C':
				return 'G';
			case 'G':
				return 'C';
			case 'T':
				return 'A';
			case 'N':
				return 'N';
			default:
				throw std::domain_error("Invalid nucleotide.");
			}
	});
	return cmp;
}




std::string
complement
(
	std::string templateString
)
{
	std::string cmp(templateString);
	std::transform(cmp.cbegin(), cmp.cend(), cmp.begin(), 
		[](const char c) -> char
		{
			switch (c)
			{
			case 'A':
				return 'T';
			case 'C':
				return 'G';
			case 'G':
				return 'C';
			case 'T':
				return 'A';
			case 'N':
				return 'N';
			default:
				throw std::domain_error("Invalid nucleotide.");
			}
	});
	return cmp;
}




//-- private functions --------- definitions -----------------------------
