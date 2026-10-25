// Created by DeviceIoControl

#include "stdafx.h"
#include "CommandLine.h"
#include "Help.h"
#include "../Console/ConsoleUtils.h"

namespace
{
	constexpr wchar_t kGeneralHelp[] = LR"(CLEVO Keyboard Colour
=====================

Usage:
  CLEVO_KeyboardColour.exe <command> [options]

Commands:
  theme <name>              Play a software animation theme.
  inbuilt <mode>            Play a firmware inbuilt keyboard animation (3-zone).
  backlight <on|off>        Turn the keyboard backlight on or off.
  colour <hex>              Set the keyboard colour, e.g. 0xff0000.
  colours <hex> <hex> <hex> Set the three keyboard zones (3-zone).
  lightbar <hex>            Set the lightbar colour.
  fan <subcommand>          Fan control. See: fan help
  help, /?                  Show this help.

Options:
  --speed <percent>         Animation speed (default 100).
  --once                    Play an animation once.
  --dmm                     Device monitor mode (debug output).
  --hide                    Hide the console window for background animations.
  --debug                   Show detailed [DEBUG] log messages.

Fan help:
  CLEVO_KeyboardColour.exe fan help
  CLEVO_KeyboardColour.exe fan /?
)";

	constexpr wchar_t kFanHelp[] = LR"(CLEVO Keyboard Colour - Fan
===========================

Usage:
  CLEVO_KeyboardColour.exe fan <subcommand>

Subcommands:
  status                    Show fan mode, offset, RPM, duty and temperature.
  mode <mode>               Set fan mode: auto, quiet, max, maxq, custom.
  offset <0-100>            Adjust the automatic fan curve offset.
  dust                      Run the firmware anti-dust routine.
  reset                     Restore the firmware default fan table and Auto mode.
  curve <subcommand>        Read or set the custom fan curve. See: fan curve help
  help, /?                  Show this help.

Notes:
  - Custom mode uses the custom 4-point fan curve.
  - fan reset is the only reset command; it restores defaults and Auto mode.
)";

	constexpr wchar_t kFanCurveHelp[] = LR"(CLEVO Keyboard Colour - Fan Curve
=================================

Usage:
  CLEVO_KeyboardColour.exe fan curve <subcommand>

Subcommands:
  show                      Show the current fan curve.
  set <fan> <t2> <d2> <t3> <d3> [--dry-run]
                            Set the adjustable curve points.
                            <fan> = cpu, gpu1, gpu2 or all
  help, /?                  Show this help.

Curve format:
  T = temperature in Celsius
  D = fan duty/speed in percent

  T1/D1   Fixed start point, defined by the firmware/model
          (for example 40 C / 35%)
  T2/D2   First adjustable point  (temperature / speed)
  T3/D3   Second adjustable point (temperature / speed)
  T4/D4   Fixed end point (normally 100 C / 100%)

  Fan speed is interpolated linearly between the points.

Example:
  CLEVO_KeyboardColour.exe fan curve set cpu 55 50 75 80

  Results in:
    40 C / 35%    (fixed start; actual value from fan curve show)
    55 C / 50%    (T2/D2)
    75 C / 80%    (T3/D3)
    100 C / 100%  (fixed end)

Validation:
  T2: 41-99, T3: T2+1-99
  D2/D3: 1-99, D2 < D3
  D2 > D1, D3 < D4

Notes:
  - set writes the EC fan table, switches to Custom mode, and persists page 4.
  - --dry-run prints the fan-table bytes and changes without writing.
  - Use 'fan mode auto' to return to the firmware curve.
  - fan reset restores the firmware default fan table and Auto mode.
  - gpu2 is only accepted when the firmware reports 3 fans.
  - The fixed T1/D1 values are read from the current table and may differ by model.
)";

	void WriteHelpText(const wchar_t* text)
	{
		const std::wstring help(text);
		size_t start = 0;

		while (start <= help.size())
		{
			const auto end = help.find(L'\n', start);
			if (end == std::wstring::npos)
			{
				if (start < help.size())
				{
					AppConsole().WriteLine(help.substr(start));
				}
				break;
			}

			AppConsole().WriteLine(help.substr(start, end - start));
			start = end + 1;
		}
	}
}

std::optional<HelpScope> DetectHelpScope(const std::vector<std::wstring>& cmdLines)
{
	const bool wantsHelp = CommandLine::Contains(L"help", cmdLines) || CommandLine::Contains(L"/?", cmdLines);
	if (!wantsHelp)
	{
		return std::nullopt;
	}

	const bool isFan = CommandLine::Contains(L"fan", cmdLines);
	const bool isCurve = CommandLine::Contains(L"curve", cmdLines);

	if (isFan && isCurve)
	{
		return HelpScope::FanCurve;
	}

	if (isFan)
	{
		return HelpScope::Fan;
	}

	return HelpScope::General;
}

void ShowHelp(HelpScope scope)
{
	switch (scope)
	{
	case HelpScope::General:
		WriteHelpText(kGeneralHelp);
		break;

	case HelpScope::Fan:
		WriteHelpText(kFanHelp);
		break;

	case HelpScope::FanCurve:
		WriteHelpText(kFanCurveHelp);
		break;
	}
}
