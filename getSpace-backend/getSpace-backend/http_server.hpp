#pragma once
#include <crow.h>

#include <string>
#include <utility>
#include <chrono>
#include <atomic>

#include <nlohmann/json.hpp>

#include "utils.hpp"
#include "customFilesystem.hpp"

using json = nlohmann::json;
namespace cfs = customFilesystem;

namespace http_server {
	bool done_mapping = false;
    void map_drive(cfs::customFilesystem& cfs, char letter, bool fast) {
        if (fast) {
            cfs.Drives[letter].mapDriveFast();
        }
        else {
            cfs.Drives[letter].mapDrive();
        }
		done_mapping = true;
    }

    void http_server(cfs::customFilesystem& cfs, int port) {
        crow::SimpleApp app;
        CROW_ROUTE(app, "/init_disks")([&cfs]() { // Capture cfs by reference
            utils::LOG("HTTP: /init_disks called");
            json response = json::parse(R"(
                {
                    "disks": []
                }
             )");

            for (auto& [letter, drive] : cfs.Drives) {
                std::string label = drive.getName() + " (" + drive.getLetter() + ":) - ";
                label += utils::cutPrecision(utils::MBtoGB(drive.getUsedSize())) +
                    "/" + utils::cutPrecision(utils::MBtoGB(drive.getTotalSize())) + "GB Used";

                float used = ((float)drive.getUsedSize() / (float)drive.getTotalSize()) * 100.0f;
                response["disks"].push_back({
                    {"id", std::to_string(letter)},
                    {"label", label},
                    {"used", utils::cutPrecision(used)}
                    });
            }
            return crow::response(response.dump());
            });

        char drive_mapped = 0;
        // params are : <drive_letter><fast(0/1)><windows(0/1)><skip_small_files(0/1)>
        // example : C101  -> map drive C, fast, skip windows folder, skip small files
        CROW_ROUTE(app, "/map/<string>")([&cfs, &drive_mapped](std::string params) {
            utils::LOG("HTTP: /map/" + params + " called");
            if (drive_mapped != 0) {
                return crow::response(400, "Already mapped");
            }

            if (params.size() != 4) {
                return crow::response(400, "Invalid parameters");
            }
            char drive_letter = params[0];
            bool fast = params[1] - '0';
            bool windows = params[2] - '0';
            bool skip_small = params[3] - '0';
            drive_mapped = drive_letter;

            cfs::skip_little_files = skip_small;
            cfs::map_windows_folder = windows;

            utils::LOG("mapping drive " + drive_letter);
            map_drive(cfs, drive_letter, fast);
            return crow::response(200, "Drive mapped");
            });

        CROW_ROUTE(app, "/map_info")([&cfs, &drive_mapped]() {
            utils::LOG("HTTP: /map_info called");
            if (drive_mapped == 0) {
                return crow::response(400, "No drive mapped");
            }
            if (done_mapping) {
                return crow::response(200, "done");
            }

            float mapped_gb = cfs.Drives[drive_mapped].mapped_bytes.load() / (1024.0f * 1024.0f * 1024.0f);
            float progress = ((mapped_gb) / cfs.Drives[drive_mapped].getUsedSize()) * 100;
            std::string progress_label = utils::cutPrecision(mapped_gb);
            progress_label += "/" + utils::cutPrecision(utils::MBtoGB(cfs.Drives[drive_mapped].getUsedSize())) + " GB";

            json response;
            response["progress"] = std::to_string(progress);
            response["progress_label"] = "mapped " + progress_label;
            return crow::response(response.dump());
            });

        struct removed_file_info {
            std::string path;
            std::string size_label;
            std::string type; // "file" or "folder"
        };
        std::vector<removed_file_info> removed_files;
        float freed_space = 0.0f; // in MB
        float space_to_free = 0.0f; // in MB

        std::shared_ptr<cfs::File> current_dir;

        CROW_ROUTE(app, "/get_interface_data/<int>")([&cfs, &drive_mapped, &current_dir, &removed_files, &freed_space, &space_to_free](int offset) {
            utils::LOG("HTTP: /get_interface_data called");
            if (drive_mapped == 0) {
                return crow::response(400, "No drive mapped");
            }

            if (current_dir == nullptr) { // first call
                current_dir = cfs.Drives[drive_mapped].getRoot();
                space_to_free = current_dir->size;
            }

            json response;
            response["current_path"] = current_dir->getPath();
            response["removed_label"] = utils::cutPrecision(utils::MBtoGB(freed_space)) + "/" + utils::cutPrecision(utils::MBtoGB(space_to_free)) + " GB";
            response["removed_percent"] = utils::cutPrecision((freed_space / space_to_free) * 100.0f);
            response["last_removed"] = json::array();

            for (int i = 0; i < removed_files.size(); i++) {
                response["last_removed"].push_back({
                    {"id", std::to_string(i)},
                    {"path", removed_files[i].path},
                    {"size_label", removed_files[i].size_label},
                    {"type", removed_files[i].type} });
            }

            response["dir_size"] = current_dir->getChilds().size();
            response["dir_info"] = std::to_string(current_dir->getChilds().size()) +
                " files in this directory | " + utils::cutPrecision(utils::MBtoGB(current_dir->size)) + "GB";
            response["files"] = json::array();

            current_dir->sortChilds();
            auto childs = current_dir->getChilds();
            for (int i = offset; i < offset + 500 && i < childs.size(); i++) {
                response["files"].push_back({
                    {"id", std::to_string(i)},
                    {"name", childs[i]->filename},
                    {"type", childs[i]->isDir() ? "folder" : "file"},
                    {"size", utils::createSizeLabel(childs[i]->size)},
                    {"date_label", childs[i]->last_modified}
                    });
            }
            return crow::response(response.dump());
            });

        CROW_ROUTE(app, "/change_dir/<string>")([&cfs, &drive_mapped, &current_dir](std::string id) {
            utils::LOG("HTTP: /change_dir/" + id + " called");
            if (drive_mapped == 0) {
                return crow::response(400, "No drive mapped");
            }

            if (id == "up-item") {
                current_dir = current_dir->getParent();
                return crow::response(200, "Changed directory");
            }

            int index = std::stoi(id);
            auto childs = current_dir->getChilds();

            if (index < 0 || index >= childs.size()) {
                return crow::response(400, "Invalid index");
            }

            if (childs[index]->isDir() == false) {
                return crow::response(400, "Not a directory");
            }

            current_dir = childs[index];
            return crow::response(200, "Changed directory");
            });

        CROW_ROUTE(app, "/remove/<string>")([&cfs, &drive_mapped, &current_dir, &removed_files, &freed_space](std::string id) {
            utils::LOG("HTTP: /remove/" + id + " called");
            if (drive_mapped == 0) {
                return crow::response(400, "No drive mapped");
            }

            if (id == "up-item") {
                return crow::response(400, "Cant remove parent");
            }

            int index = std::stoi(id);
            auto childs = current_dir->getChilds();

            if (index < 0 || index >= childs.size()) {
                return crow::response(400, "Invalid index");
            }

            removed_file_info file;
            file.path = childs[index]->getPath();
            file.size_label = utils::createSizeLabel(childs[index]->size);
            file.type = childs[index]->isDir() ? "folder" : "file";

            if (utils::moveToRecycleBin(file.path)) {
                freed_space += childs[index]->size;
                childs[index]->getParent()->removeChild(childs[index]);
                removed_files.push_back(file);

                return crow::response(200, "Removed file");
            }
            else return crow::response(500, "Failed to remove file");
        });

        CROW_ROUTE(app, "/open")([&current_dir]() {
            utils::LOG("HTTP: /open/ called");
            utils::openInExplorer(current_dir->getPath());
            return crow::response(200, "Opened in explorer");
        });

        app.port(port).multithreaded().run();
	}
} // namespace http_server