// Copyright (c) 2025-2026 Michal Wierzbinski
#pragma once

#include <memory>
#include <vector>
#include <string>

#include "src/customFilesystem/config.hpp"

namespace customFilesystem {
	class File : public std::enable_shared_from_this<File> {
		std::shared_ptr<File> parent;
		std::vector<std::shared_ptr<File>> childs;
		void setParent(std::shared_ptr<File> parent);
		std::shared_ptr<File> thisShared();

	protected:
		std::string path;
		uint8_t type;  // Tfile/Tdir/Troot
		friend class Drive;

	public:
		float size;
		std::string filename;
		std::string last_modified;
		bool user_have_access;
		explicit File(uint8_t type = Tfile, std::string filename = "", float size = 0, std::string last_modified = "", bool user_have_access = 1);

		int getType();
		bool isDir();
		std::shared_ptr<File> getParent();
		void sortChilds();
		std::vector<std::shared_ptr<File>> getChilds();
		std::vector<std::string> getChildsFilenames();
		std::string getPath();

		bool operator>(const std::shared_ptr<File>& other) const;
		bool operator<(const std::shared_ptr<File>& other) const;
		bool operator==(const std::shared_ptr<File>& other) const;
		bool operator!=(const std::shared_ptr<File>& other) const;

		void addChild(std::shared_ptr<File> child);
		void calculateTreeSize();

		void removeChild(std::shared_ptr<File> child);

		std::shared_ptr<File> getChild(const std::string& filename);
	};
}  // namespace customFilesystem
