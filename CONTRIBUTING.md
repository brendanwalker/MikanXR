# Contributing to MikanXR

## Code formatting

C++ source under `src/` is formatted with [clang-format](https://clang.llvm.org/docs/ClangFormat.html).
The style is defined by [`.clang-format`](.clang-format) at the repo root and is enforced
by CI on every push and pull request.

### Tool version

Use **clang-format 19.1.x** to match what CI uses. clang-format output changes between
major versions, so a different version can reformat files in ways CI then rejects.

- **Visual Studio 2022** already bundles a compatible copy at
  `VC\Tools\Llvm\bin\clang-format.exe`.
- Otherwise install the pinned wheel: `pip install clang-format==19.1.5`
  (or `pipx install clang-format==19.1.5`).

Make sure the chosen `clang-format` is on your `PATH`.

### Fixing formatting

After configuring the project, reformat all sources in place with:

```sh
cmake --build build --target format
```

To check formatting without modifying anything (this is what CI runs):

```sh
cmake --build build --target format-check
```

Both targets just wrap [`cmake/RunClangFormat.cmake`](cmake/RunClangFormat.cmake), which you
can also run directly without a configured build tree:

```sh
cmake -P cmake/RunClangFormat.cmake -- --fix     # reformat in place
cmake -P cmake/RunClangFormat.cmake -- --check   # verify only
```

Only files under `src/` are formatted; `thirdparty/` is left untouched.

### git blame

The one-time repository-wide reformat commit is listed in
[`.git-blame-ignore-revs`](.git-blame-ignore-revs) so it doesn't pollute `git blame`.
GitHub honors this automatically. To benefit locally, run once:

```sh
git config blame.ignoreRevsFile .git-blame-ignore-revs
```
