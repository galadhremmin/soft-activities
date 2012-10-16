#include "Utilities.h"
#include <sstream>
#include <algorithm>


template <class output_t, class input_t>
output_t CUtilities::cast(const input_t& input) {

}

float CUtilities::swedishNotation(const wstring input) {
	
}

template <class input_t>
const wstring CUtilities::toString(const input_t& t) {
	wistringstream ss;
	ss << t;

	return ss.str();
}