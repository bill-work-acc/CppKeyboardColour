// Created by DeviceIoControl

#include "stdafx.h"
#include "FanController.h"
#include "../Console/Logger.h"
#include <cmath>

namespace
{
	// DCHU command IDs used by the Insyde fan interface.
	constexpr uint32_t kCommandFanSpeed = 12;
	constexpr uint32_t kCommandFanInfo = 13;
	constexpr uint32_t kCommandFanCount = 56;
	constexpr uint32_t kCommandBiosSpecialFeature = 16;
	constexpr uint32_t kCommandFanFeatures = 96;
	constexpr uint32_t kCommandFanless = 122;
	constexpr uint32_t kCommandSystemControl = 121;

	// Sub-commands for DCHU command 121 (SystemControlFunction).
	constexpr uint8_t kSubCommandFanMode = 1;
	constexpr uint8_t kSubCommandFanOffset = 14;
	constexpr uint8_t kSubCommandLoadDefaults = 34;
	constexpr uint8_t kSubCommandAntiDust = 41;

	// App-settings page/offsets used by the OEM fan application.
	constexpr uint32_t kFanSettingsPage = 4;
	constexpr uint32_t kFanModeOffset = 5;
	constexpr uint32_t kFanOffsetOffset = 7;
	constexpr uint32_t kAntiDustStatusOffset = 80;

	// The DCHU DLL appends one extra byte after the package data, so this must
	// be larger than the 0x400-byte DCHU output buffer.
	constexpr size_t kFanBufferSize = 0x800;

	int TachToRpm(uint16_t tach)
	{
		if (tach == 0)
		{
			return 0;
		}

		// Same conversion used by the OEM fan application:
		// raw tach period -> displayed RPM.
		constexpr double kTachScale = 0.00005565217391304348;
		return static_cast<int>(std::lround(60.0 / (kTachScale * tach))) * 2;
	}

	int DutyToPercent(uint8_t duty)
	{
		return (static_cast<int>(duty) * 100) / 255;
	}

	FanMode ToFanMode(uint8_t mode)
	{
		switch (mode)
		{
		case static_cast<uint8_t>(FanMode::Auto):
			return FanMode::Auto;

		case static_cast<uint8_t>(FanMode::Max):
			return FanMode::Max;

		case static_cast<uint8_t>(FanMode::Quiet):
			return FanMode::Quiet;

		case static_cast<uint8_t>(FanMode::MaxQ):
			return FanMode::MaxQ;

		case static_cast<uint8_t>(FanMode::Custom):
			return FanMode::Custom;
		}

		return FanMode::Unknown;
	}

	constexpr uint32_t kCommandFanTable = 14;

	constexpr size_t kFanTableCpuOffset = 2;
	constexpr size_t kFanTableGpu1Offset = 6;
	constexpr size_t kFanTableGpu2Offset = 10;
	constexpr size_t kFanTableCpuSlopeOffset = 14;
	constexpr size_t kFanTableGpu1SlopeOffset = 20;
	constexpr size_t kFanTableGpu2SlopeOffset = 26;

	constexpr size_t kFanInfoCpuCurveOffset = 16;
	constexpr size_t kFanInfoGpu1CurveOffset = 24;
	constexpr size_t kFanInfoGpu2CurveOffset = 32;

	constexpr size_t kFanCurveCpuDutyOffset = 16;
	constexpr size_t kFanCurveCpuDDefaultOffset = 20;
	constexpr size_t kFanCurveCpuTempOffset = 22;
	constexpr size_t kFanCurveCpuTDefaultOffset = 26;
	constexpr size_t kFanCurveCpuSlopeOffset = 28;

	constexpr size_t kFanCurveGpu1DutyOffset = 34;
	constexpr size_t kFanCurveGpu1DDefaultOffset = 38;
	constexpr size_t kFanCurveGpu1TempOffset = 40;
	constexpr size_t kFanCurveGpu1TDefaultOffset = 44;
	constexpr size_t kFanCurveGpu1SlopeOffset = 46;

	constexpr size_t kFanCurveGpu2DutyOffset = 52;
	constexpr size_t kFanCurveGpu2DDefaultOffset = 56;
	constexpr size_t kFanCurveGpu2TempOffset = 58;
	constexpr size_t kFanCurveGpu2TDefaultOffset = 62;
	constexpr size_t kFanCurveGpu2SlopeOffset = 64;

	uint8_t PercentToDuty(uint8_t percent)
	{
		return static_cast<uint8_t>(std::lround((static_cast<double>(percent) / 100.0) * 255.0));
	}

	int CalculateSlope(uint8_t d1, uint8_t t1, uint8_t d2, uint8_t t2)
	{
		const int deltaTemp = static_cast<int>(t2) - static_cast<int>(t1);
		if (deltaTemp == 0)
		{
			return 0;
		}

		const auto slope = static_cast<int>(std::lround(
			(static_cast<double>(d2) - static_cast<double>(d1)) / deltaTemp * 2.55 * 16.0));

		return std::clamp(slope, -32768, 32767);
	}

	void WriteSlopeBE(std::array<uint8_t, 256>& buffer, size_t offset, int slope)
	{
		buffer[offset] = static_cast<uint8_t>((slope >> 8) & 0xff);
		buffer[offset + 1] = static_cast<uint8_t>(slope & 0xff);
	}

	void WriteSlopeLE(std::array<uint8_t, 256>& buffer, size_t offset, int slope)
	{
		buffer[offset] = static_cast<uint8_t>(slope & 0xff);
		buffer[offset + 1] = static_cast<uint8_t>((slope >> 8) & 0xff);
	}

	bool IsCurveValidInternal(const FanCurve& curve)
	{
		const auto& points = curve.points;

		if (points[0].temperature == 0 || points[0].temperature >= points[1].temperature)
		{
			return false;
		}

		if (points[1].temperature >= points[2].temperature || points[2].temperature >= points[3].temperature)
		{
			return false;
		}

		if (points[3].temperature > 100 || points[3].dutyPercent > 100)
		{
			return false;
		}

		if (points[0].dutyPercent >= points[1].dutyPercent
			|| points[1].dutyPercent >= points[2].dutyPercent
			|| points[2].dutyPercent >= points[3].dutyPercent)
		{
			return false;
		}

		return true;
	}

	bool ParseCurveFromAppSettings(const uint8_t* page, size_t dutyOffset, size_t tempOffset, FanCurve& curve)
	{
		curve = {};
		curve.points[0] = { page[tempOffset + 0], page[dutyOffset + 0] };
		curve.points[1] = { page[tempOffset + 1], page[dutyOffset + 1] };
		curve.points[2] = { page[tempOffset + 2], page[dutyOffset + 2] };
		curve.points[3] = { page[tempOffset + 3], page[dutyOffset + 3] };

		if (curve.points[3].temperature != 100 || curve.points[3].dutyPercent != 100)
		{
			return false;
		}

		return IsCurveValidInternal(curve);
	}

	bool ParseCurveFromFanInfo(const uint8_t* fanInfo, size_t offset, FanCurve& curve)
	{
		curve = {};
		curve.points[0] = { fanInfo[offset + 0], static_cast<uint8_t>(DutyToPercent(fanInfo[offset + 1])) };
		curve.points[1] = { fanInfo[offset + 2], static_cast<uint8_t>(DutyToPercent(fanInfo[offset + 3])) };
		curve.points[2] = { fanInfo[offset + 4], static_cast<uint8_t>(DutyToPercent(fanInfo[offset + 5])) };
		curve.points[3] = { 100, 100 };

		return IsCurveValidInternal(curve);
	}

	void WriteCurveToFanTable(std::array<uint8_t, 256>& table, const FanCurve& curve, size_t curveOffset, size_t slopeOffset)
	{
		table[curveOffset + 0] = curve.points[1].temperature;
		table[curveOffset + 1] = PercentToDuty(curve.points[1].dutyPercent);
		table[curveOffset + 2] = curve.points[2].temperature;
		table[curveOffset + 3] = PercentToDuty(curve.points[2].dutyPercent);

		WriteSlopeBE(table, slopeOffset + 0, CalculateSlope(
			curve.points[0].dutyPercent, curve.points[0].temperature,
			curve.points[1].dutyPercent, curve.points[1].temperature));

		WriteSlopeBE(table, slopeOffset + 2, CalculateSlope(
			curve.points[1].dutyPercent, curve.points[1].temperature,
			curve.points[2].dutyPercent, curve.points[2].temperature));

		WriteSlopeBE(table, slopeOffset + 4, CalculateSlope(
			curve.points[2].dutyPercent, curve.points[2].temperature,
			curve.points[3].dutyPercent, curve.points[3].temperature));
	}

	void WriteCurveToAppSettings(
		std::array<uint8_t, 256>& page,
		const FanCurve& curve,
		size_t dutyOffset,
		size_t tempOffset,
		size_t slopeOffset,
		size_t dutyDefaultOffset,
		size_t tempDefaultOffset)
	{
		page[dutyOffset + 0] = curve.points[0].dutyPercent;
		page[dutyOffset + 1] = curve.points[1].dutyPercent;
		page[dutyOffset + 2] = curve.points[2].dutyPercent;
		page[dutyOffset + 3] = curve.points[3].dutyPercent;

		page[tempOffset + 0] = curve.points[0].temperature;
		page[tempOffset + 1] = curve.points[1].temperature;
		page[tempOffset + 2] = curve.points[2].temperature;
		page[tempOffset + 3] = curve.points[3].temperature;

		page[dutyDefaultOffset + 0] = curve.points[1].dutyPercent;
		page[dutyDefaultOffset + 1] = curve.points[2].dutyPercent;
		page[tempDefaultOffset + 0] = curve.points[1].temperature;
		page[tempDefaultOffset + 1] = curve.points[2].temperature;

		WriteSlopeLE(page, slopeOffset + 0, CalculateSlope(
			curve.points[0].dutyPercent, curve.points[0].temperature,
			curve.points[1].dutyPercent, curve.points[1].temperature));

		WriteSlopeLE(page, slopeOffset + 2, CalculateSlope(
			curve.points[1].dutyPercent, curve.points[1].temperature,
			curve.points[2].dutyPercent, curve.points[2].temperature));

		WriteSlopeLE(page, slopeOffset + 4, CalculateSlope(
			curve.points[2].dutyPercent, curve.points[2].temperature,
			curve.points[3].dutyPercent, curve.points[3].temperature));
	}
}

bool FanController::IsAvailable() const
{
	return m_dchu.IsLoaded();
}

bool FanController::ReadStatus(FanStatus& status) const
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	status = {};

	ReadSupportFlags(status);
	status.telemetryAvailable = ReadFanSpeed(status);
	ReadFanSettings(status);

	return true;
}

bool FanController::ReadFanSpeed(FanStatus& status) const
{
	std::array<uint8_t, kFanBufferSize> buffer{};

	if (!m_dchu.GetDataBuffer(kCommandFanSpeed, buffer.data(), buffer.size()))
	{
		return false;
	}

	status.cpu.rpm = TachToRpm(static_cast<uint16_t>((buffer[2] << 8) | buffer[3]));
	status.cpu.dutyPercent = DutyToPercent(buffer[16]);
	status.cpu.temperature = buffer[18];

	status.gpu1.rpm = TachToRpm(static_cast<uint16_t>((buffer[4] << 8) | buffer[5]));
	status.gpu1.dutyPercent = DutyToPercent(buffer[19]);
	status.gpu1.temperature = buffer[21];

	status.gpu2.rpm = TachToRpm(static_cast<uint16_t>((buffer[6] << 8) | buffer[7]));
	status.gpu2.dutyPercent = DutyToPercent(buffer[22]);
	status.gpu2.temperature = buffer[24];

	return true;
}

bool FanController::ReadFanSettings(FanStatus& status) const
{
	std::array<uint8_t, kFanBufferSize> fanInfo{};
	uint8_t fanCount = 0;

	// Package 13 carries the fan table and its fan count is what the OEM
	// application uses. Prefer it over the separate fan-count command, which
	// reports 1 on some two-fan models.
	if (m_dchu.GetDataBuffer(kCommandFanInfo, fanInfo.data(), fanInfo.size()))
	{
		fanCount = fanInfo[12];
	}

	if (fanCount == 0)
	{
		uint32_t dchuFanCount = 0;
		if (m_dchu.GetDataInteger(kCommandFanCount, dchuFanCount) && dchuFanCount > 0 && dchuFanCount <= 0xff)
		{
			fanCount = static_cast<uint8_t>(dchuFanCount);
		}
	}

	if (fanCount == 0 && !status.fanless)
	{
		// The telemetry command succeeded, so at least one fan is present even if
		// the fan-count commands are not exposed by this firmware.
		fanCount = 1;
	}

	status.fanCount = fanCount;

	uint8_t mode = 0;
	if (m_dchu.ReadAppSettings(kFanSettingsPage, kFanModeOffset, 1, &mode, sizeof(mode)))
	{
		status.mode = ToFanMode(mode);
	}

	uint8_t offset = 0;
	if (m_dchu.ReadAppSettings(kFanSettingsPage, kFanOffsetOffset, 1, &offset, sizeof(offset)))
	{
		status.offset = offset;
	}

	return true;
}

void FanController::ReadSupportFlags(FanStatus& status) const
{
	uint32_t bits = 0;

	if (m_dchu.GetDataInteger(kCommandBiosSpecialFeature, bits))
	{
		status.fanSettingSupported = (bits & (1u << 7)) != 0;
	}

	bits = 0;
	if (m_dchu.GetDataInteger(kCommandFanless, bits))
	{
		status.fanless = (bits & (1u << 15)) != 0;
	}

	bits = 0;
	if (m_dchu.GetDataInteger(kCommandFanFeatures, bits))
	{
		status.antiDustSupported = (bits & (1u << 7)) != 0;
		status.fanOffsetSupported = (bits & (1u << 10)) == 0;
	}
}

bool FanController::SetMode(FanMode mode) const
{
	if (!m_dchu.IsLoaded() || mode == FanMode::Unknown)
	{
		return false;
	}

	const auto modeValue = static_cast<uint8_t>(mode);
	const std::array<uint8_t, 4> commandBuffer{ modeValue, 0, 0, kSubCommandFanMode };

	if (!m_dchu.SetData(kCommandSystemControl, commandBuffer.data(), static_cast<uint32_t>(commandBuffer.size())))
	{
		return false;
	}

	// Persist the mode in the OEM app-settings page as the Control Center does.
	// A failed write usually means the DCHU driver/ACPI interface is not
	// responding, so report the operation as failed.
	if (!m_dchu.WriteAppSettings(kFanSettingsPage, kFanModeOffset, 1, &modeValue))
	{
		KB_ERROR(L"Failed to persist the fan mode in the OEM fan settings.");
		return false;
	}

	uint8_t readBackMode = 0;
	if (!m_dchu.ReadAppSettings(kFanSettingsPage, kFanModeOffset, 1, &readBackMode, sizeof(readBackMode)) || readBackMode != modeValue)
	{
		KB_ERROR(L"Failed to verify the fan mode in the OEM fan settings.");
		return false;
	}

	return true;
}

bool FanController::SetOffset(uint8_t percent) const
{
	if (!m_dchu.IsLoaded() || percent > 100)
	{
		return false;
	}

	const auto scaledOffset = static_cast<uint8_t>(std::lround(255.0 * percent / 100.0));
	const std::array<uint8_t, 4> commandBuffer{ scaledOffset, 0, 0, kSubCommandFanOffset };

	if (!m_dchu.SetData(kCommandSystemControl, commandBuffer.data(), static_cast<uint32_t>(commandBuffer.size())))
	{
		return false;
	}

	// The app-settings page stores the percentage, while the DCHU command uses
	// the 0-255 scale.
	if (!m_dchu.WriteAppSettings(kFanSettingsPage, kFanOffsetOffset, 1, &percent))
	{
		KB_ERROR(L"Failed to persist the fan offset in the OEM fan settings.");
		return false;
	}

	uint8_t readBackOffset = 0;
	if (!m_dchu.ReadAppSettings(kFanSettingsPage, kFanOffsetOffset, 1, &readBackOffset, sizeof(readBackOffset)) || readBackOffset != percent)
	{
		KB_ERROR(L"Failed to verify the fan offset in the OEM fan settings.");
		return false;
	}

	return true;
}

bool FanController::RunAntiDust() const
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	const std::array<uint8_t, 4> commandBuffer{ 1, 0, 0, kSubCommandAntiDust };

	if (!m_dchu.SetData(kCommandSystemControl, commandBuffer.data(), static_cast<uint32_t>(commandBuffer.size())))
	{
		return false;
	}

	// Status update for the OEM app; the EC performs the actual run.
	const uint8_t status = 1;
	if (!m_dchu.WriteAppSettings(kFanSettingsPage, kAntiDustStatusOffset, 1, &status))
	{
		KB_ERROR(L"Failed to update the anti-dust status in the OEM fan settings.");
		return false;
	}

	return true;
}

bool FanController::LoadDefaults() const
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	const std::array<uint8_t, 4> commandBuffer{ 1, 0, 0, kSubCommandLoadDefaults };

	if (!m_dchu.SetData(kCommandSystemControl, commandBuffer.data(), static_cast<uint32_t>(commandBuffer.size())))
	{
		return false;
	}

	return this->SetMode(FanMode::Auto);
}

bool FanController::IsCurveValid(const FanCurve& curve)
{
	return IsCurveValidInternal(curve);
}

bool FanController::ReadFanCurves(FanCurves& curves) const
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	curves = {};

	std::array<uint8_t, kFanBufferSize> fanInfo{};
	const bool hasFanInfo = m_dchu.GetDataBuffer(kCommandFanInfo, fanInfo.data(), fanInfo.size());

	uint8_t fanCount = 0;
	if (hasFanInfo)
	{
		fanCount = fanInfo[12];
	}

	if (fanCount == 0)
	{
		uint32_t dchuFanCount = 0;
		if (m_dchu.GetDataInteger(kCommandFanCount, dchuFanCount) && dchuFanCount > 0 && dchuFanCount <= 0xff)
		{
			fanCount = static_cast<uint8_t>(dchuFanCount);
		}
	}

	if (fanCount == 0)
	{
		fanCount = 1;
	}

	curves.fanCount = fanCount;

	std::array<uint8_t, kFanBufferSize> page{};
	const bool hasPage = m_dchu.ReadAppSettings(kFanSettingsPage, 0, 256, page.data(), page.size());

	bool cpuValid = false;

	if (hasFanInfo)
	{
		// The EC table is the table the firmware is actually using.
		cpuValid = ParseCurveFromFanInfo(fanInfo.data(), kFanInfoCpuCurveOffset, curves.cpu);

		if (!ParseCurveFromFanInfo(fanInfo.data(), kFanInfoGpu1CurveOffset, curves.gpu1))
		{
			curves.gpu1 = {};
		}

		if (!ParseCurveFromFanInfo(fanInfo.data(), kFanInfoGpu2CurveOffset, curves.gpu2))
		{
			curves.gpu2 = {};
		}
	}

	if (!cpuValid && hasPage)
	{
		// Fall back to the OEM app's stored curve if the EC table is unreadable.
		cpuValid = ParseCurveFromAppSettings(page.data(), kFanCurveCpuDutyOffset, kFanCurveCpuTempOffset, curves.cpu);
		if (cpuValid)
		{
			curves.fromAppSettings = true;
		}

		if (!ParseCurveFromAppSettings(page.data(), kFanCurveGpu1DutyOffset, kFanCurveGpu1TempOffset, curves.gpu1))
		{
			curves.gpu1 = {};
		}

		if (!ParseCurveFromAppSettings(page.data(), kFanCurveGpu2DutyOffset, kFanCurveGpu2TempOffset, curves.gpu2))
		{
			curves.gpu2 = {};
		}
	}
	else if (hasPage)
	{
		// The EC table is active; fill a missing present GPU curve from page 4.
		if (curves.fanCount > 1 && !IsCurveValidInternal(curves.gpu1))
		{
			if (ParseCurveFromAppSettings(page.data(), kFanCurveGpu1DutyOffset, kFanCurveGpu1TempOffset, curves.gpu1))
			{
				curves.fromAppSettings = true;
			}
			else
			{
				curves.gpu1 = {};
			}
		}

		if (curves.fanCount > 2 && !IsCurveValidInternal(curves.gpu2))
		{
			if (ParseCurveFromAppSettings(page.data(), kFanCurveGpu2DutyOffset, kFanCurveGpu2TempOffset, curves.gpu2))
			{
				curves.fromAppSettings = true;
			}
			else
			{
				curves.gpu2 = {};
			}
		}
	}

	if (!cpuValid)
	{
		return false;
	}

	return IsCurveValidInternal(curves.cpu);
}

bool FanController::WriteFanCurves(const FanCurves& curves, bool dryRun) const
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	if (!IsCurveValidInternal(curves.cpu))
	{
		KB_ERROR(L"Invalid CPU fan curve.");
		return false;
	}

	if (curves.fanCount > 1 && !IsCurveValidInternal(curves.gpu1))
	{
		KB_ERROR(L"Invalid GPU1 fan curve.");
		return false;
	}

	if (curves.fanCount > 2 && !IsCurveValidInternal(curves.gpu2))
	{
		KB_ERROR(L"Invalid GPU2 fan curve.");
		return false;
	}

	std::array<uint8_t, kFanBufferSize> currentPage{};
	if (!m_dchu.ReadAppSettings(kFanSettingsPage, 0, 256, currentPage.data(), currentPage.size()))
	{
		KB_ERROR(L"Failed to read the OEM fan settings page.");
		return false;
	}

	std::array<uint8_t, kFanBufferSize> currentFanInfo{};
	const bool hasFanInfo = m_dchu.GetDataBuffer(kCommandFanInfo, currentFanInfo.data(), currentFanInfo.size());

	std::array<uint8_t, 256> fanTable{};
	WriteCurveToFanTable(fanTable, curves.cpu, kFanTableCpuOffset, kFanTableCpuSlopeOffset);
	WriteCurveToFanTable(fanTable, curves.gpu1, kFanTableGpu1Offset, kFanTableGpu1SlopeOffset);
	WriteCurveToFanTable(fanTable, curves.gpu2, kFanTableGpu2Offset, kFanTableGpu2SlopeOffset);

	if (hasFanInfo)
	{
		// Preserve slots that the laptop does not have.
		if (curves.fanCount < 2)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				fanTable[kFanTableGpu1Offset + i] = currentFanInfo[kFanTableGpu1Offset + i];
			}

			for (size_t i = 0; i < 6; ++i)
			{
				fanTable[kFanTableGpu1SlopeOffset + i] = currentFanInfo[kFanTableGpu1SlopeOffset + i];
			}
		}

		if (curves.fanCount < 3)
		{
			for (size_t i = 0; i < 4; ++i)
			{
				fanTable[kFanTableGpu2Offset + i] = currentFanInfo[kFanTableGpu2Offset + i];
			}

			for (size_t i = 0; i < 6; ++i)
			{
				fanTable[kFanTableGpu2SlopeOffset + i] = currentFanInfo[kFanTableGpu2SlopeOffset + i];
			}
		}
	}

	std::array<uint8_t, 256> newPage{};
	std::copy(currentPage.begin(), currentPage.begin() + 256, newPage.begin());

	WriteCurveToAppSettings(
		newPage,
		curves.cpu,
		kFanCurveCpuDutyOffset,
		kFanCurveCpuTempOffset,
		kFanCurveCpuSlopeOffset,
		kFanCurveCpuDDefaultOffset,
		kFanCurveCpuTDefaultOffset);

	if (curves.fanCount > 1)
	{
		WriteCurveToAppSettings(
			newPage,
			curves.gpu1,
			kFanCurveGpu1DutyOffset,
			kFanCurveGpu1TempOffset,
			kFanCurveGpu1SlopeOffset,
			kFanCurveGpu1DDefaultOffset,
			kFanCurveGpu1TDefaultOffset);
	}

	if (curves.fanCount > 2)
	{
		WriteCurveToAppSettings(
			newPage,
			curves.gpu2,
			kFanCurveGpu2DutyOffset,
			kFanCurveGpu2TempOffset,
			kFanCurveGpu2SlopeOffset,
			kFanCurveGpu2DDefaultOffset,
			kFanCurveGpu2TDefaultOffset);
	}

	newPage[5] = static_cast<uint8_t>(FanMode::Custom);
	newPage[6] = curves.fanCount;

	if (dryRun)
	{
		KB_INFO(L"Dry run - no changes will be written.");
		KB_INFO(L"Fan table bytes 2..31:");

		for (size_t row = 2; row < 32; row += 16)
		{
			std::wstring line;
			for (size_t i = row; i < row + 16 && i < 32; ++i)
			{
				line += xstd::format(L"%02X ", static_cast<unsigned>(fanTable[i]));
			}

			KB_INFO(L"  %s", line.c_str());
		}

		KB_INFO(L"Page 4: mode -> Custom, curve updated for %u fan(s).", static_cast<unsigned>(curves.fanCount));
		return true;
	}

	if (!m_dchu.SetData(kCommandFanTable, fanTable.data(), 256))
	{
		KB_ERROR(L"Failed to write the EC fan table.");
		return false;
	}

	if (!this->SetMode(FanMode::Custom))
	{
		KB_ERROR(L"Failed to switch the fan mode to Custom.");
		return false;
	}

	if (!m_dchu.WriteAppSettings(kFanSettingsPage, 0, 256, newPage.data()))
	{
		KB_ERROR(L"Failed to persist the fan curve in the OEM fan settings.");
		return false;
	}

	std::array<uint8_t, kFanBufferSize> verifyPage{};
	if (!m_dchu.ReadAppSettings(kFanSettingsPage, 0, 256, verifyPage.data(), verifyPage.size()))
	{
		KB_ERROR(L"Failed to verify the OEM fan settings.");
		return false;
	}

	if (verifyPage[5] != static_cast<uint8_t>(FanMode::Custom))
	{
		KB_ERROR(L"Fan mode verification failed.");
		return false;
	}

	const auto verifyCurve = [&verifyPage](const FanCurve& curve, size_t dutyOffset, size_t tempOffset) -> bool
	{
		return verifyPage[dutyOffset + 0] == curve.points[0].dutyPercent
			&& verifyPage[dutyOffset + 1] == curve.points[1].dutyPercent
			&& verifyPage[dutyOffset + 2] == curve.points[2].dutyPercent
			&& verifyPage[dutyOffset + 3] == curve.points[3].dutyPercent
			&& verifyPage[tempOffset + 0] == curve.points[0].temperature
			&& verifyPage[tempOffset + 1] == curve.points[1].temperature
			&& verifyPage[tempOffset + 2] == curve.points[2].temperature
			&& verifyPage[tempOffset + 3] == curve.points[3].temperature;
	};

	if (!verifyCurve(curves.cpu, kFanCurveCpuDutyOffset, kFanCurveCpuTempOffset)
		|| (curves.fanCount > 1 && !verifyCurve(curves.gpu1, kFanCurveGpu1DutyOffset, kFanCurveGpu1TempOffset))
		|| (curves.fanCount > 2 && !verifyCurve(curves.gpu2, kFanCurveGpu2DutyOffset, kFanCurveGpu2TempOffset)))
	{
		KB_ERROR(L"Fan curve verification failed.");
		return false;
	}

	// Best-effort EC-table readback; the page-4 verification above is authoritative.
	std::array<uint8_t, kFanBufferSize> verifyFanInfo{};
	if (m_dchu.GetDataBuffer(kCommandFanInfo, verifyFanInfo.data(), verifyFanInfo.size()))
	{
		const auto verifyTableCurve = [&verifyFanInfo](const FanCurve& curve, size_t offset) -> bool
		{
			return verifyFanInfo[offset + 0] == curve.points[1].temperature
				&& verifyFanInfo[offset + 1] == PercentToDuty(curve.points[1].dutyPercent)
				&& verifyFanInfo[offset + 2] == curve.points[2].temperature
				&& verifyFanInfo[offset + 3] == PercentToDuty(curve.points[2].dutyPercent);
		};

		if (!verifyTableCurve(curves.cpu, kFanInfoCpuCurveOffset + 2)
			|| (curves.fanCount > 1 && !verifyTableCurve(curves.gpu1, kFanInfoGpu1CurveOffset + 2))
			|| (curves.fanCount > 2 && !verifyTableCurve(curves.gpu2, kFanInfoGpu2CurveOffset + 2)))
		{
			KB_WARN(L"The EC fan table readback does not match the requested curve. Use 'fan status' to verify the result.");
		}
	}

	return true;
}

std::optional<FanMode> FanController::ParseMode(const std::wstring& mode)
{
	if (mode == L"auto")
	{
		return FanMode::Auto;
	}

	if (mode == L"max")
	{
		return FanMode::Max;
	}

	if (mode == L"quiet" || mode == L"silent")
	{
		return FanMode::Quiet;
	}

	if (mode == L"maxq" || mode == L"max-q")
	{
		return FanMode::MaxQ;
	}

	if (mode == L"custom")
	{
		return FanMode::Custom;
	}

	return std::nullopt;
}

const wchar_t* FanController::ToString(FanMode mode)
{
	switch (mode)
	{
	case FanMode::Auto:
		return L"Auto";

	case FanMode::Max:
		return L"Max";

	case FanMode::Quiet:
		return L"Quiet";

	case FanMode::MaxQ:
		return L"Max-Q";

	case FanMode::Custom:
		return L"Custom";
	}

	return L"Unknown";
}
