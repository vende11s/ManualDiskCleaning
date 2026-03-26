// Copyright (c) 2025-2026 Michal Wierzbinski
#include "src/customFilesystem/File.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "src/customFilesystem/config.hpp"

namespace customFilesystem {
	void File::setParent(std::shared_ptr<File> parent) {
		if (parent->type == Tfile)
			throw std::runtime_error("can not set file as parent");
		this->parent = parent;
	}

	std::shared_ptr<File> File::thisShared() {
		return shared_from_this();
	}

	File::File(uint8_t type, std::string filename, float size, std::string last_modified, bool user_have_access) {
		if (type != Tfile && type != Tdir && type != Troot)
			throw std::runtime_error("File type is not 0 (file) nor 1 (dir)");
		this->type = type;
		this->size = size;
		this->filename = filename;
		this->last_modified = last_modified;
		this->user_have_access = user_have_access;
	}

	int File::getType() {
		return type;
	}

	bool File::isDir() {
		return type != Tfile;
	}

	std::shared_ptr<File> File::getParent() {
		if (type == Troot) {
			return thisShared();
		}
		return parent;
	}

	// sorts childs by size
	void File::sortChilds() {
		std::sort(childs.begin(), childs.end(), [](std::shared_ptr<File>& a, std::shared_ptr<File>& b)->bool {return a->size > b->size; });
	}

	std::vector<std::shared_ptr<File>> File::getChilds() {
		return childs;
	}

	std::vector<std::string> File::getChildsFilenames() {
		std::vector<std::string> c;
		for (auto& e : childs) {
			c.push_back(e->filename);
		}
		return c;
	}

	std::string File::getPath() {
		return path;
	}

	bool File::operator>(const std::shared_ptr<File>& other) const {
		return this->size > other->size;
	}

	bool File::operator<(const std::shared_ptr<File>& other) const {
		return this->size < other->size;
	}

	bool File::operator==(const std::shared_ptr<File>& other) const {
		if (path == other->path)
			return true;
		return false;
	}

	bool File::operator!=(const std::shared_ptr<File>& other) const {
		if (path != other->path)
			return true;
		return false;
	}

	void File::addChild(std::shared_ptr<File> child) {
		if (type == Tfile)
			throw std::runtime_error("cant add child to a file");

		child->setParent(thisShared());
		childs.push_back(child);
	}

	void File::calculateTreeSize() {
		if (type == Tfile) return;
		this->size = 0;

		for (auto& child : childs) {
			if (child->type == Tdir) {
				child->calculateTreeSize();
			}
			this->size += child->size;
		}
	}

	// just unlinks child from the tree
	void File::removeChild(std::shared_ptr<File> child) {
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
	std::shared_ptr<File> File::getChild(const std::string& filename) {
		for (auto& child : childs) {
			if (child->filename == filename)
				return child;
		}

		return thisShared();
	}
}  // namespace customFilesystem
