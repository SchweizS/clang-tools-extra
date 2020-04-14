//===--- HeaderGuardCheck.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_HEADERGUARDCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_HEADERGUARDCHECK_H

#include "../utils/HeaderGuard.h"
#include <vector>
namespace clang {
namespace tidy {
namespace misc {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc-header-guard.html

class HeaderGuardCheck : public utils::HeaderGuardCheck {

public:
  struct StyleEntry {
    struct BaseDirectoryEntry {
      BaseDirectoryEntry(StringRef name, StringRef replacement = {})
          : Name(name), Replacement(replacement) {}
      StringRef Name;
      StringRef Replacement;
    };
    struct ReplaceDirectoryEntry {
      ReplaceDirectoryEntry(StringRef name, StringRef replacement)
          : Name(name), Replacement(replacement) {}
      StringRef Name;
      StringRef Replacement;
    };
    StringRef Name;
    std::vector<BaseDirectoryEntry> BaseDirectories;
    std::vector<ReplaceDirectoryEntry> ReplaceDirectories;
    bool EndIf;
  };

private:
  struct SelectedStyleEntry: StyleEntry {
  public:
    void setBaseStyle(const StyleEntry* const Style = nullptr, const StringRef Name = {});
    void setReplaceDirectories(std::string ReplaceDirectories);
    void setBaseDirectories(std::string BaseDirectories);
    void setEndIf(std::string EndIf);
    const std::string& getStyleName()  const { return RawName; }
    const std::string& getReplaceDirString() const { return RawReplaceDirectories; }
    const std::string& getBaseDirString() const { return RawBaseDirectories; }
    const std::string& getEndIf() const { return RawEndIf; }
  private:
    std::string RawName;
    std::string RawBaseDirectories;
    std::string RawReplaceDirectories;
    std::string RawEndIf;
  };

public:
  HeaderGuardCheck(StringRef Name, ClangTidyContext *Context);

  bool shouldSuggestEndifComment(StringRef Filename) override { return Style.EndIf; }
  std::string getHeaderGuard(StringRef Filename, StringRef OldGuard) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  std::vector<std::pair<llvm::StringRef, llvm::StringRef>> RenameDirs;
  void loadOptions();
  void selectStyleByName(StringRef name);

  SelectedStyleEntry Style;
};

} // namespace misc
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_HEADERGUARDCHECK_H
