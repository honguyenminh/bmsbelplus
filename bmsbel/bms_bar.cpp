#include "bmsbel\bms_bar.h"
#include "bmsbel\bms_bms.h"
#include "bmsbel\bms_util.h"

// -- BmsBarManager ------------------------------------------------------
BmsBarManager::BmsBarManager()
{
	barresolution_ = BmsConst::BAR_DEFAULT_RESOLUTION;
	Clear();
}

void BmsBarManager::Clear() {
	// fill all array with default bar count
	for (int i = 0; i < BmsConst::BAR_MAX_COUNT; i++)
		barcount_[i] = barresolution_;
	InvalidateCache();
}

unsigned int
BmsBarManager::operator [](unsigned int pos) const
{
	return barcount_[pos];
}

void
BmsBarManager::SetRatio(unsigned int pos, double l) {
	if (l < 0) return;
	barcount_[pos] *= l;
	InvalidateCache();
}

double 
BmsBarManager::GetRatio(unsigned int pos) const {
	return (double)barcount_[pos] / barresolution_;
}

barindex
BmsBarManager::GetBarNumberByMeasure(unsigned int pos) const
{
	barindex current = 0;
	for (barindex i = 0; i < pos; ++i) {
		current += barcount_[i];
	}
	return current;
}

unsigned int
BmsBarManager::GetMeasureByBarNumber(barindex pos) const
{
	// find fast using cached data
	return barcache_.equal_range(pos).first->second;
}

double
BmsBarManager::GetMeasureByBar(double pos) const
{
	// find fast using cached data
	auto iter = barcache_.equal_range(pos).first;
	unsigned int bar = iter->second;
	unsigned int measure = iter->second;
	if (++iter == barcache_.end()) return bar;
	else {
		return bar + barcount_[measure] * pos;
	}
}

double
BmsBarManager::GetBarByMeasure(double pos) const
{
	if (pos < 0) return 0;
	double current = 0;
	unsigned int m = 0;
	for (; m <= pos; ++m) {
		current += barcount_[m];
	}
	return current - barcount_[m - 1] + barcount_[m - 1] * (pos - (int)pos);
}

double
BmsBarManager::GetPosByBar(double bar) const {
	return bar / barresolution_;
}

int
BmsBarManager::GetBarByPos(double pos, int step) const {
	// 0.5: an easy round function
	return (int)(pos * step + 0.5) * barresolution_ / step;
}

unsigned int
BmsBarManager::GetDivision(const BmsBuffer& channelbuf, unsigned int measure) const
{
	barindex bar = GetBarNumberByMeasure(measure);
	barindex bar_end = GetBarNumberByMeasure(measure + 1);
	int barsize = barcount_[measure];
	unsigned int step = barsize;
	for (auto it = channelbuf.Begin(); it != channelbuf.End(); ++it) {
		if (it->first >= bar_end) break;
		if (it->first >= bar) {
			barindex relativepos = it->first - bar;
			step = BmsUtil::GCD(step, relativepos);
		}
	}
	return barsize / step;
}

void
BmsBarManager::SetResolution(double d)
{
	barresolution_ *= d;

	// make every cache size reseted and InvalidateCache
	for (int i = 0; i < BmsConst::BAR_MAX_COUNT; i++)
		barcount_[i] *= d;

	// invalidate cache
	InvalidateCache();
}

//
void BmsBarManager::InvalidateCache() {
	barcache_.clear();
	barindex barcount = 0;
	for (int i = 0; i < BmsConst::BAR_MAX_COUNT; i++) {
		barcache_[barcount] = i;
		barcount += barcount_[i];
	}
	barcache_[barcount] = BmsConst::BAR_MAX_COUNT;
}