// Copyright (c) 2025 Michal Wierzbinski
#include <iostream>
#include <vector>

#include "customFilesystem.hpp"
#include "utils.hpp"
#include "http_server.hpp"

#ifndef _DEBUG
	#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

namespace cfs = customFilesystem;

int main() {
	utils::LOG("getSpace backend started, initializing cfs");
	// init custom filesystem with all drives
	cfs::customFilesystem cfs;

	std::vector<char> drives = cfs.discoverDrives();
	for (auto& drive : drives) {
		cfs.Drives[drive] = cfs::Drive(drive);
	}

	utils::LOG("cfs initialized");
	http_server::http_server(cfs, 42690);
}

