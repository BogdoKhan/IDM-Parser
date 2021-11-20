#pragma once
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "event.h"

std::filesystem::path AskForPath();
bool PathExists(const std::filesystem::path& fpath);
void PrintFiles(const std::filesystem::path& fpath);

std::map<size_t, std::vector<Event>> OpenFile(const std::filesystem::path& fpath);

class Histo {
public:
	Histo();
	Histo(const std::vector<double>& data, const double& start, const double& step, const size_t& nbins);
	const std::vector<double> showX() const;
	const std::vector<int> showY() const;
	const size_t nbins() const;
private:
	size_t NBINS = 120;
	double _start;
	double _step;
	std::vector<double> x;
	std::vector<int> y;
};