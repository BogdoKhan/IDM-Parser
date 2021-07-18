#include "commands.h"
#include "files.h"

using namespace std;
namespace fs = std::filesystem;

istream& operator >> (istream& is, Query& q) {
	string s = "";
	cout << "Введите команду: " << endl;
	getline(cin, s);
	if (s == "stop" || s == "STOP" || s == ".q") {
		q.command = Command::STOP;
	}
	else if (s == "ls" || s == "list" || s == "LIST") {
		q.command = Command::LIST;
	}
	else if (s == "cd" || s == "CD" || s == "CHDIR") {
		q.command = Command::CHDIR;
	}
	else if (s == "mkdir" || s == "MKDIR") {
		q.command = Command::MKDIR;
	}
	else if (s == "help" || s == "HELP") {
		q.command = Command::HELP;
	}
	else if (s == "an" || s == "analyze" || s == "ANALYZE") {
		q.command = Command::ANALYZE;
	}
	else {
		q.command = Command::ERR;
	}
	return is;
}

void ListDir() {
	fs::path mypath = AskForPath();
	PrintFiles(mypath);
	cout << "\n";
}

void ChangeDir() {
	fs::path mypath = AskForPath();
	if (PathExists(mypath) && !mypath.has_extension()) {
		fs::current_path(mypath);
		cout << "Рабочая папка изменена\n";
	}
	cout << "---------------------------------------------------\n";
}

void MakeDir() {
	fs::path mypath = AskForPath();
	if (!PathExists(mypath)) {
		fs::create_directory(mypath);
		cout << "Создана новая папка в " << fs::absolute(mypath) << "\n";
	}
}

void ShowHelp() {
	cout << "Список команд:" << "\n";
	cout << "stop, STOP, .q - stops the execution of program\n";
	cout << "list, LIST - shows the list of files in specified directory\n";
	cout << "cd, CD, CHDIR - changes the working directory\n";
	cout << "mkdir, MKDIR - creates a new directory\n";
	cout << "help, HELP - shows the help\n";
	cout << "an, ANALYZE - analyze the file\n";
	cout << "---------------------------------------------------\n";
}

bool FileNameMask(const fs::path& fpath) {
	string s = fpath.stem().string();
	string mask = "DE";
	if (s.size() < 5) {
		return false;
	}
	else {
		return equal(s.end() - 2, s.end(), mask.begin());
	}
}

void OpenSession() {
	fs::path mypath = AskForPath();
	if (PathExists(mypath) && mypath.has_extension()) {
		if (mypath.extension() == ".txt" && FileNameMask(mypath)) {
			if (!fs::is_directory(mypath.stem())) {
				fs::create_directory(mypath.stem());
			}

			ofstream fileTotal(mypath.stem().string()+"\\total.txt", ios::trunc);
			ofstream hTimeN(mypath.stem().string() + "\\hTimeN.txt", ios::trunc);
			ofstream hTimeS(mypath.stem().string() + "\\hTimeS.txt", ios::trunc);
			ofstream fileMultN(mypath.stem().string() + "\\fileMultiNeutron.txt", ios::trunc);
			ofstream hSpectrN(mypath.stem().string() + "\\hSpectrN.txt", ios::trunc);

			cout << "---------------------------------------------------\n";
			cout << "Начат анализ файла.\n";

			map<size_t, vector<Event>> events = OpenFile(mypath);
			vector<double> TimesForNeutrons;
			vector<double> TimesForScint;
			vector<double> SpectrumNeutrons;

			map<size_t, size_t> Cluster;
			map<size_t, size_t> AC_Cluster; //anti-coincidence cluster
			map<size_t, size_t> clr_Cluster; //without anti-coincident

			size_t nNeutrons25 = 0;
			size_t nNeutrons = 0;
			size_t nScint = 0;
			size_t nAC = 0;
			//Fill collection of times for neutron and scint detectors, find their number
			for (const auto& event_ : events) {
				CollectionOfEvents col(event_);
				size_t NeutronsInEvent = col.Neutrons().at(Detector::NEUTRON1) + col.Neutrons().at(Detector::NEUTRON2);

				if (NeutronsInEvent > 0) {
					vector<double> col_nt = col.neutr_time();
					TimesForNeutrons.insert(TimesForNeutrons.end(), col_nt.begin(), col_nt.end());
					nNeutrons += NeutronsInEvent;
					if (col.Neutrons().at(Detector::NEUTRON1) > 0) {
						Cluster[col.Neutrons().at(Detector::NEUTRON1)]++;
					}
					if (col.Neutrons().at(Detector::NEUTRON2) > 0) {
						Cluster[col.Neutrons().at(Detector::NEUTRON2)]++;
					}

					//Fill table of registered anti-coincident neutron events
					//after application of anti-coincidence criteria (dt < 100 us)
					Detector det = Detector::UNUSED;
					if (col.Scint() > 0) {
						bool trig1 = false;
						bool trig2 = false;
						//Triggers, if the event has been already marked as anti-coincident

						for (const auto& vec : col.Scint_events()) {
							if (!trig2 && (vec.first == Detector::SCINT21 || vec.first == Detector::SCINT22) ) {
								det = Detector::NEUTRON2;
								for (const auto& item : vec.second) {
									if ((item.Time() - col.Trig().at(det) < 100.0) && (item.Time() - col.Trig().at(det) > 0)) {
										AC_Cluster[col.Neutrons().at(Detector::NEUTRON2)]++;
										nAC++;
										trig2 = true;
									}
								}
							}

							else if (!trig1 && (vec.first == Detector::SCINT11 || vec.first == Detector::SCINT12)) {
								det = Detector::NEUTRON1;
								for (const auto& item : vec.second) {
									if ((item.Time() - col.Trig().at(det) < 100.0) && (item.Time() - col.Trig().at(det) > 0)) {
										AC_Cluster[col.Neutrons().at(Detector::NEUTRON1)]++;
										nAC++;
										trig1 = true;
									}
								}
							}
						//------------------------------------------
						}
					}

					map<Detector, vector<Event>> col_neut = col.Neutr_events();
					for (const auto& vec : col_neut) {
						for (const auto& item : vec.second) {
							SpectrumNeutrons.push_back(item.Ampl());
							if (item.Ampl() > 2.0) {
								nNeutrons25++;
							}
						}
					}

					if (NeutronsInEvent > 1) {
						map<Detector, vector<Event>> colNeut = move(col_neut);
						map<Detector, vector<Event>> colScint = col.Scint_events();
						string Ndet = "";

						fileMultN << "Выстрел " << col.Shot() << " с " << NeutronsInEvent << " событиями с нейтронных "
							<< "и " << col.Scint() << " со сцинтилляционных детекторов\n";
						fileMultN << "---------------------------------------------------\nСобытия на нейтронных детекторах: \n";
						for (const auto& vec : colNeut) {
							if (vec.first == Detector::NEUTRON2) {
								Ndet = "ND2";
							}
							else if (vec.first == Detector::NEUTRON1) {
								Ndet = "ND1";
							}
							fileMultN << "Детектор нейтронов № " << Ndet << " (канал " << (*vec.second.begin()).Det() << ")"
								<< "с " << vec.second.size() << " событиями" << endl;
							fileMultN << setw(10) << "Ампл., V" << " " << setw(15) << "Время, мкс" << '\n';
							for (const auto& item : vec.second) {
								fileMultN << setw(10) << item.Ampl() << " " << setw(15) << (item.Time() - col.Trig().at(vec.first)) << '\n';
							}
						}
						fileMultN << "---------------------------------------------------\n";
						if (col.Scint() > 0) {
							fileMultN << "События на сцинтилляционных: \n";

							Detector trigDet = Detector::UNUSED;

							bool trig1 = false;
							bool trig2 = false;
							//Triggers, if the event has been already marked as anti-coincident

							for (const auto& vec : colScint) {
								double dt = 0;
								fileMultN << "Сцинтилляционные, канал " << (*vec.second.begin()).Det() << endl;
								fileMultN << setw(10) << "Ампл., V" << " " << setw(15) << "Время, мкс" << '\n';
								if (vec.first == Detector::SCINT21 || vec.first == Detector::SCINT22) {
									trigDet = Detector::NEUTRON2;
								}
								else if (vec.first == Detector::SCINT11 || vec.first == Detector::SCINT12) {
									trigDet = Detector::NEUTRON1;
								}
								for (const auto& item : vec.second) {
									dt = item.Time() - col.Trig().at(trigDet);
									fileMultN << setw(10) << item.Ampl() << " " << setw(15) << dt;
									if (dt < 100 && dt > 0) {
										if (vec.first == Detector::SCINT21 || vec.first == Detector::SCINT22) {
											fileMultN << " Событие запрещено";
											if (trig2) {
												fileMultN << " уже зарегистрированным событием";
											}
											fileMultN << "! Разница по времени " << dt << " мкс." << '\n';
											trig2 = true;
										}
										if (vec.first == Detector::SCINT11 || vec.first == Detector::SCINT12) {
											fileMultN << " Событие запрещено";
											if (trig1) {
												fileMultN << " уже зарегистрированным событием";
											}
											fileMultN << "! Разница по времени " << dt << " мкс." << '\n';
											trig1 = true;
										}
									}
									else { 
										fileMultN << endl;
									}
								}
								fileMultN << "---------------------------------------------------\n";
							}

						}
					}

					if (col.Scint() > 0) {
						vector<double> col_st = col.scint_time();
						TimesForScint.insert(TimesForScint.end(), col_st.begin(), col_st.end());
						nScint += col.Scint();
					}
				}
			}

			clr_Cluster = Cluster;
			for (auto& vec : clr_Cluster) {
				size_t key = vec.first;
				if (AC_Cluster.count(key) > 0) {
					vec.second -= AC_Cluster.at(key);
					if (key > 1) {
						clr_Cluster[key - 1] += AC_Cluster.at(key);
					}
				}
			}


			//Fill total information
			if (events.size() > 0) {
				fileTotal << "Запусков: " << events.rbegin()->first << "; ";
			}
			fileTotal << "Нейтронов: " << nNeutrons << ", "
				<< "с амплитудой выше 2 V: " << nNeutrons25 << '\n';
			if (Cluster.size() > 0) {
				fileTotal << "Событий по числу нейтронов (до запрета):\n";
				for (const auto& cl : Cluster) {
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : Cluster) {
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}

			if (clr_Cluster.size() > 0) {
				fileTotal << "Событий по числу нейтронов (после запрета):\n";
				for (const auto& cl : clr_Cluster) {
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : clr_Cluster) {
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}

			if (AC_Cluster.size() > 0) {
				fileTotal << "Запрещенных событий по числу нейтронов:\n";
				for (const auto& cl : AC_Cluster) {
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : AC_Cluster) {
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}
			fileTotal << "Событий на сцинтилляционных детекторах: " << nScint << '\n';
			fileTotal << "запрещено " << nAC << " событий" << '\n';
			fileTotal << "Всего событий на нейтронных после запрета: " << nNeutrons - nAC << '\n';

			//Fill histogram with times from neutron detector
			//fileTotal << "Non-zero times for neutron detector: " << '\n';

			Histo hTimesNeutron(TimesForNeutrons, -200.0, 0.1, 6000);
			vector<double> hXNeutron = hTimesNeutron.showX();
			vector<int> hYNeutron = hTimesNeutron.showY();
			for (size_t i = 0; i < hTimesNeutron.nbins(); i++) {
				hTimeN << setw(10) << hXNeutron.at(i) << " " << setw(10) << hYNeutron.at(i) << '\n';
			}
			//fileTotal << "\n---------------------------------------------------\n";

			//Fill histogram with times from scint detector
			//fileTotal << "Times for scint detector: " << '\n';
			Histo hTimesScint(TimesForScint, -200.0, 5, 120);
			vector<double> hXScint = hTimesScint.showX();
			vector<int> hYScint = hTimesScint.showY();
			for (size_t i = 0; i < hTimesScint.nbins(); i++) {
				hTimeS << setw(10) << hXScint.at(i) << " " << setw(10) << hYScint.at(i) << '\n';
			}
			//cout << "\n---------------------------------------------------\n";
			Histo hSpectrumN(SpectrumNeutrons, 0., 0.01, 500);
			vector<double> hXSN = hSpectrumN.showX();
			vector<int> hYSN = hSpectrumN.showY();
			for (size_t i = 0; i < hSpectrumN.nbins(); i++) {
				hSpectrN << setw(10) << hXSN.at(i) << " " << setw(10) << hYSN.at(i) << '\n';
			}

			fileTotal.close();
			hTimeN.close();
			hTimeS.close();
			fileMultN.close();
			hSpectrN.close();
			cout << "Анализ файла завершен!\n";
			cout << "---------------------------------------------------\n";
		}
	}
}