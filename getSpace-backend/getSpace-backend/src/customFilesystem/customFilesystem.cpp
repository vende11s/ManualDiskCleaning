// Copyright (c) 2025-2026 Michal Wierzbinski
#include "src/customFilesystem/customFilesystem.hpp"

#define WIN32_LEAN_AND_MEAN
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <Windows.h>

#include "src/customFilesystem/config.hpp"
#include "src/customFilesystem/File.hpp"
#include "src/customFilesystem/Drive.hpp"

// size is expressed in megabytes
namespace customFilesystem {
		// returns found drives letters
		std::vector<char> customFilesystem::discoverDrives() {
			std::vector<char> found_drives;
			DWORD dwDrives = GetLogicalDrives();
			for (char d = 'A'; d <= 'Z'; d++) {
				if (dwDrives & 1) {
					found_drives.push_back(d);
				}
				dwDrives = dwDrives >> 1;
			}
			return found_drives;
		}

		void customFilesystem::runDebugMode() {
			Drives['C'] = Drive('C');
			Drives['C'].mapDriveFast();
			DEBUGwalkthrough('C');
		}

		// need to map drive first, type folder name to enter it, type exit to exit, type .. to enter parent
		void customFilesystem::DEBUGwalkthrough(char drive) {
			auto dir = Drives[drive].getRoot();
			while (true) {
				std::cout << std::endl << std::endl << std::endl;
				std::cout << "Total mapped files size: " << dir->size << std::endl;
				std::cout << "Current dir: " << dir->getPath() << std::endl;
				dir->sortChilds();
				auto subdirs = dir->getChilds();
				for (auto& file : subdirs) {
					if (file->size < 10)
						continue;
					std::cout << std::left << std::setw(64) << file->filename + (file->isDir() ? "/" : "") 
					<< std::setw(15) << std::to_string((int)file->size)+" MB" << std::setw(15) << file->last_modified << "  "
					<< std::endl;
				}

				std::string s;
				std::getline(std::cin, s);
				if (s == "exit")
					return;

				if (s == "..") {
					dir = dir->getParent();
					continue;
				}

				if (s == "open") {
					system(std::string("start " + dir->getPath()).c_str());
					continue;
				}

				for (auto& file : subdirs) {
					if (file->filename == s) {
						dir = file;
						continue;
					}
				}

				std::cout << "dir not found" << std::endl;
			}
		}
}  // namespace customFilesystem
