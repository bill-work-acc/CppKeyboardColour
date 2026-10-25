// Created by DeviceIoControl

#include "stdafx.h"
#include "../Animator.h"
#include "CommandLine.h"
#include "ThemeCommandLine.h"
#include "FanCommandLine.h"
#include "../Console/Logger.h"
#include "../Devices/DeviceMask.h"
#include "../Fan/FanController.h"
#include "CommandOperationsHandler.h"

static DWORD DoBacklightOperation(std::unique_ptr<IHost> pHost, BacklightType backlight)
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (backlight == BacklightType::Invalid)
	{
		return ERROR_INVALID_PARAMETER;
	}

	switch (backlight)
	{
	case BacklightType::On:
		KB_INFO(L"Turning backlights on.");
		pHost->SetBacklightOn(DeviceMask::All);
		break;

	case BacklightType::Off:
		KB_INFO(L"Turning backlights off.");
		pHost->SetBacklightOff(DeviceMask::All);
		break;
	}

	return 0;
}

static DWORD DoThemeOperation(std::unique_ptr<IHost> pHost, std::unique_ptr<IAnimation> pAnimation, const std::vector<std::wstring>& cmdLines)
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (!pAnimation)
	{
		return ERROR_INVALID_PARAMETER;
	}

	Animator animator(std::move(pHost));

	// "--speed" command-line is only valid with an animation.
	const auto speedFactor = ProcessSpeedCommandLine(cmdLines);
	const auto setSpeedFactorSuccess = animator.SetSpeedFactor(speedFactor);

	if (!setSpeedFactorSuccess)
	{
		KB_WARN(L"Speed value out of range (1-250) - Will use default speed.");
	}

	if (setSpeedFactorSuccess && speedFactor != 1.0f)
	{
		KB_INFO(L"Custom speed: %0.f%%\n", (100 * speedFactor));
	}

	const auto bShouldLoop = !CommandLine::Contains(L"--once", cmdLines);
	return !animator.Play(pAnimation.get(), bShouldLoop) ? ERROR_NOT_SUPPORTED : ERROR_SUCCESS;
}

static DWORD DoSystemAnimationOperation(std::unique_ptr<IHost> pHost, SystemAnimation sysAnimation)
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (sysAnimation == SystemAnimation::KB_MODE_CUSTOM)
	{
		return ERROR_INVALID_PARAMETER;
	}

	// Restrict direct SendCode ability to WMI devices only (for now).
	// NOTE: If no keyboard is found, we would recieve a DeviceChannelType::None here.
	if (pHost->GetDeviceChannelType(DeviceMask::Keyboard) != DeviceChannelType::Wmi)
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"This operation is not supported on this system.");
	}

	KB_INFO(L"Playing Inbuilt Keyboard animation...");
	
	return !pHost->SendDeviceCode(DeviceMask::Keyboard, xstd::to_underlying(sysAnimation)) 
		? ERROR_NOT_SUPPORTED 
		: ERROR_SUCCESS;
}

static DWORD DoUserColourOperation(std::unique_ptr<IHost> pHost, std::optional<Colour> colour)
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (!colour.has_value())
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!(pHost->GetDevices() & DeviceMask::Keyboard))
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"This operation is not supported on this system.");
	}

	KB_INFO(L"Setting user provided colour...");
	
	return !pHost->SetColour(DeviceMask::Keyboard, Zone::ALL, colour.value())
		? ERROR_NOT_SUPPORTED
		: ERROR_SUCCESS;
}

static DWORD DoUserColour3Operation(std::unique_ptr<IHost> pHost, const std::optional<Colours>& colours)
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (pHost->GetKeyboardType() != KeyboardType::TRIPLE_ZONE)
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"This operation is not supported on this keyboard type.");
	}

	if (!colours.has_value() || colours->size() != 3)
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"Invalid argument: Must be 3 hexadecimal colours specified.");
	}

	KB_INFO(L"Setting user provided zone colours...");

	constexpr std::array zones = { Zone::LEFT, Zone::MID, Zone::RIGHT };

	for (size_t i = 0; i < colours->size(); ++i)
	{
		pHost->SetColour(DeviceMask::Keyboard, zones[i], colours->at(i));
	}

	return 0;
}

static DWORD DoLightbarOperation(std::unique_ptr<IHost> pHost, const std::optional<Colour>& colour) 
{
	// Somewhat unsafe, but pHost should always be valid before we get here.

	if (!colour.has_value())
	{
		return ERROR_INVALID_PARAMETER;
	}

	if (!(pHost->GetDevices() & DeviceMask::Lightbar))
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"This operation is not supported on this system.");
	}

	KB_INFO(L"Setting user provided lightbar colour...");

	return !pHost->SetColour(DeviceMask::Lightbar, Zone::ALL, colour.value())
		? ERROR_NOT_SUPPORTED
		: ERROR_SUCCESS;
}

static const wchar_t* FanCurveTargetToString(FanCurveTarget target)
{
	switch (target)
	{
	case FanCurveTarget::Cpu:
		return L"CPU";

	case FanCurveTarget::Gpu1:
		return L"GPU1";

	case FanCurveTarget::Gpu2:
		return L"GPU2";

	case FanCurveTarget::All:
		return L"all present fans";
	}

	return L"unknown";
}

static void PrintFanCurve(const wchar_t* name, const FanCurve& curve)
{
	KB_INFO(L"%s: %u C / %u%%  ->  %u C / %u%%  ->  %u C / %u%%  ->  %u C / %u%%",
		name,
		static_cast<unsigned>(curve.points[0].temperature),
		static_cast<unsigned>(curve.points[0].dutyPercent),
		static_cast<unsigned>(curve.points[1].temperature),
		static_cast<unsigned>(curve.points[1].dutyPercent),
		static_cast<unsigned>(curve.points[2].temperature),
		static_cast<unsigned>(curve.points[2].dutyPercent),
		static_cast<unsigned>(curve.points[3].temperature),
		static_cast<unsigned>(curve.points[3].dutyPercent));
}

static void PrintFanCurves(const FanCurves& curves)
{
	KB_INFO(L"Curve source: %s", curves.fromAppSettings ? L"OEM app settings" : L"EC table");

	PrintFanCurve(L"CPU", curves.cpu);

	if (curves.fanCount > 1)
	{
		PrintFanCurve(L"GPU1", curves.gpu1);
	}

	if (curves.fanCount > 2)
	{
		PrintFanCurve(L"GPU2", curves.gpu2);
	}
}

static void ApplyCurveInput(FanCurve& curve, const FanCurveInput& input)
{
	curve.points[1] = { input.t2, input.d2 };
	curve.points[2] = { input.t3, input.d3 };
}

static DWORD DoFanOperation(const FanCommand& command)
{
	if (command.operation == FanOperation::Invalid)
	{
		KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
		KB_FATAL(L"Invalid fan command.");
	}

	FanController fan;

	if (!fan.IsAvailable())
	{
		KB_SET_FATAL_ERROR(STATUS_DLL_NOT_FOUND);
		KB_FATAL(L"Cannot load %s. Please ensure the DLL is within the same directory!", kInsydeDchuDll);
	}

	switch (command.operation)
	{
	case FanOperation::Status:
	{
		FanStatus status{};

		if (!fan.ReadStatus(status))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to read fan status from the device.");
		}

		KB_INFO(L"Fan Count (firmware): %u", static_cast<unsigned>(status.fanCount));
		KB_INFO(L"Fan Mode: %s", FanController::ToString(status.mode));
		KB_INFO(L"Fan Offset: %u%%", static_cast<unsigned>(status.offset));
		KB_INFO(L"Fan Setting Supported: %s", status.fanSettingSupported ? L"Yes" : L"No");
		KB_INFO(L"Fanless System: %s", status.fanless ? L"Yes" : L"No");
		KB_INFO(L"Anti-Dust Supported: %s", status.antiDustSupported ? L"Yes" : L"No");
		KB_INFO(L"Fan Offset Supported: %s", status.fanOffsetSupported ? L"Yes" : L"No");

		if (!status.telemetryAvailable)
		{
			KB_WARN(L"Fan telemetry (RPM/duty/temperature) is not available on this system.");
			return ERROR_SUCCESS;
		}

		if (status.fanCount == 0 && status.fanless)
		{
			KB_WARN(L"No fan was reported by the device.");
			return ERROR_SUCCESS;
		}

		// The DCHU telemetry package always has three slots (CPU, GPU1, GPU2),
		// but the laptop may have fewer physical fans. Print a slot when the
		// firmware reports that many fans, or when the slot actually has data.
		const auto hasGpu1 = status.fanCount > 1
			|| status.gpu1.rpm > 0
			|| status.gpu1.dutyPercent > 0
			|| status.gpu1.temperature > 0;

		const auto hasGpu2 = status.fanCount > 2
			|| status.gpu2.rpm > 0
			|| status.gpu2.dutyPercent > 0
			|| status.gpu2.temperature > 0;

		KB_INFO(L"CPU Fan: %d RPM, %d%%, %d C", status.cpu.rpm, status.cpu.dutyPercent, status.cpu.temperature);

		if (hasGpu1)
		{
			KB_INFO(L"GPU1 Fan: %d RPM, %d%%, %d C", status.gpu1.rpm, status.gpu1.dutyPercent, status.gpu1.temperature);
		}

		if (hasGpu2)
		{
			KB_INFO(L"GPU2 Fan: %d RPM, %d%%, %d C", status.gpu2.rpm, status.gpu2.dutyPercent, status.gpu2.temperature);
		}

		return ERROR_SUCCESS;
	}

	case FanOperation::SetMode:
	{
		if (!fan.SetMode(command.mode))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to set the fan mode.");
		}

		KB_INFO(L"Fan mode set to %s.", FanController::ToString(command.mode));

		if (command.mode == FanMode::Custom)
		{
			KB_WARN(L"Custom mode uses the fan curve currently stored in the firmware/OEM application.");
		}

		return ERROR_SUCCESS;
	}

	case FanOperation::SetOffset:
	{
		if (!fan.SetOffset(command.offset))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to set the fan offset.");
		}

		KB_INFO(L"Fan offset set to %u%%.", static_cast<unsigned>(command.offset));
		return ERROR_SUCCESS;
	}

	case FanOperation::AntiDust:
	{
		if (!fan.RunAntiDust())
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to start anti-dust mode.");
		}

		KB_INFO(L"Anti-dust mode started.");
		return ERROR_SUCCESS;
	}

	case FanOperation::LoadDefaults:
	{
		if (!fan.LoadDefaults())
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to restore the default fan settings.");
		}

		KB_INFO(L"Default fan settings restored (auto mode).");
		return ERROR_SUCCESS;
	}

	case FanOperation::ShowCurve:
	{
		FanStatus status{};
		if (!fan.ReadStatus(status))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to read fan status from the device.");
		}

		KB_INFO(L"Fan Mode: %s", FanController::ToString(status.mode));
		KB_INFO(L"Fan Offset: %u%%", static_cast<unsigned>(status.offset));
		KB_INFO(L"Fan Count (firmware): %u", static_cast<unsigned>(status.fanCount));

		if (status.mode != FanMode::Custom)
		{
			KB_INFO(L"The custom curve is stored but not active. Use 'fan mode custom' to activate it.");
		}

		FanCurves curves{};
		if (!fan.ReadFanCurves(curves))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to read the fan curve from the device.");
		}

		PrintFanCurves(curves);
		return ERROR_SUCCESS;
	}

	case FanOperation::SetCurve:
	{
		FanCurves current{};
		if (!fan.ReadFanCurves(current))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to read the current fan curve from the device.");
		}

		if (command.curveTarget == FanCurveTarget::Gpu1 && current.fanCount < 2)
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"The GPU1 fan is not present on this system (fan count: %u).", static_cast<unsigned>(current.fanCount));
		}

		if (command.curveTarget == FanCurveTarget::Gpu2 && current.fanCount < 3)
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"The GPU2 fan is not present on this system (fan count: %u).", static_cast<unsigned>(current.fanCount));
		}

		FanCurves proposed = current;

		switch (command.curveTarget)
		{
		case FanCurveTarget::Cpu:
			ApplyCurveInput(proposed.cpu, command.curveInput);
			break;

		case FanCurveTarget::Gpu1:
			ApplyCurveInput(proposed.gpu1, command.curveInput);
			break;

		case FanCurveTarget::Gpu2:
			ApplyCurveInput(proposed.gpu2, command.curveInput);
			break;

		case FanCurveTarget::All:
			ApplyCurveInput(proposed.cpu, command.curveInput);

			if (current.fanCount > 1)
			{
				ApplyCurveInput(proposed.gpu1, command.curveInput);
			}

			if (current.fanCount > 2)
			{
				ApplyCurveInput(proposed.gpu2, command.curveInput);
			}
			break;
		}

		if (!FanController::IsCurveValid(proposed.cpu))
		{
			KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
			KB_FATAL(L"Invalid CPU curve. Ensure T2 > T1, T3 < T4, D2 > D1 and D3 < D4.");
		}

		if (current.fanCount > 1 && !FanController::IsCurveValid(proposed.gpu1))
		{
			KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
			KB_FATAL(L"Invalid GPU1 curve. Ensure T2 > T1, T3 < T4, D2 > D1 and D3 < D4.");
		}

		if (current.fanCount > 2 && !FanController::IsCurveValid(proposed.gpu2))
		{
			KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
			KB_FATAL(L"Invalid GPU2 curve. Ensure T2 > T1, T3 < T4, D2 > D1 and D3 < D4.");
		}

		KB_INFO(L"Target: %s", FanCurveTargetToString(command.curveTarget));

		KB_INFO(L"Current curve:");
		PrintFanCurves(current);

		KB_INFO(L"Proposed curve:");
		PrintFanCurves(proposed);

		if (!fan.WriteFanCurves(proposed, command.dryRun))
		{
			KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
			KB_FATAL(L"Failed to write the fan curve.");
		}

		if (command.dryRun)
		{
			KB_INFO(L"Dry run complete: no changes written.");
		}
		else
		{
			KB_INFO(L"Fan curve applied (Custom mode).");
			KB_INFO(L"Use 'fan status' to verify, 'fan mode auto' to return to Auto, or 'fan reset' to restore the firmware defaults.");
		}

		return ERROR_SUCCESS;
	}

	default:
		KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
		KB_FATAL(L"Invalid fan operation.");
	}

	return ERROR_INVALID_PARAMETER;
}

DWORD DoFanCommandOperation(const std::vector<std::wstring>& cmdLines)
{
	const auto command = ProcessFanCommandLine(cmdLines);
	return DoFanOperation(command);
}

DWORD DoCommandOperation(std::unique_ptr<IHost> pHost, const std::vector<std::wstring>& cmdLines)
{
	// pHost will always be valid before reaching here, so no need to check the pointer again.

	switch (ProcessCmdOperation(cmdLines))
	{
	case CmdOperation::Animation:
	{
		auto pAnimation = ProcessThemeCommandLine(cmdLines);
		return DoThemeOperation(std::move(pHost), std::move(pAnimation), cmdLines);
	}

	case CmdOperation::InBuilt:
	{
		const auto sysAnimation = ProcessSystemAnimationCommandLine(cmdLines);
		return DoSystemAnimationOperation(std::move(pHost), sysAnimation);
	}

	case CmdOperation::Backlight:
	{
		const auto backlight = ProcessBacklightCommandLine(cmdLines);
		return DoBacklightOperation(std::move(pHost), backlight);
	}

	case CmdOperation::UserColour:
	{
		const auto userColour = ProcessColourCommandLine(cmdLines);
		return DoUserColourOperation(std::move(pHost), userColour);
	}

	case CmdOperation::UserColour3:
	{
		const auto userColours = ProcessColoursCommandLine(cmdLines);
		return DoUserColour3Operation(std::move(pHost), userColours);
	}
	
	case CmdOperation::Lightbar:
	{
		const auto lightbarColour = ProcessLightbarCommandLine(cmdLines);
		return DoLightbarOperation(std::move(pHost), lightbarColour);
	}

	case CmdOperation::Fan:
	{
		return DoFanCommandOperation(cmdLines);
	}

	// Logic from the caller should ensure that we NEVER reach here.
	default:
		KB_SET_FATAL_ERROR(ERROR_INVALID_OPERATION);
		KB_FATAL(L"An invalid command was provided.");
	}

	KB_INFO(L"Exiting...");

	return 1;
}