// Created by DeviceIoControl

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Fan/FanController.h"

enum class FanOperation
{
	Invalid,
	Status,
	SetMode,
	SetOffset,
	AntiDust,
	LoadDefaults,
	ShowCurve,
	SetCurve
};

enum class FanCurveTarget
{
	Cpu,
	Gpu1,
	Gpu2,
	All
};

struct FanCurveInput
{
	uint8_t t2 = 0;
	uint8_t d2 = 0;
	uint8_t t3 = 0;
	uint8_t d3 = 0;
};

struct FanCommand
{
	FanOperation operation = FanOperation::Invalid;
	FanMode mode = FanMode::Unknown;
	uint8_t offset = 0;
	FanCurveTarget curveTarget = FanCurveTarget::Cpu;
	FanCurveInput curveInput{};
	bool dryRun = false;
};

FanCommand ProcessFanCommandLine(const std::vector<std::wstring>& cmdLines);
