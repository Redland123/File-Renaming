#include "List.h"
#include <thread>
#include <string>
#include <chrono>
#include <iostream>

namespace fs = std::filesystem;
using namespace std;

class progressObject {
public:
	bool show = true;
	float numberOfFiles = 0;
	float* progress;

	progressObject(float x, float* newPtr) {
		this->progress = newPtr;
		this->numberOfFiles = x;
	};

	void display() {
		while (this->show == true) {
			cout << (((float)(*progress) / (float)(numberOfFiles) * (float)100.0) / (float)2.0) << "%" << "\r";
			this_thread::sleep_for(chrono::milliseconds(250));
			cout << "              " << "\r";
		}
		cout << "100.0" << "%";
	}
};

bool getWorkingDir(fs::path& workingDir) {
	cout << "Enter the directory of the files to be renamed: ";
	string userProvidedDir;
	getline(cin, userProvidedDir);

	//Checks if something has been entered into "userProvidedDir". 
	if (userProvidedDir.empty()) {
		cout << "Error: Path not entered. \nPress Enter to Exit.";
		cin.get();
		return false;
	}

	//Checks if the current directory is a valid folder within the system.
	if (!(fs::exists(userProvidedDir))) {
		cout << "Error: Path entered does not exist. \nPress Enter to Exist.";
		cin.get();
		return false;
	}

	try {
		workingDir = userProvidedDir;
	}
	catch (...) {
		return false;
	}
	return true;
}

string getPrefix() {
	cout << "Add a prefix for the names of these files: ";
	string prefix; cin >> prefix;
	cin.ignore(100, '\n');

	if (prefix.find_first_not_of("!@#$%^&()QWERTYUIOPASDFGHJKLZXCVBNM{}+=_-qwertyuiopasdfghjklzxcvbnm12345678910") != string::npos) {
		return "";
	}
	return prefix;
}

void renameFile(const LL* fileLL, const string prefix, const fs::path workingDir, const unsigned int startValue, float* progress) {
	//Checks if the array sent to the function was null
	if (fileLL == nullptr) return;
	node* current = fileLL->head;

	for (unsigned int i(0); i < fileLL->size; ++i) {
		try {
			rename(current->path, workingDir / (prefix + to_string((i + startValue)) + current->path.extension().string()));
		}
		catch (...) { cout << "\nError while renaming file."; }

		++(*progress);
		current = current->next;
	}
	return;
}

void printWarning() {
	cout << "------------------------------------WARNING-----------------------------------------\n"
		<< "This program attempts to rename ALL files contained within a provided directory.\nProviding a directory that contains"
		<< " files vital to the system, or programs on \nthe system, will result in the destruction of the system or program.\n\n"
		<< "Note(s): -The contents of folders inside the provided directory will NOT be renamed.\n"
		<< "\t -Files found in the provided directory whose names contain unsupported\n"
		<< "\t  characters will not be displayed. However, they will still be renamed.\n\n";
}

//Get confirmation from the user.
short userCheck() {
	char temp;
	cin >> temp;
	cin.ignore(100, '\n');
	if (temp == 'N' || temp == 'n') {
		return 0;
	}
	else if (temp == 'Y' || temp == 'y') {
		return 1;
	}
	else {
		cout << "\nInvalid option. The program will now exit.";
		cin.get();
		return -1;
	}
}

float displayFoundFiles(bool display, const fs::path& workingDir) {
	//String for displaying all folders in the current working directory.
	string foundFiles = "";
	float numberOfFilesFound = 0;

	//Adds the file names of the current directory to a the "foundFiles" string.
	for (const fs::path& currentFile : fs::directory_iterator(workingDir)) {
		//Tries to add the current file name to the string.
		try { foundFiles += currentFile.filename().string() + "\n"; }
		catch (...) { foundFiles += "File name contains unsupported characters.\n"; }

		++numberOfFilesFound;
	}

	if (display)
		cout << foundFiles;

	return numberOfFilesFound;
}

string stringToLower(string stringToConvert) {
	transform(stringToConvert.begin(), stringToConvert.end(), stringToConvert.begin(), [](unsigned char x) {return tolower(x); });
	return stringToConvert;
}

//Uses the fs::directory_iterator to traverse through the 'workingDir' placing file paths into 'entry'.
//This is done at the last possible second in an attempt to prevent data loss in the case of the program unexpectedly closing.
void populateFileArray(const fs::path& workingDir, LL* arrayOfLL[], string filePrefix, const unsigned short numberOfThreads, bool duplicateCheck) {
	unsigned short arrayIndex = 0;
	for (const fs::path& currentFile : fs::directory_iterator(workingDir)) {
		if (arrayIndex > (numberOfThreads - 1)) arrayIndex = 0;

		//Creates a new LL if the current pointer is empty. 
		if (arrayOfLL[arrayIndex] == nullptr) {
			arrayOfLL[arrayIndex] = new LL;
		}

		if (duplicateCheck == true) {
			if (filePrefix == "") {
				if (currentFile.filename().string().find_first_not_of("1234567890") != string::npos) {
					arrayOfLL[arrayIndex]->add(currentFile);
				}
			}
			else {
				//Converts strings to all lower case before compairing.
				string tempFileName(stringToLower(currentFile.filename().string()));
				string tempFilePrefix(stringToLower(filePrefix));

				if (tempFileName.rfind(tempFilePrefix, 0) != string::npos) {
					arrayOfLL[arrayIndex]->add(currentFile);
				}
			}
		}
		else {
			arrayOfLL[arrayIndex]->add(currentFile);
		}
		++arrayIndex;
	}
	return;
}

void populateThreadArray(const fs::path& workingDir, LL* arrayOfLL[], thread* threads[], const unsigned short numberOfThreads, string filePrefix, float* progress) {
	for (unsigned short i = 0; i < numberOfThreads; ++i) {
		int startValue = 0;
		for (unsigned short ii = 0; ii < i; ii++) {
			if (arrayOfLL[ii] != nullptr) {
				startValue += arrayOfLL[ii]->size;
			}
		}
		//Sends the renamingFile function to the available threads
		threads[i] = new thread(renameFile, ((arrayOfLL[i])), filePrefix, workingDir, startValue, progress);
	}
	return;
}

void joinThreads(thread* threads[], const unsigned short numberOfThreads) {
	for (unsigned short i = 0; i < numberOfThreads; ++i) {
		threads[i]->join();
	}
	return;
}

void cleanArrays(thread* threads[], LL* arrayOfLL[], const unsigned short numberOfThreads) {
	for (unsigned short i = 0; i < numberOfThreads; ++i) {
		delete threads[i];
		delete arrayOfLL[i];

		threads[i] = nullptr;
		arrayOfLL[i] = nullptr;
	}
	return;
}

int main() {
	//Creates a check point at every exit of the program inorder to find possible memory leaks.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	printWarning();

	//Gets working directory.
	fs::path workingDir;
	if (!getWorkingDir(workingDir)) return 0;

	//Gets the number of system threads.
	unsigned short numberOfThreads = thread::hardware_concurrency();
	numberOfThreads = numberOfThreads * 4;

	//Creates an array of linked List pointers.
	LL** arrayOfLL = new LL * [numberOfThreads];
	for (unsigned short i = 0; i < numberOfThreads; ++i) {
		arrayOfLL[i] = nullptr;
	}

	//Displays the files found and returns the amount.
	float numberOfFilesFound = displayFoundFiles(1, workingDir);

	cout << "\nThe above files were found in the entered directory.\n";
	cout << "Total number of files found: " << numberOfFilesFound << "\n";

	cout << "Are you sure you want to rename these files? (Y/N): ";
	short check = userCheck();
	if (check == 0 || check == -1) {
		delete[] arrayOfLL;
		return 0;
	};

	cout << "Would you like to enter a prefix the new file names? (Y/N): ";
	string filePrefix;
	check = userCheck();
	if (check == -1) {
		//Deletes the array of linkedlist
		for (unsigned short i = 0; i < numberOfThreads; ++i) {
			delete arrayOfLL[i];
		}
		delete[] arrayOfLL;
		return 0;
	}
	else if (check == 1) {
		filePrefix = getPrefix();
		if (filePrefix == "") {
			cout << "\nPrefix is not a valid file name. Press enter to exit.";
			cin.get();
			return 0;
		}
	}
	else {
		filePrefix = "";
	}

	cout << "\nWARNING: After the renaming process has started stopping it preemptively is not recommended.\n";
	cout << "Would you like to start? (Y/N):";
	check = userCheck();
	if (check == 0 || check == -1) {
		delete[] arrayOfLL;
		return 0;
	};

	cout << "\nProgress: " << "\n";

	auto start = chrono::high_resolution_clock::now();

	//Creates a dynamic array of thread pointers.
	thread** threads = new thread * [numberOfThreads];

	//Creates and starts the progress bar and thread.
	float* progress = new float(0);
	progressObject* progBar = new progressObject(numberOfFilesFound, progress);
	thread* progThread = new thread(&progressObject::display, progBar);

	populateFileArray(workingDir, arrayOfLL, filePrefix, numberOfThreads, true);
	populateThreadArray(workingDir, arrayOfLL, threads, numberOfThreads, "!@#$%^&", progress);

	joinThreads(threads, numberOfThreads);
	cleanArrays(threads, arrayOfLL, numberOfThreads);

	populateFileArray(workingDir, arrayOfLL, filePrefix, numberOfThreads, false);
	populateThreadArray(workingDir, arrayOfLL, threads, numberOfThreads, filePrefix, progress);

	joinThreads(threads, numberOfThreads);
	cleanArrays(threads, arrayOfLL, numberOfThreads);

	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

	//Stops and deletes progress bar and thread.
	progBar->show = false;
	progThread->join();
	delete progThread;
	delete progBar;

	cout << "\n\nCompleted. \n";
	cout << "This operation took:\n";
	cout << "Milliseconds: " << (duration.count()) << "\n";
	cout << "Seconds: " << ((duration.count() / 1000)) << "\n";
	cout << "Files lost: " << numberOfFilesFound - displayFoundFiles(0, workingDir);
	cout << "\nPress Enter to Exit.\n";

	cin.get();

	cleanArrays(threads, arrayOfLL, numberOfThreads);

	delete[] arrayOfLL;
	delete[] threads;

	return 1;
}
