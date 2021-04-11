#include <iostream>
#include <sstream>
#include <windows.h>
#include <locale>

#include "event.h"
#include "commands.h"

using namespace std;
namespace fs = std::filesystem;

int main() {
	//setlocale(LC_ALL, "");
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	cout << "IDM Parser v. 1.0.0\n";
	cout << "Введите HELP для вывода списка команд\n";
	Query q;
	cin >> q;
	cout << "---------------------------------------------------\n";
	while (q.command != Command::STOP) {
		switch (q.command) {
		case Command::LIST:
			ListDir();
			break;
		case Command::CHDIR:
			ChangeDir();
			break;
		case Command::MKDIR:
			MakeDir();
			break;
		case Command::HELP:
			ShowHelp();
			break;
		case Command::ANALYZE:
			OpenSession();
			break;
		default:
			cout << "Неверная команда! " << "\n";
			break;
		}
		cin >> q;
		cout << "---------------------------------------------------\n";
	}
	return 0;
}