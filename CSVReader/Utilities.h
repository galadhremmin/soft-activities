#pragma once
#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

namespace Utilities
{
	static bool alphaFilter(wchar_t c) {
		return c != L'.' && c != L'-' && (c < L'0' || c > L'9');
	}

	template <class output_t, class input_t>
	inline output_t cast(const input_t& input) {
		wstringstream ss;
		output_t output;

		ss << input;
		ss >> output;

		return output;
	}

	template <class input_t>
	inline const wstring toString(const input_t& t) {
		wostringstream ss;
		ss << t;
		return ss.str();
	}

	inline float swedishNotation(const wstring input) {
		wstring tmp = input;

		replace(tmp.begin(), tmp.end(), L',', L'.'); // replace , with .
		remove_if(tmp.begin(), tmp.end(), &alphaFilter); // remove all non-numeric characters 

		return cast<float>(tmp);
	}
    
    // trim from start
    static inline wstring ltrim(wstring s) {
        s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
        return s;
    }
    
    // trim from end
    static inline wstring rtrim(wstring s) {
        s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
        return s;
    }
    
    // trim from both ends
    static inline wstring trim(const wstring s) {
        return ltrim(rtrim(s));
    }
}

