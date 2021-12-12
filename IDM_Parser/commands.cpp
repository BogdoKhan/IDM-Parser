#include "commands.h"
#include "files.h"

#include <iostream>
#include <iomanip>

using namespace std;
namespace fs = std::filesystem;

istream& operator >> (istream& is, Query& q) {
	string s = "";
	cout << "---------------------------------------------------\n";
	cout << "\nВведите команду: " << endl;
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
	else if (s == "SETB" || s == "sb") {
		q.command = Command::BOUNDS;
	}
	else if (s == "SHOWB" || s == "shb") {
		q.command = Command::SHOWBOUNDS;
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
	cout << "sb, SETB - set boundary values for data processing";
	cout << "shb, SHOWB - show boundary values for data processing";
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

void SetBounds(double& lowbnd, double& upbnd) {
	string s1, s2;
	double value1 = 0;
	double value2 = 10.0;
	try {
		cout << "---------------------------------------------------\n\n";
		cout << "Введите нижнюю границу обработки по амплитуде (В)\n";
		int c = cin.peek();
		if (!isdigit(c) && c != '.') {
			throw "Wrong";
		}
		cin >> s1;
		value1 = stod(s1);
		cout << "Введите верхнюю границу обработки по амплитуде (В)\n";
		if (!isdigit(c) && c != '.') {
			throw "Wrong";
		}
		cin >> s2;
		value2 = stod(s2);
		cout << "---------------------------------------------------\n";
		if (value1 >= value2) {
			throw "Верхняя граница меньше нижней!";
		}
		if (value1 < 0 || value2 < 0) {
			throw "Задана отрицательная граница по амплитуде!";
		}
		lowbnd = value1;
		upbnd = value2;
		cout << "Границы установлены\n";
	}
	catch (const exception& e) {
		cout << "Ошибка: неверный ввод. Описание: " << e.what() << endl;
	}
	catch (const char* e) {
		cout << "Ошибка: неверный ввод. Описание: " << e << endl;
	}
	cin.clear();
	cin.ignore(10, '\n');
}

void ShowBounds(const double& lowbnd, const double& upbnd) {
	cout << "Текущие границы обработки по амплитуде (В):\n";
	cout << "Нижняя: " << lowbnd << ", В; верхняя: " << upbnd << ", В\n";
}

void OpenSession(const double& lowerbnd, const double& upperbnd) {
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
			ofstream hSpectrNR(mypath.stem().string() + "\\hSpectrNRange.txt", ios::trunc);

			double SessionTime = 1;

			cout << "---------------------------------------------------\n";
			cout << "Введите длительность сессии в секундах (для пропуска введите 1):\n";
			cin >> SessionTime;
			cin.clear();
			cin.ignore(10, '\n');
			cout << "Длительность сессии: " << SessionTime << " секунд\n";
			cout << "---------------------------------------------------\n";
			cout << "Начат анализ файла.\n";

			map<size_t, vector<Event>> events = OpenFile(mypath);
			vector<double> TimesForNeutrons;
			vector<double> TimesForScint;
			vector<double> SpectrumNeutrons;
			vector<double> SpectrumNeutronsRange;

			map<size_t, size_t> Cluster;
			map<size_t, size_t> AC_Cluster; //anti-coincidence cluster
			map<size_t, size_t> clr_Cluster; //without anti-coincident

			map<string, size_t> map_neutrons;
			map<string, size_t> nNeutrons = {
					{"TOTAL", 0}, {"RANGE", 0}, {"OOR", 0}, {"CRPT", 0}
			};
			size_t nScint = 0;
			size_t nAC = 0;
			size_t nAC_n = 0;
			size_t nAC_range = 0;
			//Fill collection of times for neutron and scint detectors, find their number
			for (const auto& event_ : events) {
				//take Nth key (# of shot) with vector of events in Nth shot
				CollectionOfEvents col(event_, lowerbnd, upperbnd);
				//number of neutrons in event, neutrons in range, out of range & from corrupted events
				map<string, size_t> NeutronsInEvent = {
					{"TOTAL", 0}, {"RANGE", 0}, {"OOR", 0}, {"CRPT", 0}
				};
				for (auto& item : NeutronsInEvent) {
					NeutronsInEvent.at(item.first) = col.Neutrons().at(item.first).at(Detector::NEUTRON1) 
						+ col.Neutrons().at(item.first).at(Detector::NEUTRON2);
				}


				if (NeutronsInEvent.at("TOTAL") > 0) {
					//put times into vector for neutron times histogram
					vector<double> col_nt = col.neutr_time().at("RANGE");
					TimesForNeutrons.insert(TimesForNeutrons.end(), col_nt.begin(), col_nt.end());
					//add number of neutrons for every case
					bool is_fault = 0;
					int is_ok_n = 0;
					int nbad = 0;
					for (const auto& item : NeutronsInEvent) {
						if (item.first == "RANGE") {
							vector<Event> ncd;
							map <Detector, vector<Event>> e1 = col.Neutr_events().at("RANGE");
							bool is_nok = false;

							if (e1.find(Detector::NEUTRON1) != e1.end()) {
								for (const auto& ev : e1.at(Detector::NEUTRON1)) {
									ncd.push_back(ev);
								}
							}
							if (e1.find(Detector::NEUTRON2) != e1.end()) {
								for (auto& ev : e1.at(Detector::NEUTRON2)) {
									ncd.push_back(ev);
								}
							}
							if (ncd.size() > 0) {
								for (const auto& item : ncd) {
									if ((item.Time() - col.Trig()) > 0.0001 && (item.Time() - col.Trig()) < 2.) {
										is_fault = true;
										is_nok = true;
										nbad++;
									}
								}
							}
							if (item.first == "RANGE" &&
								col.Neutrons().at("CRPT").at(Detector::NEUTRON1) == 0 &&
								col.Neutrons().at("CRPT").at(Detector::NEUTRON2) == 0 && col.Trig() > 0
								) {
								nNeutrons.at(item.first) += (item.second - nbad);
								nNeutrons.at("CRPT") += nbad;
							}
							else if (item.first == "RANGE" || is_fault) {
								nNeutrons.at("CRPT") += nbad;
							}

							is_ok_n = NeutronsInEvent.at("TOTAL") - nbad;
						}
						
						else if (item.first == "TOTAL") {
							nNeutrons.at(item.first) += item.second;
						}
						else {
							nNeutrons.at(item.first) += item.second;
						}
					}
					if ((col.Neutrons().at("RANGE").at(Detector::NEUTRON1) > 0 ||
						col.Neutrons().at("RANGE").at(Detector::NEUTRON2) > 0) &&
						col.Neutrons().at("CRPT").at(Detector::NEUTRON1) == 0 &&
						col.Neutrons().at("CRPT").at(Detector::NEUTRON2) == 0 && col.Trig() > 0) {
						if (is_ok_n > 0) {
							Cluster[is_ok_n]++;
						}
						

					}

					//Fill table of registered anti-coincident neutron events
					//after application of anti-coincidence criteria (dt < 100 us)
					Detector det = Detector::UNUSED;
					if (col.Scint() > 0) {
						bool trig1 = false;
						bool trig2 = false;
						//Triggers, if the event has been already marked as anti-coincident

						for (const auto& vec : col.Scint_events()) {
							if (!trig1 && !trig2 && vec.first == Detector::SCINT11) {
								for (const auto& item : vec.second) {
									if ((item.Time() - col.Trig() < 200.0) && (item.Time() - col.Trig() > 0)) {

										if (col.Scint() >= 1
											&& col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1
											&& col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1) {

											AC_Cluster[
												(col.Neutrons().at("RANGE").at(Detector::NEUTRON1) +
													col.Neutrons().at("RANGE").at(Detector::NEUTRON2))]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON1);
											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON2);
											nAC_range++;
										}

										else if (col.Scint() >= 1 && col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1) {

											AC_Cluster[col.Neutrons().at("RANGE").at(Detector::NEUTRON2)]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON2);
											nAC_range++;
										}
										else if (col.Scint() >= 1 && col.Neutrons().at("RANGE").at(Detector::NEUTRON1) >= 1) {

											AC_Cluster[col.Neutrons().at("RANGE").at(Detector::NEUTRON1)]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON1);
											nAC_range++;
										}
										nAC++;
										trig1 = true;
									}
								}
							}

							else if (!trig1 && !trig2 && vec.first == Detector::SCINT12) {
								for (const auto& item : vec.second) {
									if ((item.Time() - col.Trig() < 200.0) && (item.Time() - col.Trig() > 0)) {
										if (col.Scint() >= 1
											&& col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1
											&& col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1) {
											AC_Cluster[
												(col.Neutrons().at("RANGE").at(Detector::NEUTRON1) +
													col.Neutrons().at("RANGE").at(Detector::NEUTRON2))]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON1);
											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON2);
											nAC_range++;
										}
										else if (col.Scint() >= 1 && col.Neutrons().at("RANGE").at(Detector::NEUTRON2) >= 1) {
											AC_Cluster[col.Neutrons().at("RANGE").at(Detector::NEUTRON2)]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON2);
											nAC_range++;
										}
										else if (col.Scint() >= 1 && col.Neutrons().at("RANGE").at(Detector::NEUTRON1) >= 1) {
											AC_Cluster[col.Neutrons().at("RANGE").at(Detector::NEUTRON1)]++;

											nAC_n += col.Neutrons().at("RANGE").at(Detector::NEUTRON1);
											nAC_range++;
										}
										nAC++;
										trig2 = true;
									} //time comparison
								} //loop
							} //for scint12
						//------------------------------------------
						}
					}

					map<Detector, vector<Event>> col_nRange = col.Neutr_events().at("RANGE");
					for (const auto& vec : col_nRange) {
						for (const auto& item : vec.second) {
							if (!(item.Time() - col.Trig()) > 0.000001 && (item.Time() - col.Trig()) < 2.){
								SpectrumNeutronsRange.push_back(item.Ampl());
							}
						}
						
					}
					//nNeutrons.at("RANGE") = col_nRange.at(Detector::NEUTRON1).size() + col_nRange.at(Detector::NEUTRON2).size();

					map<Detector, vector<Event>> col_neut = col.Neutr_events().at("TOTAL");
					for (const auto& vec : col_neut) {
						for (const auto& item : vec.second) {
							SpectrumNeutrons.push_back(item.Ampl());
						}
					}
					//nNeutrons.at("TOTAL") = col_neut.at(Detector::NEUTRON1).size() + col_neut.at(Detector::NEUTRON2).size();


					if (NeutronsInEvent.at("RANGE") > 1) {
						map<Detector, vector<Event>> colNeut = move(col_neut);
						map<Detector, vector<Event>> colScint = col.Scint_events();
						string Ndet = "";

						fileMultN << "Выстрел " << col.Shot() << " с " << NeutronsInEvent.at("TOTAL")
							<< " событиями с нейтронных "
							<< "и " << col.Scint() << " со сцинтилляционных детекторов\n";
						fileMultN << "Из " << NeutronsInEvent.at("TOTAL") << " событий с нейтронных детекторов:\n"
							<< "в пределах области обработки " << lowerbnd << " - " << upperbnd << " B: "
							<< NeutronsInEvent.at("RANGE") << "\n" << "за пределами области обработки: "
							<< NeutronsInEvent.at("OOR") << "\n" << "ниже порога (ошибка определения пиков): "
							<< NeutronsInEvent.at("CRPT") << "\n";

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
								fileMultN << setw(10) << item.Ampl() << " " << setw(15) << (item.Time() - col.Trig()) << '\n';
							}
						}
						fileMultN << "---------------------------------------------------\n";
						fileMultN << "fault " << is_fault << endl;
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
									dt = item.Time() - col.Trig();
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
										fileMultN << " Нет запрета, не входит в +100 us!" << endl;
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
				}
			}


			//Fill total information
			if (events.size() > 0) {
				double TrigFreq = (events.rbegin()->first) / SessionTime;
				fileTotal.precision(3);
				fileTotal << "Запусков: " << events.rbegin()->first << "; частота запусков: " 
					<< TrigFreq << " Гц.\n";
				//fileTotal.precision(ios::floatfield);
			}
			fileTotal << "Нейтронов в диапазоне амплитуд " << lowerbnd << " - " << upperbnd << " В : " <<
				nNeutrons.at("RANGE") << ",\n"
				<< "с амплитудой выше " << upperbnd << " V: " << nNeutrons.at("OOR") << '\n';
			fileTotal << "Из " << nNeutrons.at("TOTAL") << " событий с нейтронных детекторов:\n"
				<< "в пределах области обработки " << lowerbnd << " - " << upperbnd << " B: "
				<< nNeutrons.at("RANGE") << "\n" << "за пределами области обработки: "
				<< nNeutrons.at("OOR") << "\n" << "ниже порога (ошибка определения пиков): "
				<< nNeutrons.at("CRPT") << "\n";
			if (Cluster.size() > 0) {
				fileTotal << "Событий по числу нейтронов в пределах обработки (до запрета):\n";
				for (const auto& cl : Cluster) {
					fileTotal<< setw(8);
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : Cluster) {
					fileTotal << setw(8);
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}

			if (clr_Cluster.size() > 0) {
				fileTotal << "Событий по числу нейтронов (после запрета):\n";
				for (const auto& cl : clr_Cluster) {
					fileTotal << setw(8);
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : clr_Cluster) {
					fileTotal << setw(8);
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}

			if (AC_Cluster.size() > 0) {
				fileTotal << "Запрещенных событий по числу нейтронов:\n";
				for (const auto& cl : AC_Cluster) {
					fileTotal << setw(8);
					fileTotal << cl.first << " ";
				}
				fileTotal << endl;
				for (const auto& cl : AC_Cluster) {
					fileTotal << setw(8);
					fileTotal << cl.second << " ";
				}
				fileTotal << endl;
			}



			fileTotal << "Событий на сцинтилляционных детекторах: " << nScint << '\n';
			fileTotal << "запрещено всего " << nAC << " запусков, из них в пределах обработки "
				<< nAC_range << " с " << nAC_n << " нейтронами" << '\n';
			fileTotal << "Всего событий на нейтронных после запрета: " << nNeutrons.at("RANGE") - nAC_n << '\n';

			//Fill histogram with times from neutron detector
			//fileTotal << "Non-zero times for neutron detector: " << '\n';

			Histo hTimesNeutron(TimesForNeutrons, -200.0, 1.2, 500);
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
			int NScintOf100 = 0;
			for (size_t i = 0; i < hTimesScint.nbins(); i++) {
				hTimeS << setw(10) << hXScint.at(i) << " " << setw(10) << hYScint.at(i) << '\n';
				//100-us scint events
				if (hXScint.at(i) == 95) {
					NScintOf100 = hYScint.at(i);
				}
			}
			double Scint100Freq = ((NScintOf100 / SessionTime) * 1000);
			//fileTotal.precision(3);
			fileTotal << "В пике +100 мкс для сцинтилляционного детектора: " << NScintOf100 << " событий\n"
				<< "Частота таких событий: " << Scint100Freq << " мГц\n";
			//fileTotal.precision(ios::floatfield);
			//cout << "\n---------------------------------------------------\n";
			Histo hSpectrumN(SpectrumNeutrons, 0., 0.012, 500);
			vector<double> hXSN = hSpectrumN.showX();
			vector<int> hYSN = hSpectrumN.showY();
			for (size_t i = 0; i < hSpectrumN.nbins(); i++) {
				hSpectrN << setw(10) << hXSN.at(i) << " " << setw(10) << hYSN.at(i) << '\n';
			}
			//cout << "\n---------------------------------------------------\n";
			Histo hSpectrumNR(SpectrumNeutronsRange, 0., 0.012, 500);
			vector<double> hXSNR = hSpectrumNR.showX();
			vector<int> hYSNR = hSpectrumNR.showY();
			for (size_t i = 0; i < hSpectrumNR.nbins(); i++) {
				hSpectrNR << setw(10) << hXSNR.at(i) << " " << setw(10) << hYSNR.at(i) << '\n';
			}

			fileTotal.close();
			hTimeN.close();
			hTimeS.close();
			fileMultN.close();
			hSpectrN.close();
			hSpectrNR.close();
			cout << "Анализ файла завершен!\n";
			cout << "---------------------------------------------------\n";
		}
	}
}