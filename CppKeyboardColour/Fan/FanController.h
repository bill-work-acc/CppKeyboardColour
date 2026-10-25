// Created by DeviceIoControl

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

#include "../Channels/DchuApi.h"

enum class FanMode : uint8_t
{
	Auto = 0,
	Max = 1,
	Quiet = 3,
	MaxQ = 5,
	Custom = 6,
	Unknown = 0xFF
};

struct FanReading
{
	int rpm = 0;
	int dutyPercent = 0;
	int temperature = 0;
};

struct FanStatus
{
	uint8_t fanCount = 0;
	FanMode mode = FanMode::Unknown;
	uint8_t offset = 0;

	FanReading cpu{};
	FanReading gpu1{};
	FanReading gpu2{};

	bool telemetryAvailable = false;
	bool fanSettingSupported = false;
	bool fanless = false;
	bool antiDustSupported = false;
	bool fanOffsetSupported = true;
};

struct FanCurvePoint
{
	uint8_t temperature = 0;
	uint8_t dutyPercent = 0;
};

struct FanCurve
{
	// [0] = T1/D1 (fixed start), [1] = T2/D2, [2] = T3/D3, [3] = T4/D4 (fixed end)
	std::array<FanCurvePoint, 4> points{};
};

struct FanCurves
{
	FanCurve cpu{};
	FanCurve gpu1{};
	FanCurve gpu2{};
	uint8_t fanCount = 0;
	bool fromAppSettings = false;
};

class FanController
{
public:
	FanController() = default;

	bool IsAvailable() const;
	bool ReadStatus(FanStatus& status) const;
	bool SetMode(FanMode mode) const;
	bool SetOffset(uint8_t percent) const;
	bool RunAntiDust() const;
	bool LoadDefaults() const;

	bool ReadFanCurves(FanCurves& curves) const;
	bool WriteFanCurves(const FanCurves& curves, bool dryRun) const;

	static bool IsCurveValid(const FanCurve& curve);
	static std::optional<FanMode> ParseMode(const std::wstring& mode);
	static const wchar_t* ToString(FanMode mode);

private:
	bool ReadFanSpeed(FanStatus& status) const;
	bool ReadFanSettings(FanStatus& status) const;
	void ReadSupportFlags(FanStatus& status) const;

	DchuApi m_dchu{};
};
