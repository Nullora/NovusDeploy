#include <string>
#include <iostream>
#include <fstream>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <libgen.h>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstring>
#include <unistd.h>

#define WATCH_FILE "/home/nullora/novusdeploy/.ndeploy/auto_watch.nd"
#define EVENT_SIZE  (sizeof(struct inotify_event))
#define EVENT_BUF   (1024 * (EVENT_SIZE + 16))

struct entry {
    std::string src;      // full path to file
    std::string filename; // basename only, for matching event->name
    std::vector<std::string> dests;
};

int fd = inotify_init1(IN_CLOEXEC);
// keyed by watch descriptor of the *directory*
std::unordered_map<int, entry> autoFiles;
std::mutex autoMutex;

// Returns the directory part of a path
std::string dirOf(const std::string& path) {
    char buf[4096];
    strncpy(buf, path.c_str(), sizeof(buf) - 1);
    return std::string(dirname(buf));
}

// Returns the filename part of a path
std::string baseOf(const std::string& path) {
    char buf[4096];
    strncpy(buf, path.c_str(), sizeof(buf) - 1);
    return std::string(basename(buf));
}

// Parse and reload the auto_watch.nd config into autoFiles
void reloadConfig() {
    std::ifstream in(WATCH_FILE);
    if (!in.is_open()) return;

    std::unordered_map<int, entry> newFiles;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream ss(line);
        std::string tag, src, destsStr;
        ss >> tag >> src >> destsStr;
        if (src.empty()) continue;

        entry e;
        e.src = src;
        e.filename = baseOf(src);

        std::istringstream dss(destsStr);
        std::string dest;
        while (std::getline(dss, dest, ',')) {
            if (!dest.empty())
                e.dests.push_back(dest);
        }

        // Watch the directory, not the file — catches gcc/editor renames
        std::string dir = dirOf(src);
        int wd = inotify_add_watch(fd, dir.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd < 0) {
            std::cerr << "[auto] failed to watch dir: " << dir << "\n";
            continue;
        }
        newFiles[wd] = e;
    }

    std::lock_guard<std::mutex> lock(autoMutex);
    autoFiles = newFiles;
}

// Thread 1: reload config every 200ms
void configWatcher() {
    while (true) {
        reloadConfig();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// Thread 2: block on inotify events and deploy on changes
void eventWatcher() {
    char buf[EVENT_BUF];
    while (true) {
        int len = read(fd, buf, EVENT_BUF);
        if (len < 0) {
            std::cerr << "[auto] inotify read error\n";
            continue;
        }

        int i = 0;
        while (i < len) {
            struct inotify_event* event = (struct inotify_event*)&buf[i];

            if (event->mask & (IN_CLOSE_WRITE | IN_MOVED_TO)) {
                std::string evName = (event->len > 0) ? std::string(event->name) : "";
                std::lock_guard<std::mutex> lock(autoMutex);
                auto it = autoFiles.find(event->wd);
                if (it != autoFiles.end()) {
                    entry& e = it->second;
                    if (!evName.empty() && evName != e.filename) {
                        i += EVENT_SIZE + event->len;
                        continue;
                    }
                    std::cout << "[auto] change detected: " << e.src << "\n";
                    for (auto& dest : e.dests) {
                        std::string cmd = "cp " + e.src + " " + dest;
                        int ret = system(cmd.c_str());
                        if (ret == 0)
                            std::cout << "[auto] deployed -> " << dest << "\n";
                        else
                            std::cerr << "[auto] deploy failed -> " << dest << "\n";
                    }
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }
}

int main() {
    if (fd < 0) {
        std::cerr << "[auto] inotify_init failed\n";
        return 1;
    }

    // Initial load
    reloadConfig();
    std::cout << "[auto] watching " << autoFiles.size() << " file(s)...\n";

    std::thread(configWatcher).detach();
    std::thread(eventWatcher).detach();

    // Keep main alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}