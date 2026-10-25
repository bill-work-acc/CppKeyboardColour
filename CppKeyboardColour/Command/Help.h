// Created by DeviceIoControl

#pragma once

#include <optional>
#include <string>
#include <vector>

enum class HelpScope
{
	General,
	Fan,
	FanCurve
};

std::optional<HelpScope> DetectHelpScope(const std::vector<std::wstring>& cmdLines);
void ShowHelp(HelpScope scope);
