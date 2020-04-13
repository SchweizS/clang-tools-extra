//===--- HeaderGuardCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "HeaderGuardCheck.h"

#include <type_traits>
namespace clang {
namespace tidy {
namespace misc {

HeaderGuardCheck::HeaderGuardCheck(StringRef Name, ClangTidyContext *Context)
    : utils::HeaderGuardCheck(Name, Context) {
  loadOptions();
}

std::string HeaderGuardCheck::getHeaderGuard(StringRef Filename,
                                             StringRef OldGuard) {
  std::string Guard = tooling::getAbsolutePath(Filename);

  // Sanitize the path. There are some rules for compatibility with the historic
  // style in include/llvm and include/clang which we want to preserve.
  std::replace(Guard.begin(), Guard.end(), '\\', '/');

  for (auto &entry : RenameDirs) {
    // We don't want _INCLUDE_ in our guards.
    size_t PosReplace = Guard.rfind(entry.first);
    if (PosReplace != StringRef::npos)
      Guard = Guard.replace(PosReplace, entry.first.size(), entry.second);
  }
  for (auto &entry : BaseDirs) {
    // We don't want _SRC_ in our guards.
    size_t PosInclude = Guard.rfind(entry);
    if (PosInclude != StringRef::npos)
      Guard = Guard.substr(PosInclude + entry.size());
  }

  std::replace(Guard.begin(), Guard.end(), '/', '_');
  std::replace(Guard.begin(), Guard.end(), '.', '_');
  std::replace(Guard.begin(), Guard.end(), '-', '_');

  // The prevalent style in clang is LLVM_CLANG_FOO_BAR_H
  if (StringRef(Guard).startswith("clang"))
    Guard = "LLVM_" + Guard;

  return StringRef(Guard).upper();
}

void HeaderGuardCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "BaseDirs", _BaseDirs);
  Options.store(Opts, "RenameDirs", _RenameDirs);
  Options.store(Opts, "Style", _Style);
}
const HeaderGuardCheck::base *
HeaderGuardCheck::getStyleByName(const std::string &name) {
  if (name.empty()) {
    return &_def;
  }
  for (auto i = 0u; i < std::extent_v<decltype(Styles)>; ++i) {
    if (name == Styles[i]->_name()) {
      return Styles[i];
    }
  };
  return &_def;
}

void HeaderGuardCheck::loadOptions() {
  _BaseDirs = Options.get("BaseDirs", "");
  _RenameDirs = Options.get("RenameDirs", "");
  _Style = Options.get("Style", "");

  Style = getStyleByName(_Style);
  if (_BaseDirs.empty()) {
    auto val = Style->_dirs();
    for (auto i = 0u; i < Style->_dirs_size(); ++i) {
      auto _val = val[i];
      if (_val == nullptr) {
        continue;
      }
      BaseDirs.emplace_back(_val);
    }
  }
  if (_RenameDirs.empty()) {
    auto val = Style->_repl();
    for (auto i = 0u; i < Style->_repl_size(); ++i) {
      auto _val = val[i];
      if (_val[0] == nullptr || _val[1] == nullptr) {
        continue;
      }
      RenameDirs.emplace_back(_val[0], _val[1]);
    }
  }
  // TODO: non defaults
}
} // namespace misc
} // namespace tidy
} // namespace clang
