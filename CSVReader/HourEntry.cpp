#include "HourEntry.h"


CHourEntry::CHourEntry(const float hours, const wstring rateType, const float hourlyRate, const long date, const wstring responsible, const wstring comments) {
	_hours = hours;
	_hourlyRate = hourlyRate;
	_rateType = rateType;
	_date = date;
	_responsible = responsible;
	_comments = comments;
}

CHourEntry::~CHourEntry(void) {

}
const float CHourEntry::getHours(void) const {
	return _hours;
}

const float CHourEntry::getHourlyRate(void) const {
	return _hourlyRate;
}

const long CHourEntry::getDate(void) const {
	return _date;
}

const wstring CHourEntry::getRateType(void) const {
	return _rateType;
}

const wstring CHourEntry::getResponsible(void) const {
	return _responsible;
}

const wstring CHourEntry::getComments(void) const {
	return _comments;
}