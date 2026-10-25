# CLEVO Keyboard Colour Program

**A command-line RGB Keyboard backlight control program for CLEVO & Tongfang based laptops.**

This is a cleaner, more organized, and efficient version of the initial program. It is designed to provide enhanced features for customizing CLEVO & Tongfang based laptop keyboards with **RGB** lighting, supporting devices with **1-zone**, **3-zone** keyboards and devices with **RGB Lightbars**. The program also includes a variety of animated effects to enhance your keyboard’s visual experience.

---

## 🎨 Current Features

- **Support for CLEVO & Tongfang based systems with the following RGB components:**
  - **RGB 3-zone keyboards**.
  - **RGB 1-zone keyboards**.
  - **RGB Lightbars**.
- **Fan Control (Insyde DCHU systems):**
  - 🌡️ **Fan Status:** Read fan count, mode, offset, RPM, duty cycle and temperature.
  - 🎛️ **Fan Modes:** Switch between auto, quiet, max, Max-Q and custom profiles.
  - 📈 **Custom Curves:** Read and set the 4-point CPU/GPU fan curve, with `--dry-run` preview.
  - 🌀 **Maintenance:** Adjust the automatic fan offset, run anti-dust and restore firmware defaults.
- **Predefined Animated Effects:**
  - 🌈 **Fading Colors:** Smooth color transitions (predefined themes).
  - 🎥 **Optimized Animations:** Enhanced visual effects using animation objects.
  - ⏱️ **Custom Speed:** Change the speed to make an animation faster or slower.
  
⚠️ **Important Note:** This application requests for **Administrator privileges**. Please ensure you accept the Windows UAC prompt, as Windows Management Instrumentation (WMI) requires this to function correctly.

⚠️ **Warning:** Certain themes may utilize **~15% of CPU** at certain times.

---

## ❓FAQ - Frequently asked questions:

**How do I request for support?** - *Please create a request for it [here](https://github.com/DeviceIoControl/CppKeyboardColour/issues/new/choose).*

**How do I report a software bug?** - *Please create a bug report for it [here](https://github.com/DeviceIoControl/CppKeyboardColour/issues/new/choose).*

**Is my laptop supported?** - *This program is built for laptops based off of the CLEVO / Tongfang ODM design - For a list of supported CLEVO Models, please take a look at the [releases](https://github.com/DeviceIoControl/CppKeyboardColour/releases/latest).*

**How do I get more themes?** - *Please create an issue and request for it [here](https://github.com/DeviceIoControl/CppKeyboardColour/issues/new/choose).*

**Does this program have a GUI?** - *No. The intention of this program is to be a lightweight app to enable keyboard themes on your system without the need for the bloatware included with the Control Center. This program does not intend to be a complete replacement of the software shipped with your system.*

**How do I hide the command window?** - *Please refer to the 'Background Modes' section of the README for more information.*

**How long does it take to add support for my system?** - *Depends on your device. In most cases (If GitHub notifications worked properly), I can add support for certain models of laptop and create a new release within an hour. On certain occassions (depending on the device) it can take 24-hours or more, but I will ensure you are informed about this when that is the case.*

**How do I fix this error?** - *Please refer to the 'Common Errors' section of the README for more information.*

**NOTE:** Please bear in mind, I am the sole developer for this project, and although there may be a few minor code contributions on occassion, the majority of the time, it's *only me* adding support and servicing requests for this project. **So, please understand that some requests may take longer than expected, if I am not available or busy. Thank you for your understanding!**

---

## 🔧 Functional Commands

The following commands are supported and can be executed via **Command Prompt or PowerShell**.

### 🖥️ General Syntax:
Extract the program zip and run the commands **as an administrator**. To execute the `.exe` files, prepend the `.\` before the command.

```bash
.\CLEVO_KeyboardColour.exe theme [theme] --speed [speed] [--dmm] [--hide] [--debug]
.\CLEVO_KeyboardColour.exe inbuilt [theme] [--dmm] [--hide] [--debug] (3-Zone keyboards only)
.\CLEVO_KeyboardColour.exe backlight [on/off] [--dmm] [--hide] [--debug]
.\CLEVO_KeyboardColour.exe colour [hexColour] [--dmm] [--hide] [--debug]
.\CLEVO_KeyboardColour.exe colours [hexColour] [hexColour] [hexColour] [--dmm] [--hide] [--debug] (3-Zone keyboards only)
.\CLEVO_KeyboardColour.exe lightbar [hexColour] [--dmm] [--hide] [--debug]
.\CLEVO_KeyboardColour.exe fan status
.\CLEVO_KeyboardColour.exe fan mode [auto/quiet/max/maxq/custom]
.\CLEVO_KeyboardColour.exe fan offset [0-100]
.\CLEVO_KeyboardColour.exe fan dust
.\CLEVO_KeyboardColour.exe fan reset
.\CLEVO_KeyboardColour.exe fan curve show
.\CLEVO_KeyboardColour.exe fan curve set [cpu/gpu1/gpu2/all] [t2] [d2] [t3] [d3] [--dry-run]
.\CLEVO_KeyboardColour.exe fan help
.\CLEVO_KeyboardColour.exe fan curve help
.\CLEVO_KeyboardColour.exe help
```
---

### ⌨️ Backlight Operations: 

- **Turn Backlight Off:**  
  ```bash
  .\CLEVO_KeyboardColour.exe backlight off
  ```
  🖤 Turns off the keyboard backlight.

- **Default Backlight:**
  ```bash
  .\CLEVO_KeyboardColour.exe backlight on
  ```
  💙 Enables the default keyboard backlight colour (Solid Blue).

---

### ⌨️ Custom Colour Operations:

- **Set backlight colour to Red:**  
  ```bash
  .\CLEVO_KeyboardColour.exe colour 0xff0000
  ```
  ❤️ Turns the keyboard backlight Red.

- **Set backlight colour to Green:**
  ```bash
  .\CLEVO_KeyboardColour.exe colour 0x00ff00
  ```
  💚 Turns the keyboard backlight Green.

- **Set backlight colour to Blue:**  
  ```bash
  .\CLEVO_KeyboardColour.exe colour 0x0000ff
  ```
  💙 Turns the keyboard backlight Blue.

- **Set backlight colour to custom "Sakura Pink":**  
  ```bash
  .\CLEVO_KeyboardColour.exe colour 0xff6ea8
  ```
  🩷 Turns the keyboard backlight "Sakura Pink".

- **Set RGB colour to individual zones (3-Zone keyboards ONLY):**
  ```bash
  .\CLEVO_KeyboardColour.exe colours 0xff0000 0x00ff00 0x0000ff
  ```
  🌈 Sets the Left zone to Red, the middle zone to Green and right zone to Blue on the keyboard (3-Zone keyboards ONLY)
---

### 🚥 Custom Lightbar Colour Operations:

- **Set lightbar colour to Red:**  
  ```bash
  .\CLEVO_KeyboardColour.exe lightbar 0xff0000
  ```
  ❤️ Turns the lightbar Red.

- **Set lightbar colour to Green:**
  ```bash
  .\CLEVO_KeyboardColour.exe lightbar 0x00ff00
  ```
  💚 Turns the lightbar Green.

- **Set lightbar colour to Blue:**  
  ```bash
  .\CLEVO_KeyboardColour.exe lightbar 0x0000ff
  ```
  💙 Turns the lightbar Blue.

This command works in the same way as the `colour` command - For more info, please read the 'Custom Colour Operations' section.

---

### 🌀 Fan Control (InsydeDCHU.dll):

Fan control is available on systems whose BIOS exposes the Insyde DCHU fan interface. It uses the same **InsydeDCHU.dll** as the single-zone keyboard control, so the DLL must be in the same directory as `CLEVO_KeyboardColour.exe`. The DLL and executable architectures must also match (use the x64 build with the x64 DLL).

⚠️ **Warning:** Fan control is model-dependent and experimental. Use `fan status` first to check which features your firmware reports, and use `fan reset` to return to the firmware defaults. Do not use a quiet profile under sustained heavy load; the firmware's thermal protection should still ramp the fans, but use this feature at your own risk.

- **Read fan status (read-only):**
  ```bash
  .\CLEVO_KeyboardColour.exe fan status
  ```
  Displays the fan count, current fan mode, fan offset, CPU/GPU RPM, duty cycle, temperature and the support flags reported by the firmware.

- **Set the fan mode:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan mode auto
  .\CLEVO_KeyboardColour.exe fan mode quiet
  .\CLEVO_KeyboardColour.exe fan mode max
  .\CLEVO_KeyboardColour.exe fan mode maxq
  .\CLEVO_KeyboardColour.exe fan mode custom
  ```
  🌬️ `auto` uses the firmware's automatic fan curve. `quiet` requests the quiet/silent profile. `max` runs the fans at full speed. `maxq` selects the Max-Q profile (if supported). `custom` uses the custom fan curve currently stored in the firmware/OEM application.

- **Adjust the automatic fan offset:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan offset 20
  ```
  Applies a 0-100 fan offset to the automatic fan curve. Higher values make the fans ramp up sooner; `0` leaves the curve unchanged.

- **Run the anti-dust cycle (if supported):**
  ```bash
  .\CLEVO_KeyboardColour.exe fan dust
  ```
  🌀 Starts the firmware anti-dust routine. The fans may run at full speed for a short period.

- **Restore the firmware fan defaults:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan reset
  ```
  Restores the default fan table and switches back to `auto` mode. This is the only reset command; there is no separate `fan curve reset`.

- **Read the current custom curve:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan curve show
  ```
  Shows the current mode, offset and the 4-point curve for each fan present on the system.

- **Set a custom curve:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan curve set cpu 55 50 75 80
  .\CLEVO_KeyboardColour.exe fan curve set gpu1 60 45 80 85
  .\CLEVO_KeyboardColour.exe fan curve set all 55 50 75 80
  ```
  The curve has four points:

  ```text
  T1/D1 (fixed) -> T2/D2 (adjustable) -> T3/D3 (adjustable) -> T4/D4 (fixed)
  ```

  - `T` = temperature in °C.
  - `D` = fan duty/speed in %.
  - `T1/D1` is the fixed start point defined by the firmware/model (for example `40 °C / 35%`).
  - `T2/D2` and `T3/D3` are the adjustable points.
  - `T4/D4` is the fixed end point, normally `100 °C / 100%`.

  Example: `fan curve set cpu 55 50 75 80` means:

  ```text
  40 °C / 35%   (fixed start)
  55 °C / 50%   (T2/D2)
  75 °C / 80%   (T3/D3)
  100 °C / 100% (fixed end)
  ```

  The actual fixed `T1/D1` values are read from the current table and can differ by model; use `fan curve show` to see them. Setting a curve writes the EC fan table, switches to `custom` mode and persists the OEM page. Use `fan mode auto` to return to the firmware curve, or `fan reset` to restore the firmware default table and `auto` mode.

- **Preview a curve without writing:**
  ```bash
  .\CLEVO_KeyboardColour.exe fan curve set cpu 55 50 75 80 --dry-run
  ```
  Prints the current and proposed curves, the fan-table bytes and the page-4 changes without writing anything.

- **Help:**
  ```bash
  .\CLEVO_KeyboardColour.exe help
  .\CLEVO_KeyboardColour.exe /?
  .\CLEVO_KeyboardColour.exe fan help
  .\CLEVO_KeyboardColour.exe fan /?
  .\CLEVO_KeyboardColour.exe fan curve help
  .\CLEVO_KeyboardColour.exe fan curve /?
  ```
  `help`/`/?` shows general help; `fan help`/`fan /?` shows fan help only; `fan curve help`/`fan curve /?` shows curve help only. Help still runs under the application's administrator manifest for now.

---

### 🎨 Supported Animations:

- **Breathing Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme breathe
  ```
  🌬️ Enables a breathing effect (smooth inhale/exhale transition pattern between colors).

- **Colourshift:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme colourshift
  ```
  🔄 Alternates colors for **RGB** keyboards (designed for 3-zone keyboards only!).

- **Fresh Breathe:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme freshbreathe
  ```
  🌀 A custom theme designed by **@mehrshad315**, featuring an additional blue tone.

- **Colour Transform (Rainbow Sweep):**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme colourtransform
  ```
  🌈 Creates a rainbow sweep effect with smooth transitions.

- **Blink (Pulsating Blink):**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme blink
  ```
  ✨ Makes the keyboard lights blink in a pulsating pattern.

- **Heartbeat (Double Pulsating Blink):**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme heartbeat
  ```
  💖 Makes the keyboard lights pulse red in a heartbeat pattern.

- **Sakura Transform (Pink & Violet colour sweep):**
  ```bash
  .\CLEVO_KeyboardColour.exe theme sakuratransform
  ```
  🩷 💙 Makes the keyboard lights sweep through various shades of Pink & Violet.

- **Sakura Sunset (Pink colour sweep):**
  ```bash
  .\CLEVO_KeyboardColour.exe theme sakurasunset
  ```
  🩷 Makes the keyboard lights transition through Sakura Pink & Sunset Pink colours.

- **Sunset Breathe:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme sunsetbreathe
  ```
  🌅 Enables a Sunset Pink breathing effect (smooth inhale/exhale transition pattern).

- **Pink Breathe:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme pinkbreathe
  ```
  🫁 Enables a Pink breathing effect (smooth inhale/exhale transition pattern).

- **Colour Wave:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme colourwave
  ```
  🏄 Enables an RGB colour wave effect over the 3 keyboard zones (designed for 3-zone keyboards only!).

- **Awesome:**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme awesome
  ```
  😎 Cycles through all of the supported animations for the device.

---

### ⚙️ Inbuilt Themes (3-Zone Keyboards ONLY):

- **Breathing Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_BREATHE
  ```
  🌬️ Enables a breathing effect (Smooth inhale/exhale pattern on a single colour).

- **Wave Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_WAVE
  ```
  🌊 Enables a colour wave effect (Various colours transition from Left -> Mid -> Right zone of the keyboard).

- **Cycle Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_CYCLE
  ```
  🔄 Enables a breathing effect with various colour (Smooth inhale/exhale pattern over various colours).

- **Dance Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_DANCE
  ```
  🕺 Enables a flashy side-to-side dancing effect with various colours.

- **Flash Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_FLASH
  ```
  🔦 Makes the entire keyboard blink in various colours.

- **Tempo Effect:**  
  ```bash
  .\CLEVO_KeyboardColour.exe inbuilt KB_MODE_TEMPO
  ```
  🎶 Makes the keyboard lights 'dance' to the tempo of the music being played on the system (Keyboard tempo accuracy may vary on your system).

---

### ⏱️ Custom Speed:

- **Slower**
  ```bash
  .\CLEVO_KeyboardColour.exe theme colourtransform --speed 50
  ```
  🌈 Creates a slow rainbow sweep effect with smooth transitions at 50% speed.

- **Faster**  
  ```bash
  .\CLEVO_KeyboardColour.exe theme colourtransform --speed 200
  ```
  🌈 Creates a fast rainbow sweep effect with smooth transitions at 200% speed.


**This is an optional argument, and it will default to the standard speed if not provided.**

---

### 💻 Device Monitor Mode:

  Device Monitor mode is a feature for power users and developers to display extended console output of the in-flight data that is sent to the device when performing keyboard operations.

  ```bash
  .\CLEVO_KeyboardColour.exe theme colourtransform --dmm
  ```
  Plays the requested 'colourtransform' theme, while the console outputs the in-flight data being processed by the device.

  ```bash
  .\CLEVO_KeyboardColour.exe theme colourtransform --speed 150 --dmm
  ```
  Plays the requested 'colourtransform' theme at 150% speed, while the console outputs the in-flight data being processed by the device.

  ```bash
  .\CLEVO_KeyboardColour.exe backlight on --dmm
  ```
  Turns off the backlight for all detected devices on the system, and outputs the in-flight data being processed by the device.

  **This is an optional argument, and will not display the extended console output if the argument is not provided.**

  ⚠️ **Warning:** This argument will have a performance impact when provided and is therefore only recommended for debugging purposes. 

---

### 🐞 Debug Logging:

  Add `--debug` to any command to include detailed `[DEBUG]` messages, such as DCHU return values, keyboard codes and channel information. Debug output is hidden by default.

  ```bash
  .\CLEVO_KeyboardColour.exe backlight on --debug
  .\CLEVO_KeyboardColour.exe fan status --debug
  ```

  **Note:** `--dmm` (Device Monitor Mode) also enables debug logging.

---

### 🪄 Background Modes:

If you wish to run this program on (in the background), there are multiple approaches that can be used to hide the CLI Window. 

**On Startup (as a background application)**

 1. Adjust the command line in "Scripts\StartClevoKB.cmd" to play your favourite theme.
 
 2. Please run the "Scripts\RegisterClevoKbOnBoot.cmd" script **AS ADMINISTRATOR** and follow it's instructions. (Please use Scripts\UnregisterClevoKbOnBoot.cmd to deregister it from startup.)
 
 ⚠️ **WARNING:** This argument should not be used in conjunction with the "--dmm" (Device monitor mode) argument.
 
 **For hidden mode**

 1. Append the "--hide" argument to your requested theme command-line before launching.

**NOTE:** Please use Task Manager to locate close the application to play other themes.

⚠️ **WARNING:** This argument cannot be used in conjunction with the "--dmm" (Device monitor mode) argument.

---

## 🛠️ How to Use

1. Download the release which contains the version of the program that supports your system.

2. Open a **Command Prompt** in Administrator mode:
   - Search for "Command Prompt" in the Windows search bar.
   - Right-click and select **Run as Administrator**.

3. **Again, be sure that the **InsydeDCHU.dll** file is in the same directory as `CLEVO_KeyboardColour.exe` if you're using a **Single-Zone** keyboard or fan control**.

4. Run the desired command using the syntax:
   ```
   .\CLEVO_KeyboardColour.exe theme/inbuilt/backlight/lightbar/colour(s)/fan [argument] [--speed <speed>] [--dmm] [--hide] [--debug]
   ```

---

### Examples:
To activate the 'breathing' effect, run:
```bash
.\CLEVO_KeyboardColour.exe theme breathe
```

To activate an effect for a single iteration, add the `--once` argument:
```bash
.\CLEVO_KeyboardColour.exe theme breathe --once
```
NOTE: This will run the theme once, and stop. Use this command line if you want to experiment with the available themes.

To enable 'Device Monitor Mode' for the 'breathing' effect, run:
```bash
.\CLEVO_KeyboardColour.exe theme breathe --dmm
```
NOTE: This argument will have a performance impact when provided and is therefore only recommended for debugging purposes.

To activate the inbuilt 'Wave' effect, run:
```bash
.\CLEVO_KeyboardColour.exe inbuilt KB_MODE_WAVE
```
NOTE: This command does not support the `--once` argument.

⚠️ **Warning (1):** This will use the system's inbuilt themes, thus this application has no control over these types of themes.

⚠️ **Warning (2):** This is supported on 3-Zone keyboard ONLY.


To activate an effect to run in the background, add the `--hide` argument:
```bash
.\CLEVO_KeyboardColour.exe theme breathe --hide
```
NOTE: This will run the 'breathe' theme, and hide the command-line window. Please use Task Manager to locate and close the application to play other themes.

To turn off the keyboard backlight, run:
```bash
.\CLEVO_KeyboardColour.exe backlight off
```
NOTE: This command does not support the `--once` argument.

To check the fan status on an Insyde DCHU system, run:
```bash
.\CLEVO_KeyboardColour.exe fan status
```
NOTE: This command is read-only. Use `fan reset` to restore the firmware fan defaults after experimenting with the fan modes.

---

## ❗Common Errors

The section will address all of the most common errors and provide troubleshooting tips and fixes.

⚠️ **Error "Cannot load InsydeDCHU.dll..."**

- If you're using a **Single-Zone** keyboard system, please ensure that the **InsydeDCHU.dll** file is in the same directory as `CLEVO_KeyboardColour.exe`.
- Fan control uses the same DLL; ensure the DLL architecture matches the executable (x64 build with the x64 DLL).

⚠️ **Error "Failed to read fan status..." or "Failed to persist the fan mode..."**

- Ensure the Insyde DCHU driver/ACPI interface is available. It is usually installed by the OEM Control Center.
- Ensure **InsydeDCHU.dll** is in the same directory as `CLEVO_KeyboardColour.exe` and matches the executable architecture.
- Run `fan status` to check which fan features your firmware reports.

⚠️ **Error "Cannot load clevomof.dll..."**

- If you're using a **Triple-Zone** keyboard system, please ensure you've run the "Scripts\RegisterClevoMof.cmd" script **AS ADMINISTRATOR** and follow the instructions **carefully.**

- If you have already installed any variation of the "CLEVO Control Center", **you do not need to execute this script.**

⚠️ **Error "Please change the current directory (using the `cd` command) to 'Scripts\' directory before running this command." (RegisterClevoMof.cmd)**

1. Open a command prompt as Administrator.
2. Change directory (using the `cd` command) in this command prompt to the "Scripts" folder located in the same folder you extracted the files to.
3. Run "RegisterClevoMof.cmd" from this command prompt.
4. Follow the instructions outputted by the script.

⚠️**Application keeps disappearing when trying to launch it?** 

- Some anti-virus products may incorrectly flag this application as **malware** and quarantine the program. If this happens, **please add the application to the AV exclusion list**.

---

## 📋 Requirements

- **Compatible CLEVO & Tongfang based devices:**
  - **RGB 1-zone** or **3-zone** lighting.
  - **RGB Lightbar** lighting.
  - For **1-zone keyboards**, the **InsydeDCHU.dll** file must be in the same directory as `CLEVO_KeyboardColour.exe`.
  - For **3-zone keyboards**, the **CLEVOMOF.dll** must be registered using the "Scripts\RegisterClevoMof.cmd" script on 1st use.
- **Fan control:** Requires a firmware that exposes the Insyde DCHU fan interface. Run `fan status` to check support. The **InsydeDCHU.dll** file must be in the same directory as `CLEVO_KeyboardColour.exe`.
- **Administrator Permissions:** This program requests for administrator privileges. Please ensure you accept the Windows UAC prompt to avoid WMI failures.

---

## 🧱 Building from Source

See [BUILD.md](BUILD.md) for Visual Studio 2022 / MSBuild build instructions on Windows 11, including the required SDK/toolset, output paths, `InsydeDCHU.dll` placement, and troubleshooting.

---

## Legal / Disclaimer

**This is an unofficial, non-commercial project. CLEVO, Tongfang, Insyde and related names or binaries (except CLEVO_KeyboardColour.exe), are property of CLEVO. This project is not affiliated with, endorsed by, or sponsored by CLEVO, Tongfang or Insyde.**

---

## 📌 Final Notes

- ⭐ Feedback, suggestions, and contributions are highly appreciated to help improve the project.

---