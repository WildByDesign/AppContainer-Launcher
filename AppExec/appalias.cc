/////////////
#include <json.hpp>
#ifndef _WINDOWS_
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN //
#endif
#include <Windows.h>
#endif
#include <Shlwapi.h>
#include <PathCch.h>
#include <string>
#include <fstream>
#include <iomanip>
#include "app.hpp"

bool PathAppImageCombine(std::wstring &path, const wchar_t *file) {
  if (PathFileExistsW(file)) {
    path.assign(file);
    return true;
  }
  path.resize(PATHCCH_MAX_CCH, L'\0');
  auto N = GetModuleFileNameW(nullptr, &path[0], PATHCCH_MAX_CCH);
  path.resize(N);
  auto pos = path.rfind(L'\\');
  if (pos != std::wstring::npos) {
    path.resize(pos);
  }
  path.append(L"\\").append(file);
  return true;
}

/// PathAppImageCombineExists
bool PathAppImageCombineExists(std::wstring &path, const wchar_t *file) {
  if (!PathAppImageCombine(path, file)) {
    return false;
  }
  if (PathFileExistsW(path.data())) {
    return true;
  }
  path.clear();
  return false;
}

inline std::wstring utf8towide(std::string_view str) {
  std::wstring wstr;
  auto N =
      MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
  if (N > 0) {
    wstr.resize(N);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &wstr[0], N);
  }
  return wstr;
}

namespace priv {

bool AppThemeLookup(DWORD &dwcolor) {
  std::wstring file;
  if (!PathAppImageCombineExists(file, L"AppExec.json")) {
    return false;
  }
  try {
    std::ifstream fs;
    fs.open(file);
    auto json = nlohmann::json::parse(fs);
    auto root = json["AppExec"];
    dwcolor = root["Background"].get<uint32_t>();
  } catch (const std::exception &e) {
    OutputDebugStringA(e.what());
    return false;
  }
  return true;
}

bool AppThemeApply(DWORD color) {
  std::wstring file;
  if (!PathAppImageCombine(file, L"AppExec.json")) {
    return false;
  }
  try {
    std::ofstream o;
    o.open(file, std::ios::out);
    nlohmann::json a;
    a["Background"] = color;
    nlohmann::json v;
    v["AppExec"] = a;
    o << std::setw(4) << v << std::endl;
  } catch (const std::exception &e) {
    OutputDebugStringA(e.what());
    return false;
  }
  return true;
}

bool AppAliasInitialize(HWND hbox, priv::alias_t &alias) {
  std::wstring file;
  if (!PathAppImageCombineExists(file, L"Privexec.json")) {
    return false;
  }
  try {
    std::ifstream fs;
    fs.open(file);
    auto json = nlohmann::json::parse(fs);
    auto cmds = json["Alias"];
    for (auto &cmd : cmds) {
      auto desc = utf8towide(cmd["Desc"].get<std::string>());
      auto target = utf8towide(cmd["Target"].get<std::string>());
      alias.insert(std::make_pair(desc, target));
      ::SendMessage(hbox, CB_ADDSTRING, 0, (LPARAM)desc.data());
    }
  } catch (const std::exception &e) {
    OutputDebugStringA(e.what());
    return false;
  }
  return true;
}
} // namespace priv