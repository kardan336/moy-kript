#include "plugin_manager.h"
#include <windows.h>

void PluginManager::LoadPlugins(const std::wstring& directory) {
    plugins_.clear();
    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW((directory + L"\\*.dll").c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            plugins_.push_back(directory + L"\\" + fd.cFileName);
        } while (FindNextFileW(hFind, &fd));
        FindClose(hFind);
    }
}
void PluginManager::ForEach(const std::function<void(const std::wstring&)>& cb) {
    for (auto& p : plugins_) cb(p);
}