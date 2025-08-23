#pragma once
#include <string>
#include <vector>
#include <functional>

class PluginManager {
public:
    void LoadPlugins(const std::wstring& directory);
    void ForEach(const std::function<void(const std::wstring&)>& cb);
private:
    std::vector<std::wstring> plugins_;
};