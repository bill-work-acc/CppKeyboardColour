// Created by DeviceIoControl

#pragma once

#include <cstddef>
#include <cstdint>

inline constexpr wchar_t kInsydeDchuDll[] = L"InsydeDCHU.dll";

// Thin wrapper around the generic InsydeDCHU.dll exports.
// The DLL forwards commands to the Insyde ACPI _DSM interface; the individual
// commands/pages are defined by the firmware and the OEM application.
class DchuApi
{
private:
	using T_SetDCHU_Data = DWORD(__stdcall*)(DWORD command, const UINT8* buffer, DWORD length);
	using T_GetDCHU_Data_Buffer = DWORD(__stdcall*)(DWORD command, UINT8* buffer);
	using T_GetDCHU_Data_Integer = DWORD(__stdcall*)(DWORD command, DWORD* data);
	using T_ReadAppSettings = DWORD(__stdcall*)(DWORD page, DWORD offset, DWORD length, UINT8* buffer);
	using T_WriteAppSettings = DWORD(__stdcall*)(DWORD page, DWORD offset, DWORD length, const UINT8* buffer);

public:
	DchuApi();
	~DchuApi();

	DchuApi(const DchuApi&) = delete;
	DchuApi& operator=(const DchuApi&) = delete;

	bool IsLoaded() const { return m_hModule != nullptr; }

	bool SetData(uint32_t command, const uint8_t* buffer, uint32_t length) const;

	// bufferSize must be at least 0x800: the DLL writes the package data plus
	// one extra byte, and the DCHU output buffer itself is 0x400 bytes.
	bool GetDataBuffer(uint32_t command, uint8_t* buffer, size_t bufferSize) const;
	bool GetDataInteger(uint32_t command, uint32_t& value) const;
	bool ReadAppSettings(uint32_t page, uint32_t offset, uint32_t length, uint8_t* buffer, size_t bufferSize) const;
	bool WriteAppSettings(uint32_t page, uint32_t offset, uint32_t length, const uint8_t* buffer) const;

private:
	void ResetFunctionPointers();

	HMODULE m_hModule = nullptr;
	T_SetDCHU_Data m_pfnSetData = nullptr;
	T_GetDCHU_Data_Buffer m_pfnGetDataBuffer = nullptr;
	T_GetDCHU_Data_Integer m_pfnGetDataInteger = nullptr;
	T_ReadAppSettings m_pfnReadAppSettings = nullptr;
	T_WriteAppSettings m_pfnWriteAppSettings = nullptr;
};
