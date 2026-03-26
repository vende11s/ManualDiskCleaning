// Copyright (c) 2025-2026 Michal Wierzbinski
#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>

#include "src/customFilesystem/File.hpp"

namespace customFilesystem {
	namespace fs = std::filesystem;
	class Drive {
		int TotalSize{ 0 };
		int UsedSize{ 0 };
		char letter;
		std::string name;
		std::shared_ptr<File> root = std::make_shared<File>();
		std::vector<std::vector<std::shared_ptr<File>>> files_container;  // trash container to store share pointers so they dont get killed by scope
		std::mutex m_new_file;

		float BytesToMB(int64_t Bytes);
		float getDriveSize();
		float getDriveUsedSize();
		std::string getDriveName();
		bool dirAccess(fs::path p);
		bool fileAccess(fs::path p);
		void _mapDriveFast(std::shared_ptr<File> start);
		bool isCursedSystemDir(const fs::path p);
		std::shared_ptr<File> mapFile(fs::directory_entry file_dir, std::shared_ptr<File> parent);

	public:
		std::atomic<uint64_t> mapped_bytes{ 0 };
		std::atomic<uint64_t> items_mapped{ 0 };

		explicit Drive(char letter);
		Drive();
		Drive& operator=(const Drive& d);

		int getTotalSize();
		int getUsedSize();
		char getLetter();
		std::string getName();
		std::shared_ptr<File> getRoot();

		void mapDriveFast();
		void mapDrive();
	};
}  // namespace customFilesystem
