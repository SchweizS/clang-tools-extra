//===--- HeaderGuardCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "HeaderGuardCheck.h"

namespace clang {
namespace tidy {
namespace misc {

const HeaderGuardCheck::StyleEntry DefinedStyles[] = {
    {"default", {{"include/"}, {"src/", "_SRC_"}}, {}, true},
};

void HeaderGuardCheck::SelectedStyleEntry::setBaseStyle(
    const StyleEntry *const Style, const StringRef Name) {
  this->RawBaseDirectories.clear();
  this->RawReplaceDirectories.clear();
  this->RawEndIf.clear();

  this->BaseDirectories = Style->BaseDirectories;
  this->ReplaceDirectories = Style->ReplaceDirectories;
  this->Name = Style->Name;
  this->EndIf = Style->EndIf;
  this->RawName = Name;
};
void HeaderGuardCheck::SelectedStyleEntry::setEndIf(std::string EndIf) {
  this->RawEndIf = std::move(EndIf);
  if (this->RawEndIf == "'") {
    return;
  }
  this->EndIf = this->RawEndIf.find('+') != std::string::npos;
}

template <class T>
inline bool prepareData(std::string &data, std::vector<T> &vec,
                        std::string &raw) {
  raw = std::move(data);
  if (raw == "'")
    return false;
  vec.clear();
  if (raw.empty()) {
    return false;
  }
  std::replace(raw.begin(), raw.end(), '\\', '/');
  return true;
};

StringRef splitAt(StringRef& Data, const char Split) {
  if (Data.empty()) return "";
  auto pos = Data.find(Split);
  auto ret = Data.substr(0, pos);
  if (pos==StringRef::npos) {
    Data = "";
  } else {
    Data = Data.substr(pos+1);
  }
  return ret;
} 

void HeaderGuardCheck::SelectedStyleEntry::setBaseDirectories(
    std::string BaseDirectories) {
  if (!prepareData(BaseDirectories, this->BaseDirectories,
                   this->RawBaseDirectories)) {
    return;
  }
  StringRef Data = this->RawBaseDirectories;
  while (Data.size()>0) {
    auto sub = splitAt(Data, ',');
    auto sub2 = splitAt(sub, '=');
    this->BaseDirectories.emplace_back(sub2, sub);
  }
}

void HeaderGuardCheck::SelectedStyleEntry::setReplaceDirectories(
    std::string ReplaceDirectories) {
  if (!prepareData(ReplaceDirectories, this->ReplaceDirectories,
                   this->RawReplaceDirectories)) {
    return;
  }
  StringRef Data = this->RawReplaceDirectories;
  while (Data.size()>0) {
    auto sub = splitAt(Data, ',');
    auto sub2 = splitAt(sub, '=');
    this->BaseDirectories.emplace_back(sub2, sub);
  }
}

HeaderGuardCheck::HeaderGuardCheck(StringRef Name, ClangTidyContext *Context)
    : utils::HeaderGuardCheck(Name, Context) {
  loadOptions();
}

std::string HeaderGuardCheck::getHeaderGuard(StringRef Filename,
                                             StringRef OldGuard) {
  std::string Guard = tooling::getAbsolutePath(Filename);

  std::replace(Guard.begin(), Guard.end(), '\\', '/');

  for (auto &entry : Style.ReplaceDirectories) {
    size_t PosReplace = Guard.rfind(entry.Name);
    if (PosReplace != StringRef::npos)
      Guard = Guard.replace(PosReplace, entry.Name.size(), entry.Replacement);
  }
  for (auto &entry : Style.BaseDirectories) {
    size_t PosInclude = Guard.rfind(entry.Name);
    if (PosInclude != StringRef::npos) {
      Guard = entry.Replacement.str() +
              Guard.substr(PosInclude + entry.Name.size());
    }
  }

  std::replace(Guard.begin(), Guard.end(), '/', '_');
  std::replace(Guard.begin(), Guard.end(), '.', '_');
  std::replace(Guard.begin(), Guard.end(), '-', '_');

  return StringRef(Guard).upper();
}

void HeaderGuardCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  auto store = [this, &Opts](const char* Name, const std::string& Value) {
    if (Value=="'") return;
    Options.store(Opts, Name, Value);
  };
  store("BaseDirs", Style.getBaseDirString());
  store("RenameDirs", Style.getReplaceDirString());
  store("Style", Style.getStyleName());
  store("EndIf", Style.getEndIf());
}


void HeaderGuardCheck::loadOptions() {

  auto BaseDirs = Options.get("BaseDirs", "'");
  auto RenameDirs = Options.get("RenameDirs", "'");
  auto Style = Options.get("Style", "'");
  auto EndIf = Options.get("EndIf", "'");

  selectStyleByName(Style);
  this->Style.setBaseDirectories(std::move(BaseDirs));
  this->Style.setReplaceDirectories(std::move(RenameDirs));
  this->Style.setEndIf(std::move(EndIf));
}

void HeaderGuardCheck::selectStyleByName(StringRef Name) {
  if (Name == "'" ||  Name.empty()) {
    Style.setBaseStyle(&DefinedStyles[0], Name);
    return;
  }
  for (auto i = 0u; i < std::extent_v<decltype(DefinedStyles)>; ++i) {
    if (Name == DefinedStyles[i].Name) {
      Style.setBaseStyle(&DefinedStyles[i], Name);
      return;
    }
  };
  Style.setBaseStyle(&DefinedStyles[0], Name);
}
} // namespace misc
} // namespace tidy
} // namespace clang
