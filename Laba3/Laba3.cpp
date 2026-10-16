#include <iostream>
#include <fstream>
#include <string>
#include <locale>
#include <algorithm>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>
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

	std::filesystem::path filePth;
	std::filesystem::path outFilePth;

	bool fileFound = false;
	bool savePathFound = false;

#ifdef _WIN32

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {

		IFileOpenDialog* pFileOpen = nullptr;

		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr)) {

			COMDLG_FILTERSPEC fileTypes[] = {
				{ L"Текстовые файлы (*.txt)", L"*.txt" },
				{ L"Я динозавр (*.dyno)", L"*.dyno"}
			};
			pFileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);

			hr = pFileOpen->Show(NULL);

			if (SUCCEEDED(hr)) {
				IShellItem* pItem = nullptr;
				hr = pFileOpen->GetResult(&pItem);

				if (SUCCEEDED(hr)) {
					PWSTR pszFilePth = nullptr;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePth);

					if (SUCCEEDED(hr)) {

						filePth = pszFilePth;
						fileFound = true;
						CoTaskMemFree(pszFilePth);

					}

					pItem->Release();
				}
			}
			pFileOpen->Release();
		}

		if (!fileFound) {
			std::wcout << L"Выбор файла был отменен. Пожалуйста, выберите нужный файл для чтения данных." << std::endl;
			return 1;
		}

		IFileSaveDialog* pFileSave = nullptr;
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));
		if (SUCCEEDED(hr)) {
			COMDLG_FILTERSPEC fileTypes[] = {
					{ L"Текстовые файлы (*.txt)", L"*.txt" },
					{ L"Я динозавр (*.dyno)", L"*.dyno"}
			};
			pFileSave->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
			pFileSave->SetDefaultExtension(L"txt");

			std::wstring defName = filePth.stem().wstring() + L"_reversed";
			pFileSave->SetFileName(defName.c_str());

			hr = pFileSave->Show(NULL);
			if (SUCCEEDED(hr)) {
				IShellItem* pItem = nullptr;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszOutFilePth = nullptr;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszOutFilePth);
					if (SUCCEEDED(hr)) {
						outFilePth = pszOutFilePth;
						savePathFound = true;
						CoTaskMemFree(pszOutFilePth);
					}
					pItem->Release();
				}
			}
			pFileSave->Release();
		}
	CoUninitialize();
	}
	if (!savePathFound) {
		std::wcout << L"Выбор пути для сохранения был отменен." << std::endl;
		return 1;
	}

#else

	std::wstring fileN;
	std::wcout << L"Введите название файла: ";
	if (!(std::wcin >> fileN)) {
		return 1;
	}
	filePth = fileN;

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

#endif

	std::wifstream file(filePth);
	std::wofstream outFile(outFilePth);

	try {
		file.imbue(std::locale("en_US.UTF-8"));
		outFile.imbue(std::locale("en_US.UTF-8"));
	}
	catch (const std::runtime_error&) {
		file.imbue(std::locale(""));
		outFile.imbue(std::locale(""));
	}

	if (!file.is_open()) {
		std::wcout << L"Не удалось открыть указанный файл" << std::endl;
		return 1;
	}
	if (!outFile.is_open()) {
		std::wcout << L"Не удалось создать файл для записи" << std::endl;
		file.close();
		return 1;
	}
	

	std::wstring line;
	std::wcout << L"\n--------------------Перевернутый текст--------------------\n" << std::endl;

	int c = 0;



	while (std::getline(file, line)) {
		std::reverse(line.begin(), line.end());
		std::wcout << line << std::endl;
		outFile << line << std::endl;
		c += 1;
	}
	file.close();
	outFile.close();

	if (c == 0) {
		std::wcout << L"Исходный файл был пуст. Создан пустой файл." << std::endl;
	}
	else {
		std::wcout << L"Файл сохранен в: " << outFilePth.wstring() << std::endl;
	}
	return 0;
}