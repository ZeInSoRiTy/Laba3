#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif



int main() {

#ifdef _WIN32
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
#else

	try {
		std::locale::global(std::locale("en_US.UTF-8"));
	}
	catch (const std::runtime_error&) {
		try {
			std::locale::global(std::locale(".UTF-8"));
		}
		catch (const std::runtime_error&) {
			std::wcerr << L"Система не поддерживает кодировку UTF-8" << std::endl;
		}
	}
	
	std::wcout.imbue(std::locale());
	std::wcin.imbue(std::locale());
#endif
	std::wstring fileN;

	std::wcout << L"Введите название файла: ";

	if (!(std::wcin >> fileN)) {
		return 1;
	}

	std::filesystem::path filePth(fileN);
	
	bool fileFound = false;

	if (std::filesystem::exists(filePth) && std::filesystem::is_regular_file(filePth)) {
		fileFound = true;
	}
	else {
		for (const auto& entry : std::filesystem::directory_iterator(L".")) {
			if (entry.is_regular_file()) {

				if (entry.path().stem() == fileN) {
					filePth = entry.path();
					fileFound = true;
					break;
				}
			}
		}
	}

	if (!fileFound) {
		std::wcout << L"Указанный файл не найден в текущей папке" << std::endl;
		return 1;
	}

	std::wifstream file(filePth);

	try {
		file.imbue(std::locale("en_US.UTF-8"));
	}
	catch (const std::runtime_error&) {
		file.imbue(std::locale(""));
	}

	if (!file.is_open()) {
		std::wcout << L"Не удалось открыть указанный файл" << std::endl;
		return 1;
	}

	std::wstring line;
	std::wcout << L"\n--------------------Перевернутый текст--------------------\n" << std::endl;

	while (std::getline(file, line)) {
		std::reverse(line.begin(), line.end());
		std::wcout << line << std::endl;
	}

	file.close();
	return 0;
}