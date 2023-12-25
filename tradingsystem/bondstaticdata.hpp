/**
 * bondstaticdata.hpp
 * Contains static data for the 7 current on-the-run US treasuries.
 *
 * @author Krystal Lin
 */

#ifndef BONDSTATICDATA_HPP
#define BONDSTATICDATA_HPP

#include <string>
#include <map>
#include "products.hpp"
#include <type_traits>

static std::map<std::string, std::string> CUSIP_MAPPING = { {"2Y","91282CJL6"},
														   {"3Y","91282CJP7"},
														   {"5Y","91282CJN2"},
														   {"7Y","91282CJM4"},
														   {"10Y","91282CJJ1"},
														   {"20Y","912810TW8"},
														   {"30Y","912810TV0"} };


template <typename T>
T get_product(string ticker);  // Declaration only

// Get Bond object for US Treasury
template <>
Bond get_product(string ticker)
{
	if (ticker == "2Y")  return Bond("91282CJL6", CUSIP, "2Y", 4.875, from_string("2025/11/30"));
	else if (ticker == "3Y") return Bond("91282CJP7", CUSIP, "3Y", 4.375, from_string("2026/12/15"));
	else if (ticker == "5Y") return Bond("91282CJN2", CUSIP, "5Y", 4.375, from_string("2028/11/30"));
	else if (ticker == "7Y") return Bond("91282CJM4", CUSIP, "7Y", 4.375, from_string("2030/11/30"));
	else if (ticker == "10Y") return Bond("91282CJJ1", CUSIP, "10Y", 4.5, from_string("2033/11/15"));
	else if (ticker == "20Y") return Bond("912810TW8", CUSIP, "20Y", 4.75, from_string("2043/11/15"));
	else if (ticker == "30Y") return Bond("912810TV0", CUSIP, "30Y", 4.75, from_string("2053/11/15"));
}


// Get PV01 value for US Treasury. From bbg ASOF 12/22/2023
double get_pv01(string _cusip)
{
	double _pv01 = 0;
	if (_cusip == "91282CJL6") _pv01 = 0.0184433;
	else if (_cusip == "91282CJP7") _pv01 = 0.027892;
	else if (_cusip == "91282CJN2") _pv01 = 0.0451297;
	else if (_cusip == "91282CJM4") _pv01 = 0.0613336;
	else if (_cusip == "91282CJJ1") _pv01 = 0.0840999;
	else if (_cusip == "912810TW8") _pv01 = 0.1410550;
	else if (_cusip == "912810TV0") _pv01 = 0.1890362;
	return _pv01;
}



#endif
