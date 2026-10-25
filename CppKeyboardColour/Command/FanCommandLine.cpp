// Created by DeviceIoControl

#include "stdafx.h"
#include "CommandLine.h"
#include "FanCommandLine.h"
#include "../Console/Logger.h"

FanCommand ProcessFanCommandLine(const std::vector<std::wstring>& cmdLines)
{
	std::vector<std::wstring> ourCmds{};
	for (const auto& cmd : CommandLine::GetCommandsAfter(L"fan", cmdLines))
	{
		// Global flags are not fan subcommands.
		if (cmd != L"--debug" && cmd != L"--dmm" && cmd != L"--hide")
		{
			ourCmds.push_back(cmd);
		}
	}

	if (ourCmds.empty() || ourCmds[0] == L"status")
	{
		return FanCommand{ FanOperation::Status };
	}

	if (ourCmds[0] == L"mode")
	{
		if (ourCmds.size() < 2)
		{
			KB_ERROR(L"Invalid fan mode command. Usage: fan mode <auto|quiet|max|maxq|custom>");
			return {};
		}

		const auto mode = FanController::ParseMode(ourCmds[1]);
		if (!mode.has_value())
		{
			KB_ERROR(L"Invalid fan mode '%s'. Usage: fan mode <auto|quiet|max|maxq|custom>", ourCmds[1].c_str());
			return {};
		}

		return FanCommand{ FanOperation::SetMode, mode.value(), 0 };
	}

	if (ourCmds[0] == L"offset")
	{
		if (ourCmds.size() < 2)
		{
			KB_ERROR(L"Invalid fan offset command. Usage: fan offset <0-100>");
			return {};
		}

		const auto offset = xstd::stoi(ourCmds[1]);
		if (!offset.has_value() || offset.value() > 100)
		{
			KB_ERROR(L"Invalid fan offset '%s'. Must be between 0 and 100.", ourCmds[1].c_str());
			return {};
		}

		return FanCommand{ FanOperation::SetOffset, FanMode::Unknown, static_cast<uint8_t>(offset.value()) };
	}

	if (ourCmds[0] == L"dust" || ourCmds[0] == L"antidust" || ourCmds[0] == L"anti-dust")
	{
		return FanCommand{ FanOperation::AntiDust };
	}

	if (ourCmds[0] == L"reset" || ourCmds[0] == L"defaults")
	{
		return FanCommand{ FanOperation::LoadDefaults };
	}

	if (ourCmds[0] == L"curve")
	{
		if (ourCmds.size() < 2 || ourCmds[1] == L"show")
		{
			return FanCommand{ FanOperation::ShowCurve };
		}

		if (ourCmds[1] == L"set")
		{
			const bool dryRun = CommandLine::Contains(L"--dry-run", ourCmds);

			std::vector<std::wstring> args{};
			for (const auto& cmd : ourCmds)
			{
				if (cmd != L"--dry-run" && cmd != L"--debug")
				{
					args.push_back(cmd);
				}
			}

			if (args.size() < 7)
			{
				KB_ERROR(L"Invalid fan curve set command. Usage: fan curve set <cpu|gpu1|gpu2|all> <t2> <d2> <t3> <d3> [--dry-run]");
				KB_ERROR(L"T = temperature in Celsius, D = fan speed in percent.");
				return {};
			}

			FanCurveTarget target = FanCurveTarget::Cpu;
			if (args[2] == L"cpu")
			{
				target = FanCurveTarget::Cpu;
			}
			else if (args[2] == L"gpu1")
			{
				target = FanCurveTarget::Gpu1;
			}
			else if (args[2] == L"gpu2")
			{
				target = FanCurveTarget::Gpu2;
			}
			else if (args[2] == L"all")
			{
				target = FanCurveTarget::All;
			}
			else
			{
				KB_ERROR(L"Invalid fan target '%s'. Use cpu, gpu1, gpu2 or all.", args[2].c_str());
				return {};
			}

			const auto parseTemperature = [&args](size_t index, const wchar_t* name, uint8_t& out) -> bool
			{
				const auto value = xstd::stoi(args[index]);
				if (!value.has_value() || value.value() < 41 || value.value() > 99)
				{
					KB_ERROR(L"Invalid %s '%s'. Temperature must be between 41 and 99 C.", name, args[index].c_str());
					return false;
				}

				out = static_cast<uint8_t>(value.value());
				return true;
			};

			const auto parseDuty = [&args](size_t index, const wchar_t* name, uint8_t& out) -> bool
			{
				const auto value = xstd::stoi(args[index]);
				if (!value.has_value() || value.value() < 1 || value.value() > 99)
				{
					KB_ERROR(L"Invalid %s '%s'. Fan speed must be between 1 and 99%%.", name, args[index].c_str());
					return false;
				}

				out = static_cast<uint8_t>(value.value());
				return true;
			};

			FanCurveInput input{};
			if (!parseTemperature(3, L"T2", input.t2)
				|| !parseDuty(4, L"D2", input.d2)
				|| !parseTemperature(5, L"T3", input.t3)
				|| !parseDuty(6, L"D3", input.d3))
			{
				return {};
			}

			if (input.t2 >= input.t3)
			{
				KB_ERROR(L"T2 must be lower than T3.");
				return {};
			}

			if (input.d2 >= input.d3)
			{
				KB_ERROR(L"D2 must be lower than D3.");
				return {};
			}

			FanCommand command{};
			command.operation = FanOperation::SetCurve;
			command.curveTarget = target;
			command.curveInput = input;
			command.dryRun = dryRun;
			return command;
		}

		if (ourCmds[1] == L"reset")
		{
			KB_ERROR(L"'fan curve reset' is not supported. Use 'fan reset' to restore the firmware defaults.");
			return {};
		}

		KB_ERROR(L"Invalid fan curve command. Usage: fan curve show | fan curve set <cpu|gpu1|gpu2|all> <t2> <d2> <t3> <d3> [--dry-run]");
		return {};
	}

	KB_ERROR(L"Invalid fan command '%s'. Usage: fan status | fan mode <auto|quiet|max|maxq|custom> | fan offset <0-100> | fan dust | fan reset | fan curve show | fan curve set <cpu|gpu1|gpu2|all> <t2> <d2> <t3> <d3> [--dry-run] | fan help", ourCmds[0].c_str());

	return {};
}
