// Copyright (c) 2025-2026 Michal Wierzbinski
#pragma once

namespace customFilesystem {
    constexpr int8_t Tfile = 0;
    constexpr int8_t Tdir = 1;
    constexpr int8_t Troot = 2;

    inline bool skip_little_files = true;
    inline bool map_windows_folder = false;
	inline bool skip_cursed = true;
	/*
		example of cursed dir is Program Files\WindowsApps
		windows apps is rlly broken
		check size of Program Files\ its gonna tell u that its lets say 50gb
		check size of Program Files\WindowsApps its gonna tell u that its 180gb

		inside there is a lot of symlinks and junctions that point to other places on the disk that arent normal symlinks
		hellalot of them is detectable only by windows api calls
	*/
}  // namespace customFilesystem
