# Building CppKeyboardColour on Windows 11

This is a **Visual Studio 2022 C++17 MSBuild project** (`.sln` + `.vcxproj`). It is **not** a CMake project and **not** a .NET project, so `cmake` and `dotnet build` are not used.

The instructions below assume the repository is at:

```text
D:\work\repo\CppKeyboardColour
```

Adjust the path if your checkout is somewhere else.

---

## Contents

- [Quick start](#quick-start)
- [Prerequisites](#prerequisites)
- [Install Visual Studio 2022](#install-visual-studio-2022)
- [Check your toolset and SDK](#check-your-toolset-and-sdk)
- [Build in Visual Studio](#build-in-visual-studio)
- [Build from PowerShell / MSBuild](#build-from-powershell--msbuild)
- [Output layout](#output-layout)
- [Run and test](#run-and-test)
- [Build configurations](#build-configurations)
- [Known build issue: hard-coded MSVC library paths](#known-build-issue-hard-coded-msvc-library-paths)
- [Troubleshooting](#troubleshooting)
- [Fan feature notes](#fan-feature-notes)
- [Clean rebuild](#clean-rebuild)
- [Appendix: useful commands](#appendix-useful-commands)

---

## Quick start

1. Install **Visual Studio 2022** with the **Desktop development with C++** workload.
2. Install the **Windows 10 SDK 10.0.19041.0** (or change the project's SDK version, see below).
3. Open `CppKeyboardColour.sln`.
4. Select **Release** and **x64** in the toolbar.
5. **Build → Build Solution** (`Ctrl+Shift+B`).
6. From the repository root, copy `InsydeDCHU.dll` next to the built executable:
   ```bat
   copy /Y InsydeDCHU.dll Build\Bin\Release\x64\
   ```
7. Open an **elevated Command Prompt** and run:
   ```bat
   cd /d D:\work\repo\CppKeyboardColour\Build\Bin\Release\x64
   .\CppKeyboardColour.exe fan status
   ```

> **Important:** Use the **x64** configuration. The included `InsydeDCHU.dll` is x64, and the `Win32`/`x86` configurations are not suitable for it.

> **If the build fails with `LNK1104`** for a path under `...\VC\Tools\MSVC\14.44.35207\...`, apply the portable library fix in **Known build issue: hard-coded MSVC library paths** below.

---

## Prerequisites

### Required

- **Windows 11** (Windows 10 also works)
- **Visual Studio 2022** (17.11 or newer recommended; the solution was created with VS 17.11)
- **Desktop development with C++** workload
- **MSVC v143** C++ build tools
- **Windows 10 SDK 10.0.19041.0** (or a newer installed SDK with `WindowsTargetPlatformVersion` updated)
- **Administrator privileges** to run the application
- **`InsydeDCHU.dll`** (already in the repository root) copied next to the executable

### Not required

- CMake
- .NET SDK
- vcpkg
- NuGet packages
- Third-party libraries

The project uses only the Windows SDK, the MSVC CRT, and the Windows COM/WMI libraries. `wbemuuid.lib` and `advapi32.lib` are linked automatically via `#pragma comment(lib, ...)` in `stdafx.h`.

---

## Install Visual Studio 2022

1. Download **Visual Studio 2022 Community** (or any edition) from Microsoft.
2. In the Visual Studio Installer, select the workload:
   - **Desktop development with C++**
3. In **Individual components**, make sure these are installed:
   - **MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)**
   - **Windows 10 SDK (10.0.19041.0)**
   - **C++ core features**
4. Install/update.

If you use **Build Tools for Visual Studio 2022** instead of the IDE, install the same workload/components; `MSBuild.exe` is enough to build.

---

## Check your toolset and SDK

Open **PowerShell** and run:

```powershell
# Where is Visual Studio installed?
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * `
  -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
  -property installationPath
```

List installed MSVC toolsets:

```powershell
Get-ChildItem "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC" |
  Select-Object -ExpandProperty Name
```

List installed Windows SDK versions:

```powershell
Get-ChildItem "C:\Program Files (x86)\Windows Kits\10\Include" |
  Select-Object -ExpandProperty Name
```

The project currently requests:

- `PlatformToolset` = `v143`
- `WindowsTargetPlatformVersion` = `10.0.19041.0`

If your installed SDK is different, either install `10.0.19041.0` or change the project to use your installed version (see **Troubleshooting**).

---

## Build in Visual Studio

1. Open `CppKeyboardColour.sln` from the repository root.
2. In the toolbar, select:
   - **Configuration:** `Release`
   - **Platform:** `x64`
3. Select **Build → Build Solution** (`Ctrl+Shift+B`).
4. The output appears in:
   ```text
   Build\Bin\Release\x64\CppKeyboardColour.exe
   ```
5. From the repository root, copy the DLL:
   ```powershell
   Copy-Item .\InsydeDCHU.dll .\Build\Bin\Release\x64\ -Force
   ```

### Debug build

If you want to debug:

1. Select **Debug** and **x64**.
2. Build.
3. Output:
   ```text
   Build\Bin\Debug\x64\CppKeyboardColour.exe
   ```
4. Copy `InsydeDCHU.dll` into `Build\Bin\Debug\x64\` as well.

> **Note:** The `Debug` configuration sets `DEBUG_BUILD`, which makes the keyboard path use the **debug/fake host** (`USE_DEBUGGABLE_HOST=true`). It is intended for UI/animation debugging, not for real keyboard hardware testing. Use **Release** or **TestBuild** for real hardware. Fan commands bypass the host and still use the real DCHU DLL, but Release is still recommended.

> **Note:** The app's manifest requests administrator privileges (`UACExecutionLevel=RequireAdministrator`). When debugging, run Visual Studio as Administrator, otherwise the debugger may fail to start the app or the app may not be able to access the DCHU driver.

> **Debug arguments:** `CppKeyboardColour.vcxproj.user` currently sets `LocalDebuggerCommandArguments` to `theme awesome --speed 1000`. Change it to `fan status` if you want to debug the fan command, or run the executable from an elevated terminal. This `.user` file is user-specific; changes affect only your local debugging setup.

### TestBuild

`TestBuild | x64` uses the real hardware host and forces Device Monitor Mode (`--dmm`) because `ShouldEnableDMM()` returns `true` under `TEST_BUILD`. Use it when you need verbose DCHU/WMI output. In a `Release` build, add `--debug` to any command to enable the same detailed logging. Output:

```text
Build\Bin\TestBuild\x64\CppKeyboardColour.exe
```

---

## Build from PowerShell / MSBuild

### Developer PowerShell for VS 2022

```powershell
Set-Location D:\work\repo\CppKeyboardColour
msbuild .\CppKeyboardColour.sln /m /p:Configuration=Release /p:Platform=x64
```

### x64 Native Tools Command Prompt for VS 2022

```bat
cd /d D:\work\repo\CppKeyboardColour
msbuild .\CppKeyboardColour.sln /m /p:Configuration=Release /p:Platform=x64
```

If `msbuild` is not on `PATH`, locate it with `vswhere` in PowerShell:

```powershell
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * `
  -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
  -property installationPath

$msbuild = Join-Path $vs "MSBuild\Current\Bin\MSBuild.exe"

& $msbuild .\CppKeyboardColour.sln /m /p:Configuration=Release /p:Platform=x64
```

You can also build the project file directly:

```powershell
& $msbuild .\CppKeyboardColour\CppKeyboardColour.vcxproj /m /p:Configuration=Release /p:Platform=x64
```

Then copy the DLL:

```powershell
Copy-Item .\InsydeDCHU.dll .\Build\Bin\Release\x64\ -Force
```

---

## Output layout

The project builds a console application (`SubSystem=Console`), so a console window opens when you run it.

The project uses these output paths:

```text
Build\Bin\<Configuration>\<Platform>\CppKeyboardColour.exe
Build\Inter\<Configuration>\<Platform>\...
```

For the recommended configuration:

```text
Build\Bin\Release\x64\CppKeyboardColour.exe
```

The README uses the name `CLEVO_KeyboardColour.exe`. The project does not define a custom `TargetName`, so the build produces `CppKeyboardColour.exe`. You can either:

- use `CppKeyboardColour.exe` in commands, or
- rename it:
  ```powershell
  Rename-Item .\Build\Bin\Release\x64\CppKeyboardColour.exe CLEVO_KeyboardColour.exe
  ```

If you rename it, use `CLEVO_KeyboardColour.exe` in the README commands.

---

## Run and test

The app requires Administrator privileges. Open an **elevated** PowerShell:

```powershell
Set-Location D:\work\repo\CppKeyboardColour\Build\Bin\Release\x64
.\CppKeyboardColour.exe fan status
```

Expected fan test sequence:

```bat
.\CppKeyboardColour.exe fan status
.\CppKeyboardColour.exe fan mode quiet
.\CppKeyboardColour.exe fan status
.\CppKeyboardColour.exe fan mode max
.\CppKeyboardColour.exe fan status
.\CppKeyboardColour.exe fan reset
```

Keyboard smoke test (Release or TestBuild, real hardware):

```bat
.\CppKeyboardColour.exe colour 0xff0000
.\CppKeyboardColour.exe backlight off
```

> **Console output note:** The logger uses `WriteConsoleW`, so redirecting output to a file or pipe (for example `> fan.txt` or `| Tee-Object`) will **not** capture the messages. Run the app in a real console window to see the output.

> **Single-instance note:** The app allows only one instance. If a background keyboard animation is running, close it before running fan commands, otherwise you will see “You can only have 1 instance of this program running.”

---

## Build configurations

| Configuration | Platform | Real keyboard hardware? | Device Monitor Mode | Recommended use |
|---|---|---|---|---|
| `Release` | `x64` | Yes | Only with `--dmm` | Normal use and real hardware testing |
| `Debug` | `x64` | No (uses debug host for keyboard) | Only with `--dmm` | UI/animation debugging |
| `TestBuild` | `x64` | Yes | Forced on | DCHU/WMI protocol debugging |
| `Debug` / `Release` / `TestBuild` | `Win32` (`x86`) | Not recommended | — | Not suitable: the DLL is x64, and the Win32 configurations are missing the `*_BUILD` defines |
| `ARM64` | maps to `x64` | Yes (builds x64) | Same as x64 | Use `x64` directly |

Use **`Release | x64`** unless you have a specific reason to use another configuration.

Add `--debug` to any command to show `[DEBUG]` log messages without Device Monitor Mode.

---

## Known build issue: hard-coded MSVC library paths

Some older copies of this project contain absolute paths to the original author's Visual Studio installation, for example:

```xml
<AdditionalDependencies>C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\onecore\$(Platform)\msvcprtd.lib;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\$(Platform)\comsuppwd.lib</AdditionalDependencies>
```

If your Visual Studio edition, installation path, or MSVC version differs, the linker will fail with an error such as:

```text
LNK1104: cannot open file 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\lib\onecore\x64\msvcprtd.lib'
```

This repository's `CppKeyboardColour.vcxproj` has been updated to use portable library names, so a fresh checkout should not hit this issue. If you are building an older copy, replace the six `<AdditionalDependencies>` lines as follows.

For the two **Debug** configurations (`Debug|Win32` and `Debug|x64`):

```xml
<AdditionalDependencies>msvcprtd.lib;comsuppwd.lib;%(AdditionalDependencies)</AdditionalDependencies>
```

For the **Release** and **TestBuild** configurations (`Win32` and `x64`):

```xml
<AdditionalDependencies>msvcprt.lib;comsuppw.lib;%(AdditionalDependencies)</AdditionalDependencies>
```

The linker will find these libraries in the standard VC library directories for your installed toolset.

If you specifically need the OneCore CRT variant, use the MSBuild property instead of a hard-coded path:

```xml
<AdditionalDependencies>$(VCToolsInstallDir)lib\onecore\$(Platform)\msvcprtd.lib;$(VCToolsInstallDir)lib\$(Platform)\comsuppwd.lib;%(AdditionalDependencies)</AdditionalDependencies>
```

After editing, reopen the solution and do **Build → Rebuild Solution**.

Alternatively, edit it in Visual Studio:

1. Right-click the project → **Properties**.
2. Select the configuration/platform you want to fix.
3. **Linker → Input → Additional Dependencies**.
4. Replace the absolute paths with `msvcprtd.lib;comsuppwd.lib` (Debug) or `msvcprt.lib;comsuppw.lib` (Release/TestBuild).
5. Repeat for the other configurations.

---

## Troubleshooting

### `MSB8020: The build tools for v143 cannot be found`

Install the **MSVC v143 - VS 2022 C++ x64/x86 build tools** component in the Visual Studio Installer.

### `MSB8036: The Windows SDK version 10.0.19041.0 was not found`

Either:

- Install **Windows 10 SDK (10.0.19041.0)** in the Visual Studio Installer, or
- Change `WindowsTargetPlatformVersion` in `CppKeyboardColour.vcxproj` to an installed version, for example:
  ```xml
  <WindowsTargetPlatformVersion>10.0.22621.0</WindowsTargetPlatformVersion>
  ```
  or use `10.0` to select the latest installed SDK:
  ```xml
  <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
  ```

### `RC2135: file not found: C:\Users\joshs\Pictures\keyboard.ico`

The original project referenced an icon in the author's local Pictures folder. That reference has been removed from this repository, so a fresh checkout builds with the default application icon.

If you are building an older copy, either:

- Remove the `IDI_ICON1 ICON "C:\\Users\\joshs\\Pictures\\keyboard.ico"` line from `CppKeyboardColour.rc`, and remove the `<Image Include="..\..\..\..\Users\joshs\Pictures\keyboard.ico" />` items from `CppKeyboardColour.vcxproj` and `CppKeyboardColour.vcxproj.filters`, or
- Put your own `.ico` file in the project and update the `.rc`/`.vcxproj` paths.

If Visual Studio's resource editor still shows the old icon after opening the project, delete `CppKeyboardColour.aps` and reopen the `.rc` file; it is a regenerated cache, not a build input.

If Visual Studio still shows the missing image item in Solution Explorer, close Visual Studio, delete the `.vs` folder, and reopen the solution.

### `LNK1104: cannot open file '...msvcprtd.lib'` or `...comsuppwd.lib`

Apply the **Known build issue: hard-coded MSVC library paths** fix above.

### `LNK2019: unresolved external symbol` related to COM

Make sure `comsuppw.lib` (Release) or `comsuppwd.lib` (Debug) is in **Linker → Input → Additional Dependencies**.

### The app exits with `-1073740791` / `0xC0000409`

`0xC0000409` is `STATUS_STACK_BUFFER_OVERRUN`. In this project it was caused by `GetDCHU_Data_Buffer`: the DLL writes the package data plus one extra byte, so the original 256-byte stack buffer was one byte too small. The DCHU buffer has been enlarged to `0x800` bytes. Rebuild to pick up the fix.

### The app prints only the title and no command output

This was a console logger visibility bug under Windows Terminal/PowerShell. The logger used `GetConsoleWindow()`/`GWL_STYLE` to decide whether to write, which can report a non-visible window even though the console is usable. This has been fixed: logging no longer depends on the window visibility. Rebuild to pick up the change.

### `Cannot load InsydeDCHU.dll`

- Copy `InsydeDCHU.dll` into the same directory as the executable.
- Build **x64** and use the **x64** DLL.
- Do not use the `Win32`/`x86` build with the x64 DLL.

### `Failed to read fan status` / `Failed to persist the fan mode`

- Run as Administrator.
- Make sure the Insyde DCHU driver/ACPI interface is available. It is usually installed by the OEM Control Center.
- Make sure `InsydeDCHU.dll` is next to the executable and matches the architecture.
- Run `fan status` to see which support flags the firmware reports.

### `You can only have 1 instance of this program running`

Close the other instance, including a background keyboard animation started with `--hide`. Use Task Manager if needed.

### `Cannot load clevomof.dll`

This is for 3-zone keyboard systems. Run `Scripts\RegisterClevoMof.cmd` as Administrator, or install the OEM Control Center. For 1-zone systems and fan control, `InsydeDCHU.dll` is the relevant DLL.

### The app is flagged by antivirus

Some antivirus products flag hardware-control tools. Add the executable to your antivirus exclusion list if you trust the build.

### The app opens and closes immediately

- Run it from an existing elevated Command Prompt/PowerShell window so the console stays open.
- Check for a fatal error message; the app waits for Enter on fatal errors.
- Make sure you copied the DLL next to the executable.

---

## Warning cleanup

The source has been adjusted to avoid the common MSVC v143 warnings seen in older builds:

- `C4229` — calling-convention anachronism on x64
- `C4244` — float-to-integer narrowing in the animation code
- `C4267` — `size_t` to `uint32_t` narrowing
- `C4603` — precompiled-header macro mismatch in `AwesomeAnimation.cpp`

A clean rebuild should no longer emit those 42 warnings.

---

## Fan feature notes

- Fan control is experimental and model-dependent. Use `fan status` first, avoid quiet profiles under sustained heavy load, and use `fan reset` to recover.
- Fan control uses the same **`InsydeDCHU.dll`** as single-zone keyboard control.
- Fan control requires a firmware that exposes the Insyde DCHU fan interface. Systems that only expose a WMI fan interface may not support these commands.
- The DLL must be in the same directory as the executable and must match the executable architecture.
- Fan commands are handled before keyboard model detection, so they can work even if the keyboard model table does not list your machine.
- `fan status` is read-only and is the best first test.
- `fan reset` restores the firmware fan table and switches back to `auto`.
- `fan mode custom` uses the custom curve currently stored by the OEM application/Control Center. Custom-curve editing is not exposed by this build.
- `fan dust` may make the fans run at full speed for a while.
- `fan offset` accepts `0`–`100`.
- Fan commands are one-shot operations; `--hide` and `--dmm` are not applied to them.
- The OEM Control Center may override fan settings while it is running. Close it if the mode does not stick.

---

## Clean rebuild

From Visual Studio:

- **Build → Clean Solution**
- **Build → Rebuild Solution**

From PowerShell:

```powershell
Set-Location D:\work\repo\CppKeyboardColour

$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * `
  -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
  -property installationPath
$msbuild = Join-Path $vs "MSBuild\Current\Bin\MSBuild.exe"

Remove-Item .\Build -Recurse -Force -ErrorAction SilentlyContinue
& $msbuild .\CppKeyboardColour.sln /m /p:Configuration=Release /p:Platform=x64
```

Remember to copy `InsydeDCHU.dll` again after a clean build.

---

## Appendix: useful commands

```powershell
# Find MSBuild
$vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
  -latest -products * `
  -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
  -property installationPath
$msbuild = Join-Path $vs "MSBuild\Current\Bin\MSBuild.exe"

# Build Release x64
& $msbuild .\CppKeyboardColour.sln /m /p:Configuration=Release /p:Platform=x64

# Copy the DLL
Copy-Item .\InsydeDCHU.dll .\Build\Bin\Release\x64\ -Force

# Start an elevated PowerShell for testing
Start-Process powershell -Verb RunAs
```

Inside the elevated PowerShell:

```powershell
Set-Location D:\work\repo\CppKeyboardColour\Build\Bin\Release\x64
.\CppKeyboardColour.exe fan status
```
