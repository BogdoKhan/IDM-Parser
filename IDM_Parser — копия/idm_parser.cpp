#include <iostream>
#include <sstream>
#include <windows.h>
#include <locale>
#include <limits>

#include "event.h"
#include "commands.h"

using namespace std;
namespace fs = std::filesystem;

int main() {
	//setlocale(LC_ALL, "");
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);

	double AnLowerBound = 0;
	double AnUpperBound = 10.0;

	cout << "IDM Parser v.1.0.2 Win10 Edition\n";
	cout << "�������� ������ ����� ����������� ������� � �������� +100 ���\n";
	cout << "���� �� ���������� ��������� ���������������� n �������, ����������� ���� �� ���.\n\n";
	cout << "������� HELP ��� ������ ������ ������ ��� STOP ��� ������\n";
	SetBounds(AnLowerBound, AnUpperBound);

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
			OpenSession(AnLowerBound, AnUpperBound);
			break;
		case Command::BOUNDS:
			SetBounds(AnLowerBound, AnUpperBound);
			break;
		case Command::SHOWBOUNDS:
			ShowBounds(AnLowerBound, AnUpperBound);
			break;
		default:
			cout << "�������� �������!\n";
			break;
		}
		cin >> q;
		cout << "---------------------------------------------------\n";
	}
	return 0;
}