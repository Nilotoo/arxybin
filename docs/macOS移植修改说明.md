# arxybin — macOS 移植修改说明

> 日期：2026-06-20
> 目的：让 arxybin 工程支持在 macOS 上编译（VST3 + AudioUnit + Standalone），同时在 Windows 上仍能正常编译。

---

## 修改总览

| # | 文件 | 改了什么 | 为什么必须改 |
|---|------|---------|-------------|
| 1 | `CMakeLists.txt` 第 11-13 行 | MSVC 运行时设置加了 `if(MSVC)` 判断 | macOS 用 Apple Clang 编译器，不认识 MSVC 的 `/MT` 选项，不加判断会报错 |
| 2 | `CMakeLists.txt` 第 17-32 行 | JUCE 路径从硬编码改为可配置 | macOS / CI 上没有 `C:/Users/Administrator/JUCE` 这个路径 |
| 3 | `CMakeLists.txt` 第 82 行 | `.rc` 资源文件加了仅 Windows 的条件 | `.rc` 是 Windows 专用格式，macOS 无法编译 |
| 4 | `CMakeLists.txt` 第 43-44 行 | 插件格式在 macOS 上额外输出 AudioUnit | 让 Logic Pro 等 DAW 能直接用 AU 格式加载插件 |
| 5 | `.github/workflows/build.yml` | **新文件** — GitHub Actions 自动编译配置 | 云端自动编译，每次推送代码自动产出 Win + Mac 插件 |
| 6 | `.gitignore` | **新文件** — Git 忽略规则 | 防止把几百 MB 的构建产物、安装包等上传到 GitHub |

### 源代码改动：无

`Source/` 下的所有 .h/.cpp 文件**零改动**。你的 C++ 代码本身是纯标准 C++，没有 Windows 专有代码，macOS 上直接就能编译。

---

## 逐条修改详情

---

### 修改 1：MSVC 运行时 → Windows 专属

**改之前（第 8-10 行）：**
```cmake
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
```

**改之后：**
```cmake
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
```

**怎么还原：** 删掉 `if(MSVC)` 和 `endif()`，把 `set(...)` 那行移回左边（去掉缩进）。

---

### 修改 2：JUCE 路径 → 可配置

**改之前（第 14-16 行）：**
```cmake
add_subdirectory(C:/Users/Administrator/JUCE
    ${CMAKE_BINARY_DIR}/JUCE)
```

**改之后：**
```cmake
if(NOT DEFINED JUCE_PATH)
    if(DEFINED ENV{JUCE_PATH})
        set(JUCE_PATH "$ENV{JUCE_PATH}")
    elseif(WIN32)
        set(JUCE_PATH "C:/Users/Administrator/JUCE")
    else()
        set(JUCE_PATH "$ENV{HOME}/JUCE")
    endif()
endif()
message(STATUS "JUCE_PATH = ${JUCE_PATH}")
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
```

**查找优先级：**
1. `cmake -DJUCE_PATH=xxx` 命令行指定的路径（最高优先级）
2. 系统环境变量 `JUCE_PATH`
3. 平台默认值（Windows → 你的原来路径，macOS → `~/JUCE`）

**怎么还原：** 删掉 `if(NOT DEFINED JUCE_PATH)...endif()` 整段和 `message(...)` 行，改回原来的两行 `add_subdirectory(...)`。

---

### 修改 3：.rc 资源文件 → Windows 专属

**改之前（原来第 66 行）：**
```cmake
        Source/arxybin.rc
```

**改之后：**
```cmake
        $<$<PLATFORM_ID:Windows>:Source/arxybin.rc>
```

**怎么还原：** 把 `$<$<PLATFORM_ID:Windows>:Source/arxybin.rc>` 改回 `Source/arxybin.rc`。

---

### 修改 4：macOS 加 AudioUnit 格式

**改之前（原来第 27-29 行）：**
```cmake
    # Plugin formats
    FORMATS                  VST3 Standalone
    VST3_CATEGORIES          ...
```

**改之后：**
```cmake
    # Plugin formats — macOS also builds AudioUnit (AU) for Logic Pro etc.
    FORMATS                  VST3 Standalone $<$<PLATFORM_ID:Apple>:AudioUnit>
    VST3_CATEGORIES          ...
```

`$<$<PLATFORM_ID:Apple>:AudioUnit>` 是一个 CMake 生成器表达式：只在 Apple 平台上展开为 `AudioUnit`，其他平台展开为空。Windows 上行为完全不变。

**怎么还原：** 把 `FORMATS` 行改回 `FORMATS                  VST3 Standalone`。

---

## 新增文件

### `.github/workflows/build.yml`

这是 GitHub Actions 的自动编译脚本。每次你推送代码到 GitHub：
- **macOS 机器** 自动编译 → 产出 `arxybin-macOS.zip`（VST3 + AU + Standalone）
- **Windows 机器** 自动编译 → 产出 `arxybin-Windows.zip`（VST3 + Standalone）

编译结果在 GitHub 网页的 **Actions** 标签页里下载。

### `.gitignore`

告诉 git 不要上传这些目录/文件：
- `build/` — 编译中间文件（几百 MB）
- `.claude/` — Claude Code 内部文件
- `release/*.exe`、`release/*.zip` — 安装包
- `.vscode/`、`.DS_Store` 等 — IDE/OS 杂项

---

## 你的下一步操作

### 完成后你会得到什么？

在 GitHub Actions 页面下载到的 `arxybin-macOS.zip` 解压后包含：

```
arxybin..vst3/          ← VST3 插件（Ableton / Cubase / Studio One 等）
arxybin..component/     ← AudioUnit 插件（Logic Pro 专用）
arxybin..app/           ← 独立运行程序
```

### 要实现云编译，你还需要：

1. **注册 GitHub 账号**（如果还没有）→ [github.com/signup](https://github.com/signup)
2. **创建一个新仓库**（比如叫 `arxybin`）→ 不要勾选 "Add a README"
3. **把代码推送到 GitHub** → 在终端里执行 GitHub 给你的那几行命令
4. **等 5-10 分钟** → 去仓库的 Actions 页面下载编译好的插件

---

> 💡 **一句话总结**：代码一行没改，只改了 CMakeLists.txt 让它在 macOS 上也能通过。GitHub 免费帮你编译 macOS 版本，你不需要买 Mac。
