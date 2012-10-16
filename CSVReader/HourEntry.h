#pragma once
#include <string>

using namespace std;

class CHourEntry {
public:
	CHourEntry(const float hours, const wstring rateType, const float hourlyRate, const long date, const wstring responsible, const wstring comments);
	~CHourEntry(void);

	const float getHours(void) const;
	const float getHourlyRate(void) const;
	const long getDate(void) const;
	const wstring getRateType(void) const;
	const wstring getResponsible(void) const;
	const wstring getComments(void) const;

private:
	float _hours, _hourlyRate;
	long _date;
	wstring _responsible, _comments, _rateType;
};

