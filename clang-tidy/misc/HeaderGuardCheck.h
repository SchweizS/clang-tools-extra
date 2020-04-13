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
#include <array>
#include <utility>
#include <vector>
namespace clang {
namespace tidy {
namespace misc {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc-header-guard.html

#define CLANG_TIDY_MISC_HEADER_GUARD_CHECK_ENTRY                               \
  const char *_name() const override { return name; }                          \
  const char *const *_dirs() const override { return dirs.data(); }            \
  size_t _dirs_size() const override { return dirs.size(); }                \
  const std::array<const char *const, 2> * _repl() const override {       \
    return repl.data();                                                        \
  }                                                                            \
  size_t _repl_size() const override { return repl.size(); }

class HeaderGuardCheck : public utils::HeaderGuardCheck {

  struct base {
    using repl_t = std::array<const char *const, 2>;
    static constexpr auto name = "";
    static constexpr std::array<const char *, 0> dirs{};
    static constexpr std::array<repl_t, 0> repl{};
    virtual const char *_name() const = 0;
    virtual const char *const *_dirs() const = 0;
    virtual size_t _dirs_size() const = 0;
    virtual const repl_t *  _repl() const = 0;
    virtual size_t _repl_size() const = 0;
  };

  static constexpr struct llvm_t : base {
    CLANG_TIDY_MISC_HEADER_GUARD_CHECK_ENTRY

    static constexpr auto name = "llvm";
    static constexpr std::array<const char *, 2> dirs{"include/", "llvm/"};
    static constexpr std::array<repl_t, 2> repl{{{"tools/clang/", "tools/"},{"/llvm/", "h"}}};
  } _llvm;
  static constexpr struct def_t : base {
    CLANG_TIDY_MISC_HEADER_GUARD_CHECK_ENTRY

    static constexpr auto name = "default";
    static constexpr std::array<const char *, 2> dirs{"src", "include"};
    static constexpr std::array<repl_t, 0> repl{};
  } _def;

  static constexpr const base *Styles[] = {&_def, &_llvm};
  static const base *getStyleByName(const std::string &name);

  std::vector<llvm::StringRef> BaseDirs;
  const base *Style;
  std::string _BaseDirs;
  std::string _RenameDirs;
  std::string _Style;

public:
  HeaderGuardCheck(StringRef Name, ClangTidyContext *Context);

  bool shouldSuggestEndifComment(StringRef Filename) override { return false; }
  std::string getHeaderGuard(StringRef Filename, StringRef OldGuard) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  std::vector<std::pair<llvm::StringRef, llvm::StringRef>> RenameDirs;
  void loadOptions();
};

} // namespace misc
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_HEADERGUARDCHECK_H
