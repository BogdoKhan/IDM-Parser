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
void OpenSession();
bool FileNameMask(const std::filesystem::path& fpath);