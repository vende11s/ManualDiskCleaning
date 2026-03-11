// Copyright (c) 2025 Michal Wierzbinski
#pragma once
#include <string>
#include <ctime>
#include <iostream>
#include <chrono>
#include <fstream>
#include <format>

#include <Windows.h>
#include <shellapi.h>

// Cross-platform thread-safe localtime
static std::tm get_local_tm(std::time_t t) {
	std::tm tm{};
#ifdef _WIN32
	localtime_s(&tm, &t);  // Windows
#else
	localtime_r(&t, &tm);  // Linux/macOS
#endif
	return tm;
}

std::wstring stringToWString(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
	return wstr;
}

// Return current time as HH:MM:SS
static std::string current_time_hms() {
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm tm = get_local_tm(now_time);

	char buf[9]; // HH:MM:SS
	std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
	return std::string(buf);
}

namespace utils {
	inline void LOG(std::string message) {
		std::string data = "[" + current_time_hms() + "] " + message + "\n";

		std::clog << data;
		std::ofstream log("data.log", std::ios_base::app);
		log << data;
	}

	// converts MB to GB with only 1 precision
	float MBtoGB(float MB) {
		return MB / 1024.0f;
	}

	// cuts float precision to 1 decimal place
	std::string cutPrecision(float GB) {
		return std::format("{:.1f}", GB);
	}

	bool moveToRecycleBin(const std::string& path)
	{
		std::string doubleNullPath = path + '\0';

		SHFILEOPSTRUCTA fileOp = { 0 };
		fileOp.hwnd = GetConsoleWindow(); // attach dialog to console window
		fileOp.wFunc = FO_DELETE;
		fileOp.pFrom = doubleNullPath.c_str();
		fileOp.fFlags = FOF_ALLOWUNDO; // move to Recycle Bin, show confirmation
		return SHFileOperationA(&fileOp) == 0;
	}

} // namespace utils