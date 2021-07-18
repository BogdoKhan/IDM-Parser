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

	cout << "IDM Parser v.1.0.2 Win10 Edition\n";
	cout << "ƒобавлен расчет числа запрещенных событий в пределах +100 мкс\n";
	cout << "≈сли на нейтронном детекторе зарегистрировано n событий, запрещаетс€ одно из них.\n\n";
	cout << "¬ведите HELP дл€ вывода списка команд или STOP дл€ выхода\n";
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
			cout << "Ќеверна€ команда! " << "\n";
			break;
		}
		cin >> q;
		cout << "---------------------------------------------------\n";
	}
	return 0;
}