// Created by DeviceIoControl

#include "stdafx.h"
#include "HostFactory.h"
#include "Console/Logger.h"
#include "Command/CommandLine.h"
#include "Command/Help.h"
#include "Console/ConsoleUtils.h"
#include "Utils/SingleInstanceEvent.h"
#include "Command/CommandOperationsHandler.h"

#pragma warning(disable: 4995)

static void DisplayHostInformation(const IHost* pHost)
{
	KB_INFO(L"Host Devices: %s", xstd::to_wstring(pHost->GetDevices()).c_str());

	if (!!(pHost->GetDevices() & DeviceMask::Keyboard))
	{
		KB_INFO(L"Keyboard Type: %s", xstd::to_wstring(pHost->GetKeyboardType()).c_str());
		KB_DEBUG(L"Keyboard Channel: %u", static_cast<unsigned>(pHost->GetDeviceChannelType(DeviceMask::Keyboard)));
	}

	KB_INFO(L"Model Name: %s", pHost->GetModelName().c_str());
}

static bool ShouldEnableDMM(const std::vector<std::wstring>& cmdLines)
{
#ifdef TEST_BUILD
	return true;
#else
	return CommandLine::Contains(L"--dmm", cmdLines);
#endif
}

int wmain(int argc, const wchar_t* argv[])
{
	const auto cmdLines = CommandLine::GetCommandLines(argc, argv);

	// Debug logging is opt-in via --debug. Device monitor mode is inherently a
	// debugging feature, so it enables debug logging as well.
	const auto enableDebugLog = CommandLine::Contains(L"--debug", cmdLines) || ShouldEnableDMM(cmdLines);
	AppConsole().SetLogLevel(enableDebugLog ? LogLevel::Debug : LogLevel::Info);
	AppConsole().SetTitle(PROGRAM_BOOTSTRAP_STRING);

	if (const auto helpScope = DetectHelpScope(cmdLines); helpScope.has_value())
	{
		ShowHelp(helpScope.value());
		return ERROR_SUCCESS;
	}

	if (!IsSingleInstance())
	{
		KB_SET_FATAL_ERROR(ERROR_ALREADY_EXISTS);
		KB_FATAL(L"You can only have 1 instance of this program running. Please close any other instances.");
	}
	
	const auto enableDeviceMonitorMode = ShouldEnableDMM(cmdLines);
	const auto enableHiddenMode = CommandLine::Contains(L"--hide", cmdLines);

	if (!CommandLine::ExclusiveContains({ L"theme", L"inbuilt", L"backlight", L"colour", L"colours", L"lightbar", L"fan" }, cmdLines))
	{
		KB_SET_FATAL_ERROR(ERROR_INVALID_PARAMETER);
		KB_FATAL(L"Invalid command line. Command is: CLEVO_KeyboardColour.exe theme/inbuilt/backlight/lightbar/colour(s)/fan/help [<themeName>/<hexColour>] [--once] [--speed] <speed> [--dmm] [--hide] [--debug]");
	}

	// Fan control only needs the InsydeDCHU.dll interface and is independent of
	// the keyboard model detection table. Fan commands are one-shot operations,
	// so --hide is not applied to them.
	if (CommandLine::Contains(L"fan", cmdLines))
	{
		return DoFanCommandOperation(cmdLines);
	}

	HostFactory hostFactory(USE_DEBUGGABLE_HOST, enableDeviceMonitorMode);
	auto pHost = hostFactory.Create();

	if (!pHost)
	{
		KB_SET_FATAL_ERROR(ERROR_NOT_SUPPORTED);
		KB_FATAL(L"This system is not supported.\nPlease request for support using the following URL: https://github.com/DeviceIoControl/CppKeyboardColour/issues/new/choose.");
	}

	DisplayHostInformation(pHost.get());

	if (enableDeviceMonitorMode)
	{
		KB_WARN(L"Device monitor mode enabled! (Performance may be affected)");

		if (enableHiddenMode)
		{
			KB_WARN(L"Cannot enable hidden mode with Device monitor mode enabled.");
		}
	}

	EnterHiddenMode(!enableDeviceMonitorMode && enableHiddenMode);

	return DoCommandOperation(std::move(pHost), cmdLines);
}
