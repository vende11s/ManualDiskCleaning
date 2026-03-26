// Copyright (c) 2025-2026 Michal Wierzbinski
#include "src/customFilesystem/Drive.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <utility>
#include <algorithm>

#include <Windows.h>

#include "src/customFilesystem/config.hpp"
#include "src/customFilesystem/File.hpp"


namespace customFilesystem {
		namespace fs = std::filesystem;
		float Drive::BytesToMB(int64_t Bytes) {
			return Bytes / (1024 * 1024);
		}

		float Drive::getDriveSize() {
			fs::space_info si = fs::space(std::string(1, letter) + ":/");
			return BytesToMB(si.capacity);
		}

		float Drive::getDriveUsedSize() {
			fs::space_info si = fs::space(std::string(1, letter) + ":/");
			return BytesToMB(si.capacity - si.free);
		}

		std::string Drive::getDriveName() {
			char s[128];
			GetVolumeInformationA(std::string(std::string(1, letter) + ":\\").c_str(), s, 128, NULL, NULL, NULL, NULL, _MAX_PATH + 1);
			return std::string(s);
		}

		bool Drive::dirAccess(fs::path p) {
			try {
				std::filesystem::directory_iterator a(p);
			}
			catch (...) {
				return false;
			}
			return true;
		}

		bool Drive::fileAccess(fs::path p) {
			try {
				bool a = fs::is_directory(p);
			}
			catch (...) {
				return false;
			}
			return true;
		}

		void Drive::_mapDriveFast(std::shared_ptr<File> start) {
			std::u8string u8_dir(start->path.begin(), start->path.end());
			std::filesystem::path safe_path(u8_dir);

			// task queue to store directories that need to be processed, each task is a pair of (path, corresponding File object)
			std::queue<std::pair<std::filesystem::path, std::shared_ptr<File>>> task_queue;
			task_queue.push({ safe_path, start });

			std::mutex queue_mutex;  // note: maybe we can get rid of this mutex by giving each thread its own queue and task stealing?
			std::condition_variable cv;

			// counters to track active tasks and signal when done
			int active_tasks{ 0 };  // no need for atomic since it's only modified under the queue_mutex lock
			bool done{ false };

			// checking how many hardware threads there are
			uint16_t num_threads = std::thread::hardware_concurrency();
			if (num_threads == 0) num_threads = 8;  // fallback if OS can't detect

			files_container.resize(num_threads);  // one private container for each thread so we are mutex free :)
			for (auto& e : files_container) {
				e.reserve(10000);  // to avoid too many reallocations
			}

			std::vector<std::thread> workers;

			for (uint16_t i = 0; i < num_threads; i++) {
				workers.emplace_back([&, i]() {
					while (true) {
						std::pair<std::filesystem::path, std::shared_ptr<File>> task;

						{  // manipulating the scope to free lock

							// thread waits until there is a task in the queue or all tasks are done
							std::unique_lock<std::mutex> lock(queue_mutex);
							cv.wait(lock, [&]() {
								return !task_queue.empty() || (done && active_tasks == 0);
								});

							if (done && task_queue.empty() && active_tasks == 0) return;

							task = task_queue.front();
							task_queue.pop();
							active_tasks++;
						}

						// mapping only the current directory and adding subdirectories to the queue
						try {
							auto options = std::filesystem::directory_options::skip_permission_denied;
							for (auto& file_dir : std::filesystem::directory_iterator(task.first, options)) {
								try {
									auto file = mapFile(file_dir, task.second);

									if (file == nullptr) continue;  // if file is nullptr, it means we dont have access to it or its a symlink, so we skip it

									// each thread has its own container
									// no mutex needed
									files_container[i].push_back(file);  // storing pointer so it doesnt get killed by scope

									if (file->isDir() && file->user_have_access) {
										std::lock_guard<std::mutex> lock(queue_mutex);
										task_queue.push({ file_dir.path(), file });
										cv.notify_one();  // waking one thread to process the new task
									}
								}
								catch (...) {}
							}
						}
						catch (...) {}

						{
							std::lock_guard<std::mutex> lock(queue_mutex);
							active_tasks--;
							if (task_queue.empty() && active_tasks == 0) {
								done = true;
								cv.notify_all();  // waking all threads to finish
							}
						}
					}
					});
			}

			// waitig for all threads to finish
			for (auto& t : workers) {
				if (t.joinable()) t.join();
			}
		}

		bool Drive::isCursedSystemDir(const fs::path p) {
			auto name = p.filename().string();  // just the last folder name

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

		std::shared_ptr<File> Drive::mapFile(fs::directory_entry file_dir, std::shared_ptr<File> parent) {
			if (!fileAccess(file_dir.path()))
				return nullptr;
			if (fs::is_symlink(file_dir.path()))
				return nullptr;

			auto cfile = std::make_shared<File>();

			if (file_dir.is_directory()) {
				cfile->type = Tdir;
			} else {
				cfile->type = Tfile;
			}

			if (cfile->type == Tfile) {
				cfile->size = BytesToMB(file_dir.file_size());
			}

			if (skip_little_files && cfile->type == Tfile && cfile->size < 1) {
				return nullptr;
			}

			fs::path file(file_dir.path());

			bool undefined_filename = false;
			try {
				auto u8_name = file.filename().u8string();
				auto u8_full_path = file.u8string();

				cfile->filename = std::string(u8_name.begin(), u8_name.end());
				cfile->path = std::string(u8_full_path.begin(), u8_full_path.end());
			}
			catch (...) {
				cfile->filename = "undefined";
				cfile->path = parent->path + "\\" + "undefined";
				undefined_filename = true;
			}

			// get last modified time
			fs::file_time_type ftime = file_dir.last_write_time();
			std::stringstream ss;

			ss << ftime;
			ss >> cfile->last_modified;

			parent->addChild(cfile);

			if (cfile->type == Tdir) {
				if (!dirAccess(file) || undefined_filename == true) {
					cfile->user_have_access = 0;
				}

				if (map_windows_folder == 0 && cfile->path == root->path + "Windows") {
					cfile->user_have_access = 0;
				}

				if (skip_cursed && isCursedSystemDir(cfile->path)) {
					cfile->user_have_access = 0;
				}
			}

			if (!cfile->isDir()) {
				mapped_bytes += file_dir.file_size();
			}

			items_mapped++;

			return cfile;
		}

		Drive::Drive(char letter) {
			root->type = Troot;
			root->path = std::string(1, letter) + ":\\";
			files_container.resize(1);
			files_container[0].push_back(root);

			this->letter = letter;
			this->TotalSize = getDriveSize();
			this->UsedSize = getDriveUsedSize();
			this->name = getDriveName();
		}

		Drive::Drive() {
			this->letter = 'C';
		}

		Drive& Drive::operator=(const Drive& d) {
			this->TotalSize = d.TotalSize;
			this->UsedSize = d.UsedSize;
			this->letter = d.letter;
			this->name = d.name;
			this->root = d.root;
			this->files_container = d.files_container;
			return *this;
		}

		// measured in MB
		int Drive::getTotalSize() {
			return TotalSize;
		}

		// measured in MB
		int Drive::getUsedSize() {
			return UsedSize;
		}

		char Drive::getLetter() {
			return letter;
		}

		std::string Drive::getName() {
			return name;
		}

		std::shared_ptr<File> Drive::getRoot() {
			return root;
		}

		// maps drive extremely fast with cost of high cpu usage
		void Drive::mapDriveFast() {
			_mapDriveFast(root);
			root->calculateTreeSize();
		}

		// maps drive using single thread, slower but low cpu usage.
		void Drive::mapDrive() {
			std::queue<std::shared_ptr<File>> Q;
			Q.push(root);

			while (!Q.empty()) {
				std::shared_ptr<File> dir = Q.front();
				Q.pop();

				std::u8string u8_dir(dir->path.begin(), dir->path.end());
				std::filesystem::path safe_path(u8_dir);

				for (auto& file_dir : std::filesystem::directory_iterator(safe_path)) {
					try {
						auto file = mapFile(file_dir, dir);

						if (file == nullptr) continue;  // if file is nullptr, it means we dont have access to it or its a symlink, so we skip it

						if (file->isDir() && file->user_have_access == true) {
							Q.push(file);
						}
						files_container[0].push_back(file);  // storing shared pointer in the container so it doesnt get killed by scope
					}
					catch (...) {}  // ignore access denied and symlinks
				}
			}
			root->calculateTreeSize();
		}
}  // namespace customFilesystem
