// Created by DeviceIoControl

#pragma once
#include "IModelIdRetriever.h"

namespace Detail 
{
#if defined(_M_X64) || defined(_M_ARM64)
	using T_GetProductID_PCI = DWORD(*)();
#else
	using T_GetProductID_PCI = DWORD(__stdcall*)();
#endif
} // namespace Detail

class LegacyModelIdRetriever 
	: public IModelIdRetriever
{
public:
	LegacyModelIdRetriever(bool useDebugModel = false);

	uint32_t GetModelID() override;

	~LegacyModelIdRetriever() override;

private:
	bool m_useDebugModel = false;
	HMODULE m_hGetProductDLL = nullptr;
	Detail::T_GetProductID_PCI m_pfnGetProductID = nullptr;

	// NOTE: GetProductID64 (A.K.A GetProductdll.dll) is buggy and unintializes COM multiples times,
	// even though only one call to CoInitialize succeeds.
	static uint32_t GetProductIDWorker(Detail::T_GetProductID_PCI fnGetProductID);
};