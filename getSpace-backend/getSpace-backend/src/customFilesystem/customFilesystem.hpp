// Copyright (c) 2025-2026 Michal Wierzbinski
#pragma once

#include <map>
#include <vector>

#include "src/customFilesystem/File.hpp"
#include "src/customFilesystem/Drive.hpp"
#include "src/customFilesystem/config.hpp"

namespace customFilesystem {
	class customFilesystem {
	public:
		std::map<char, Drive> Drives;
		std::vector<char> discoverDrives();
		void runDebugMode();
		void DEBUGwalkthrough(char drive);
	};
}  // namespace customFilesystem
