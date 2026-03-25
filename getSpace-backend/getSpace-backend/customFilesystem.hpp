// Copyright 2025 Michal Wierzbinski
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <Windows.h>

// size is expressed in megabytes
namespace customFilesystem {
	static bool skip_little_files = 1;
	static bool map_windows_folder = 0;

	static bool skip_cursed = 1;
	/*
		example of cursed dir is Program Files\WindowsApps
		windows apps is rlly broken
		check size of Program Files\ its gonna tell u that its lets say 50gb
		check size of Program Files\WindowsApps its gonna tell u that its 180gb

		inside there is a lot of symlinks and junctions that point to other places on the disk that arent normal symlinks
		hellalot of them is detectable only by windows api calls
	*/
	namespace fs = std::filesystem;

	constexpr int Tfile = 0;
	constexpr int Tdir = 1;
	constexpr int Troot = 2;

	class File : public std::enable_shared_from_this<File> {
		std::shared_ptr<File> parent;
		std::vector<std::shared_ptr<File>> childs;

		void setParent(std::shared_ptr<File> parent) {
			if (parent->type == Tfile)
				throw std::runtime_error("can not set file as parent");
			this->parent = parent;
		}

		std::shared_ptr<File> thisShared() {
			return shared_from_this();
		}

	protected:
		std::string path;
		uint8_t type;  // Tfile/Tdir/Troot

		friend class Drive;
	public:
		float size;
		std::string filename;
		std::string last_modified;
		bool user_have_access;
		explicit File(uint8_t type = Tfile, std::string filename = "", float size = 0, std::string last_modified = "", bool user_have_access = 1) {
			if (type != Tfile && type != Tdir && type != Troot)
				throw std::runtime_error("File type is not 0 (file) nor 1 (dir)");
			this->type = type;
			this->size = size;
			this->filename = filename;
			this->last_modified = last_modified;
			this->user_have_access = user_have_access;
		}

		int getType() {
			return type;
		}

		bool isDir() {
			return type != Tfile;
		}

		std::shared_ptr<File> getParent() {
			if (type == Troot) {
				return thisShared();
			}
			return parent;
		}

		// sorts childs by size
		void sortChilds() {
			std::sort(childs.begin(), childs.end(), [](std::shared_ptr<File>& a, std::shared_ptr<File>& b)->bool {return a->size > b->size; });
		}

		std::vector<std::shared_ptr<File>> getChilds() {
			return childs;
		}

		std::vector<std::string> getChildsFilenames() {
			std::vector<std::string> c;
			for (auto& e : childs) {
				c.push_back(e->filename);
			}
			return c;
		}

		std::string getPath() {
			return path;
		}

		bool operator>(const std::shared_ptr<File>& other) const {
			return this->size > other->size;
		}

		bool operator<(const std::shared_ptr<File>& other) const {
			return this->size < other->size;
		}

		bool operator==(const std::shared_ptr<File>& other) const {
			if (path == other->path)
				return true;
			return false;
		}

		bool operator!=(const std::shared_ptr<File>& other) const {
			if (path != other->path)
				return true;
			return false;
		}

		void addChild(std::shared_ptr<File> child) {
			if (type == Tfile)
				throw std::runtime_error("cant add child to a file");

			child->setParent(thisShared());
			childs.push_back(child);
			
			if (skip_little_files && child->size < 1)  // to make it faster, we dont care about files that are < 1MB
				return;

			std::shared_ptr<File> to_actualise = thisShared();
			size += child->size;
			while (to_actualise->getType() != Troot) {
				to_actualise = to_actualise->getParent();
				to_actualise->size += child->size;
			}
		}

		// just unlinks child from the tree
		void removeChild(std::shared_ptr<File> child) {
			if (child->parent != thisShared())
				throw std::runtime_error("cant delete file from folder that is not it's parent");

			for (int i = 0; i < childs.size(); i++) {
				if (child->filename == childs[i]->filename) {
					childs.erase(childs.begin() + i);
					break;
				}
			}

			if (skip_little_files && child->size < 1)  // to make it faster, we dont care about files that are < 1MB
				return;

			std::shared_ptr<File> to_actualise = thisShared();
			size -= child->size;
			while (to_actualise->getType() != Troot) {
				to_actualise = to_actualise->getParent();
				to_actualise->size -= child->size;
			}
		}

		// returns called object if not found
		std::shared_ptr<File> getChild(const std::string filename){
			for (auto& child : childs) {
				if (child->filename == filename)
					return child;
			}

			return thisShared();
		}
	};

	class Drive {
		int TotalSize;
		int UsedSize;
		char letter;
		std::string name;
		std::shared_ptr<File> root = std::make_shared<File>();
		std::vector<std::shared_ptr<File>> files_container;
		std::mutex m_new_file;

		void addNewFile(std::shared_ptr<File> file) {
			m_new_file.lock();
			files_container.push_back(file);
			m_new_file.unlock();
		}

		float BytesToMB(int64_t Bytes) {
			return Bytes / (1024 * 1024);
		}

		float getDriveSize() {
			fs::space_info si = fs::space(std::string(1, letter) + ":/");
			return BytesToMB(si.capacity);
		}

		float getDriveUsedSize() {
			fs::space_info si = fs::space(std::string(1, letter) + ":/");
			return BytesToMB(si.capacity - si.free);
		}

		std::string getDriveName() {
			char s[128];
			GetVolumeInformationA(std::string(std::string(1, letter) + ":\\").c_str() , s, 128, NULL, NULL, NULL, NULL, _MAX_PATH + 1);
			return std::string(s);
		}

		bool dirAccess(fs::path p) {
			try {
				std::filesystem::directory_iterator a(p);
			}
			catch (...) {
				return false;
			}
			return true;
		}

		bool fileAccess(fs::path p) {
			try {
				bool a = fs::is_directory(p);
			}
			catch (...) {
				return false;
			}
			return true;
		}

		void _mapDriveFast(std::shared_ptr<File> start) {
			std::vector<std::thread> threads;
			for (auto& file_dir : std::filesystem::directory_iterator(start->path)) {
				try {
					auto file = mapFile(file_dir, start);
					if (file->isDir() && file->user_have_access == true) {
						threads.push_back(std::thread(&Drive::_mapDriveFast, this, file));
					}
				}
				catch (...) {} // ignore access denied and symlinks
			}

			for (auto& t : threads) {
				t.join();
			}
		}

		void _mapDrive(std::shared_ptr<File> start) {  // bfs algo to map all files
			std::queue<std::shared_ptr<File>> Q;
			Q.push(start);

			while (!Q.empty()) {
				std::shared_ptr<File> dir = Q.front();
				Q.pop();

				for (auto& file_dir : std::filesystem::directory_iterator(dir->path)) {
					try {
						auto file = mapFile(file_dir, dir);
						if (file->isDir() && file->user_have_access == true) {
							Q.push(file);
						}
					}
					catch (...) {} // ignore access denied and symlinks
				}
			}
		}

		bool isCursedSystemDir(const fs::path p) {
			auto name = p.filename().string(); // just the last folder name

			// condition 1: name check (case-insensitive)
			std::string lowerName = name;
			std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::towlower);

			if (lowerName == "windowsapps" ||
				lowerName == "system volume information" ||
				lowerName == "$recycle.bin" ||
				lowerName == "config.msi") {
				return true;
			}

			// condition 2: check permissions (optional, avoids chasing unreadable dirs)
			try {
				auto perms = fs::status(p).permissions();
				if ((perms & fs::perms::owner_read) == fs::perms::none &&
					(perms & fs::perms::group_read) == fs::perms::none &&
					(perms & fs::perms::others_read) == fs::perms::none) {
					return true;
				}
			}
			catch (...) {
				// permission denied? treat as system
				return true;
			}

			return false;
		}

		std::shared_ptr<File> mapFile(fs::directory_entry file_dir, std::shared_ptr<File> parent) {
			if (!fileAccess(file_dir.path()))
				throw std::runtime_error("can't access file");
			if(fs::is_symlink(file_dir.path()))
				throw std::runtime_error("symlink detected");

			auto cfile = std::make_shared<File>();
			addNewFile(cfile);

			fs::path file(file_dir.path());

			if (fs::is_directory(file)) {
				cfile->type = Tdir;
			} else {
				cfile->type = Tfile;
			}

			bool undefined_filename = false;

			try {
    			auto u8_name = file.filename().u8string();
    			auto u8_full_path = file.u8string();

    			cfile->filename = std::string(reinterpret_cast<const char*>(u8_name.c_str()));
    			cfile->path = std::string(reinterpret_cast<const char*>(u8_full_path.c_str()));
			}
			catch (...) {
				cfile->filename = "undefined";
				cfile->path = parent->path + "\\" + "undefined";
				undefined_filename = true;
			}

			// get last modified time
			fs::file_time_type ftime = fs::last_write_time(file);
			std::stringstream ss;

			ss << ftime;
			ss >> cfile->last_modified;

			if (cfile->type == Tfile) {
				cfile->size = BytesToMB(fs::file_size(file));
			}

			parent->addChild(cfile);

			if (cfile->type == Tdir) {
				if (!dirAccess(file) || undefined_filename == true) {
					cfile->user_have_access = 0;
				}

				if (map_windows_folder == 0 && cfile->path == root->path + "Windows") {
					cfile->user_have_access = 0;
				}

				if(skip_cursed && isCursedSystemDir(cfile->path)) {
					cfile->user_have_access = 0;
				}
			}
			return cfile;
		}

	public:
		explicit Drive(char letter) {
			root->type = Troot;
			root->path = std::string(1, letter) + ":\\";
			files_container.push_back(root);

			this->letter = letter;
			this->TotalSize = getDriveSize();
			this->UsedSize = getDriveUsedSize();
			this->name = getDriveName();
		}

		Drive() {
			this->UsedSize = 0;
			this->TotalSize = 0;
			this->letter = 'C';
		}

		Drive& operator=(const Drive& d) {
			this->TotalSize = d.TotalSize;
			this->UsedSize = d.UsedSize;
			this->letter = d.letter;
			this->name = d.name;
			this->root = d.root;
			this->files_container = d.files_container;
			return *this;
		}

		// measured in MB
		int getTotalSize() {
			return TotalSize;
		}

		// measured in MB
		int getUsedSize() {
			return UsedSize;
		}

		char getLetter() {
			return letter;
		}

		std::string getName() {
			return name;
		}

		std::shared_ptr<File> getRoot() {
			return root;
		}

		// maps drive extremely fast with cost of high cpu usage
		void mapDriveFast() {
			_mapDriveFast(root);
		}

		void mapDrive() {
			std::vector<std::thread> threads;
			for (auto& file_dir : std::filesystem::directory_iterator(root->path)) {
				try {
					auto file = mapFile(file_dir, root);
					if (file->isDir() && file->user_have_access == true) {
						threads.push_back(std::thread(&Drive::_mapDrive, this, file));
					}
				}
				catch (...) {}
			}

			for (auto& t : threads) {
				t.join();
			}
		}
	};

	struct customFilesystem {
		std::map<char, Drive> Drives;

		// returns found drives letters
		std::vector<char> discoverDrives(){
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

		void runDebugMode() {
			Drives['C'] = Drive('C');
			Drives['C'].mapDriveFast();
			DEBUGwalkthrough('C');
		}

		// need to map drive first, type folder name to enter it, type exit to exit, type .. to enter parent
		void DEBUGwalkthrough(char drive) {
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
					std::cout <<std::left<< std::setw(64)<<file->filename+(file->isDir() ? "/" : "") 
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
	};
}  // namespace customFilesystem
