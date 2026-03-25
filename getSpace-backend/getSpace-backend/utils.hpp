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

namespace utils {
	// helpers for helpers
	namespace detail{
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

	// Return current date and time as YYYY-MM-DD HH:MM:SS
	static std::string current_datetime() {
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm tm = detail::get_local_tm(now_time);

		char buf[20]; // 19 znaków na format + 1 na '\0'
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
		return std::string(buf);
	}
	}

	inline void LOG(std::string message) {
		std::string data = "[" + detail::current_datetime() + "] " + message + "\n";

		std::clog << data;
		std::ofstream log("data.log", std::ios_base::app);
		log << data;
	}

	// converts MB to GB with only 1 precision
	float MBtoGB(float MB) {
		return MB / 1024.0f;
	}

	// cuts float precision to 1 decimal place
	std::string cutPrecision(float size) {
		return std::format("{:.1f}", size);
	}

	// creates size label for file/folder returning string like "1.5 GB" or "500 MB"
	std::string createSizeLabel(float size) {
		if(size>=1024) {
			return cutPrecision(MBtoGB(size)) + " GB";
		}
		else { 
			return cutPrecision(size) + " MB";
		}
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

	void openInExplorer(const std::string& path) {
		std::wstring wpath = detail::stringToWString(path);
		ShellExecuteW(NULL, L"open", wpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

} // namespace utils