#pragma once
#include <filesystem>
#include <string>
#include <iostream>

#include "files.h"

enum class Command {
	STOP,
	LIST,
	CHDIR,
	MKDIR,
	HELP,
	ANALYZE,
	BOUNDS,
	SHOWBOUNDS,
	ERR
};

struct Query {
	Command command;
};

std::istream& operator >> (std::istream& is, Query& q); 

void ListDir();
void ChangeDir();
void MakeDir();
void ShowHelp();
void OpenSession(const double& lowerbnd, const double& upperbnd);
void SetBounds(double& lowbnd, double& upbnd);
void ShowBounds(const double& lowbnd, const double& upbnd);
bool FileNameMask(const std::filesystem::path& fpath);