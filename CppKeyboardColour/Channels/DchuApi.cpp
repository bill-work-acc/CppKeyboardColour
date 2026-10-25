// Created by DeviceIoControl

#include "stdafx.h"
#include "DchuApi.h"
#include "../Console/Logger.h"
#include <cstring>

namespace
{
	constexpr uint32_t kDchuCommandBufferSize = 0x400;
	// GetDCHU_Data_Buffer writes the package data plus one extra byte (the DLL
	// appends the first data byte after the copy loop). Keep this larger than
	// the 0x400-byte DCHU output buffer so the extra write stays in bounds.
	constexpr size_t kDchuDataBufferSize = 0x800;
	constexpr uint32_t kAppSettingsSize = 0x1000;
	constexpr uint32_t kAppSettingsPageSize = 0x100;
}

DchuApi::DchuApi()
{
	m_hModule = LoadLibraryW(kInsydeDchuDll);

	if (!IS_HANDLE_VALID(m_hModule))
	{
		m_hModule = nullptr;
		return;
	}

	m_pfnSetData = reinterpret_cast<T_SetDCHU_Data>(GetProcAddress(m_hModule, "SetDCHU_Data"));
	m_pfnGetDataBuffer = reinterpret_cast<T_GetDCHU_Data_Buffer>(GetProcAddress(m_hModule, "GetDCHU_Data_Buffer"));
	m_pfnGetDataInteger = reinterpret_cast<T_GetDCHU_Data_Integer>(GetProcAddress(m_hModule, "GetDCHU_Data_Integer"));
	m_pfnReadAppSettings = reinterpret_cast<T_ReadAppSettings>(GetProcAddress(m_hModule, "ReadAppSettings"));
	m_pfnWriteAppSettings = reinterpret_cast<T_WriteAppSettings>(GetProcAddress(m_hModule, "WriteAppSettings"));

	if (!m_pfnSetData || !m_pfnGetDataBuffer || !m_pfnGetDataInteger || !m_pfnReadAppSettings || !m_pfnWriteAppSettings)
	{
		KB_ERROR(L"%s is missing one or more required exports.", kInsydeDchuDll);
		FreeLibrary(m_hModule);
		m_hModule = nullptr;
		ResetFunctionPointers();
		return;
	}

	wchar_t modulePath[MAX_PATH]{};
	GetModuleFileNameW(m_hModule, modulePath, MAX_PATH);
	KB_DEBUG(L"Loaded %s from %s", kInsydeDchuDll, modulePath);
}

DchuApi::~DchuApi()
{
	if (IS_HANDLE_VALID(m_hModule))
	{
		FreeLibrary(m_hModule);
	}

	ResetFunctionPointers();
}

void DchuApi::ResetFunctionPointers()
{
	m_pfnSetData = nullptr;
	m_pfnGetDataBuffer = nullptr;
	m_pfnGetDataInteger = nullptr;
	m_pfnReadAppSettings = nullptr;
	m_pfnWriteAppSettings = nullptr;
}

bool DchuApi::SetData(uint32_t command, const uint8_t* buffer, uint32_t length) const
{
	if (!m_pfnSetData || (!buffer && length > 0) || length > kDchuCommandBufferSize)
	{
		return false;
	}

	// The DLL returns the ACPI output value, which is not a reliable success code.
	// The call itself is considered successful once the export has been resolved.
	const auto result = m_pfnSetData(command, buffer, length);
	KB_DEBUG(L"SetDCHU_Data(command=0x%02X, length=%u) returned 0x%08X", command, length, result);
	return true;
}

bool DchuApi::GetDataBuffer(uint32_t command, uint8_t* buffer, size_t bufferSize) const
{
	if (!m_pfnGetDataBuffer || !buffer || bufferSize < kDchuDataBufferSize)
	{
		return false;
	}

	std::memset(buffer, 0, kDchuDataBufferSize);

	// On success the DLL returns the command that was read.
	return m_pfnGetDataBuffer(command, buffer) != 0;
}

bool DchuApi::GetDataInteger(uint32_t command, uint32_t& value) const
{
	if (!m_pfnGetDataInteger)
	{
		return false;
	}

	DWORD data = 0;
	m_pfnGetDataInteger(command, &data);
	value = data;

	return true;
}

bool DchuApi::ReadAppSettings(uint32_t page, uint32_t offset, uint32_t length, uint8_t* buffer, size_t bufferSize) const
{
	if (!m_pfnReadAppSettings || !buffer || length > bufferSize)
	{
		return false;
	}

	if (page >= (kAppSettingsSize / kAppSettingsPageSize) || offset >= kAppSettingsPageSize || offset + length > kAppSettingsPageSize)
	{
		return false;
	}

	std::memset(buffer, 0, length);

	// On success the DLL returns the size of the app-settings area (0x1000).
	return m_pfnReadAppSettings(page, offset, length, buffer) != 0;
}

bool DchuApi::WriteAppSettings(uint32_t page, uint32_t offset, uint32_t length, const uint8_t* buffer) const
{
	if (!m_pfnWriteAppSettings || !buffer)
	{
		return false;
	}

	if (page >= (kAppSettingsSize / kAppSettingsPageSize) || offset >= kAppSettingsPageSize || offset + length > kAppSettingsPageSize)
	{
		return false;
	}

	const auto result = m_pfnWriteAppSettings(page, offset, length, buffer);
	KB_DEBUG(L"WriteAppSettings(page=%u, offset=0x%02X, length=%u) returned %u", page, offset, length, result);
	return result != 0;
}
