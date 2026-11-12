# AGENTS.md

Instructions for AI coding assistants that prepare changes for M.A.X.R. Everything below is taken from the code,
the CI workflows, the reviews and the merged pull requests of this repository; the links point to the sources.

## Layout

- C++20. `src/lib`: the game library `maxr_lib` (model, actions, network); `src/ui`: the game `maxr`;
  `src/dedicatedserver`; `resinstaller`; `tests`.
- Third-party code, leave it unchanged: `submodules/` (doctest, nlohmann/json, spiritless_po) and `src/3rd/`.
- Pull requests target `develop`.
- Text shown to the player goes through `lngPack.i18n ("Key~Name")`; the translations are gettext files in
  `data/languages/` (see `data/languages/readme.md`).

## Build

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DCMAKE_CXX_FLAGS="-Wall -Wextra"
cmake --build build --target maxr maxr_dedicatedserver maxr_tests resinstaller
ctest --test-dir build --output-on-failure
```

- The packages for Linux, macOS and MSYS2 UCRT64 are listed in `.github/workflows/ubuntu.yml`, `macos.yml` and
  `cmake-msys2-windows.yml` (MSYS2 builds with `-G Ninja`).
- CMake collects the sources with `file(GLOB_RECURSE ...)`: run cmake again after adding or removing a file.
- premake5, as in `premake5-ubuntu.yml` and `premake5-windows.yml` (`--crashRpt_root` is optional):

```sh
premake5 gmake --to=solution/gmake
cd solution/gmake && make config=release -j4 maxr dedicated_server resinstaller tests
bin/Release/maxr_tests
```

```bat
premake5 vs2022 --to=solution/vs2022
cd solution/vs2022
nuget install packages.config -OutputDirectory packages
msbuild /property:Configuration=Release Maxr.sln
bin\Release\maxr_tests.exe
```

## CI

| Workflow | Compiler | Build | Warning flags |
|---|---|---|---|
| `ubuntu.yml` | GCC | CMake, make | none |
| `macos.yml` | AppleClang (arm64) | CMake, make | none |
| `cmake-msys2-windows.yml` | GCC (MSYS2 UCRT64) | CMake, Ninja, Debug | none |
| `premake5-ubuntu.yml` | GCC | premake5 gmake, Release | `-Wall -Wextra -Werror` |
| `premake5-windows.yml` | MSVC, 32-bit (Win32) | premake5 vs2022, Release | `/W4 /WX` |
| `check-format.yml` | | clang-format 18 | |

The premake builds treat warnings as errors in `maxr_lib`, `maxr`, `dedicated_server` and `tests`, so a change that
passes the CMake jobs can still fail there. Build with `-Wall -Wextra` (above) and add no warnings. Known traps:

- GCC: `-Wall` includes `-Wdangling-reference`. `const auto& vehicle = addVehicle (model, cPosition (1, 2));` is an
  error when `addVehicle()` is a free function (for example a test helper) that returns a reference and takes
  `const cPosition&`. Take such parameters by value.
- MSVC: a local variable that hides another local variable is C4456, an error with `/WX`; GCC does not warn here
  without `-Wshadow`. `premake5.lua` disables C4458 (hides a class member), and C4459 (hides a global) in `tests`.
- MSVC, 32-bit: `std::size_t` has 32 bits, so `std::size_t size = std::filesystem::file_size (path);` is C4244, an
  error. Check the range, then convert with `narrow_cast` (`utility/narrow_cast.h`).
- No CI job builds for Linux on ARM, where `char` is unsigned: a result (a checksum, the bytes of a file) must not
  depend on the sign of `char`.

## Formatting

`.clang-format` is in `src/`, `tests/` and `resinstaller/` (the same style); `src/3rd/` and `submodules/` are not
formatted. The CI checks the C and C++ files (except `mk/`) with clang-format 18: run `clang-format-18 -i` on the
changed files. The style in short: tabs, braces on their own lines, `f (x)` but `f()`, includes: the file's own
header first, then `"..."`, then `<...>`.

## Code

- Names: classes `cName`, structs `sName`, enums `eName`. Each file starts with the same GPL header as the others;
  header guards like `utility_narrowcastH` (no `#pragma once`); a `//---...` line precedes each function definition
  in a `.cpp`, as in the neighbouring code.
- No function that nothing calls: unused methods are removed with a tool
  ([#72](https://github.com/maxr-dot-org/maxr/pull/72#issuecomment-6656053004)). Add a function in the pull request
  that uses it.
- No duplicated logic: expose the existing check and use it in both places, for example in the GUI and in the action
  ([#78](https://github.com/maxr-dot-org/maxr/pull/78#discussion_r4491158739)).
- Put a new check into the existing block of the same condition, symmetric with the neighbouring case
  ([#77](https://github.com/maxr-dot-org/maxr/pull/77#discussion_r4492106409),
  [#62](https://github.com/maxr-dot-org/maxr/pull/62#discussion_r4395960760)).
- Bytes of files and network data: `unsigned char` or `std::byte`, not `char`
  ([#70](https://github.com/maxr-dot-org/maxr/pull/70#discussion_r4466658453)).
- Fields of a binary format: fixed-size integers such as `std::int16_t`
  ([#70](https://github.com/maxr-dot-org/maxr/pull/70#discussion_r4466681793)).
- No dependence on the byte order of the machine
  ([#65](https://github.com/maxr-dot-org/maxr/pull/65#discussion_r4388460299)).
- C++20 only: a C++23 algorithm can be implemented in `src/lib/utility/ranges.h`
  ([#93](https://github.com/maxr-dot-org/maxr/pull/93#discussion_r4490770439)).
- Standard algorithms such as `std::ranges::any_of` instead of hand-written loops
  ([#67](https://github.com/maxr-dot-org/maxr/pull/67#discussion_r4437862606)).
- `execute()` of an action (`src/lib/game/logic/action/`) runs on the server and on every client with data from the
  network: "Make every possible sanity check!" (`action.h`). Check that each unit exists and belongs to the sender
  (`playerNr`), check positions, distances, states and amounts, and otherwise return without a change: a modified
  client can send what the GUI never offers.

## Tests

- doctest; `tests/main.cpp` defines `main`. The tests of `src/lib/<dir>/` are in `tests/tests/<dir>/`, in the file
  of the same code if there is one ([#69](https://github.com/maxr-dot-org/maxr/pull/69#discussion_r4465417611)).
- Each `TEST_CASE` is a ctest test; run one with `maxr_tests -tc="<name>"`.
- A test of a fix fails without the fix. A change that must keep the values (a faster checksum) gets a test of the
  old values ([#65](https://github.com/maxr-dot-org/maxr/pull/65#discussion_r4388367167)).
- Helpers used by several test files go into a shared test utility file in `tests/`
  ([#62](https://github.com/maxr-dot-org/maxr/pull/62#discussion_r4492951342)).
- RAII for the clean-up: a temporary file is removed by a destructor
  ([#70](https://github.com/maxr-dot-org/maxr/pull/70#discussion_r4466688897)).

## Pull requests

- One fix per pull request, on the current `develop`, complete on its own: what it adds is used and tested in it.
- Commit messages and descriptions in English.
- The description of a fix has the sections of the merged ones (for example
  [#53](https://github.com/maxr-dot-org/maxr/pull/53)): `## Summary`, `## Steps to reproduce`, `## Expected`,
  `## Actual`, `## Root cause` (file and function), `## Proposed fix`, `## Testing` (compiler, build type, `ctest`
  result, the test that fails without the fix, what was not tested), `## Affected version` (the `develop` commit).
  Each heading on its own line, followed by an empty line: joined into one line, the whole text becomes the heading
  ([#92](https://github.com/maxr-dot-org/maxr/issues/92#issuecomment-6627653830)).
- All CI checks must pass. In a workflow, use existing actions rather than hand-written commands with retries
  ([#49](https://github.com/maxr-dot-org/maxr/pull/49#discussion_r4091270596)).
