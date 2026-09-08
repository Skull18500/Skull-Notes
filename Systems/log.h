#pragma once
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
inline std::mutex logMutex; // Protects the log file





//Made By : Microsoft Copilot(Remake Needed)
std::string getTime() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm buf{};
	localtime_s(&buf, &in_time_t);  // safer than localtime()

	std::stringstream ss;
	ss << std::put_time(&buf, "%H-%M-%S");
	return ss.str();
}


//Made By : Microsoft Copilot(Remake Needed)
std::string getDate() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm buf{};
	localtime_s(&buf, &in_time_t);  // safer than localtime()

	std::stringstream ss;
	ss << std::put_time(&buf, "%Y-%m-%d");
	return ss.str();
}

//log filename with time stamp
inline std::string logfilename = getTime() + "_engine.txt";

inline std::ofstream logFile;

//call at start of program
void openlog() {
	logFile.open(logfilename, std::ios::app);
}

//call at end of program
void closelog() {
	if (logFile.is_open()) {
		logFile.close();
	}
}

//add line to queue
void print(const std::string& message) {
	std::cout << message << "\n";
}


//use if you need a guarenteed print
void printclear(const std::string& message) {
	std::cout << message << std::endl;
}

//log to file without printing
void log(const std::string& message) {
	std::lock_guard<std::mutex> lock(logMutex); // Lock before writing
	if (logFile.is_open()) {
		logFile << message << std::endl;
	}
	else {
		openlog();
		if (logFile.is_open()) logFile << message << std::endl;
	}
}


//perfered for most cases
void printlog(const std::string& message) {
	print(message);
	log(message);
}


