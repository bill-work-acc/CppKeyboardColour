// Created by DeviceIoControl

#include "stdafx.h"
#include "InsydeDeviceChannel.h"
#include "../Console/Logger.h"

InsydeDeviceChannel::InsydeDeviceChannel(KeyboardType kbType)
	: m_kbType(kbType)
{
	if (!m_dchu.IsLoaded())
	{
		KB_SET_FATAL_ERROR(STATUS_DLL_NOT_FOUND);
		KB_FATAL(L"Cannot load %s. Please ensure the DLL is within the same directory!", kInsydeDchuDll);
	}
}

bool InsydeDeviceChannel::SendCode(uint32_t code)
{
	if (!m_dchu.IsLoaded())
	{
		return false;
	}

	// The keyboard code packs the zone in bits 31..24 and the colour as B8R8G8.
	const auto colourValue = m_colourFactory.Convert(ColourFormat::B8R8G8, code & 0x00ffffff, ColourFormat::R8G8B8);
	const auto colour = m_colourFactory.Create(colourValue);

	KB_DEBUG(L"Insyde keyboard code=0x%08X, colour R=%u G=%u B=%u",
		code,
		static_cast<unsigned>(colour[INDEX_COLOUR_RED]),
		static_cast<unsigned>(colour[INDEX_COLOUR_GREEN]),
		static_cast<unsigned>(colour[INDEX_COLOUR_BLUE]));

	// Found in CLEVO Control Center v6.053.
	// Persist the colour and mode first so the OEM tools restore them, then
	// apply the colour to the EC last so it is the final visible state.
	const uint8_t mode = 8;

	if (!m_dchu.WriteAppSettings(2, 0x51, static_cast<uint32_t>(colour.size()), colour.data()))
	{
		KB_ERROR(L"Failed to write the keyboard colour to the OEM app settings.");
		return false;
	}

	if (!m_dchu.WriteAppSettings(2, 0x20, 1, &mode))
	{
		KB_ERROR(L"Failed to write the keyboard mode to the OEM app settings.");
		return false;
	}

	return this->WriteKeyboardColour(code);
}

bool InsydeDeviceChannel::WriteKeyboardColour(uint32_t code)
{
	// 0x67 encodes a sub-command in bits 31..28 (Local7) and a sub-argument in
	// bits 27..24 (Local4). Single-zone panels still use the multi-colour
	// sequence path (Local7 = 0xF): slots 0..2 are written via Local4 = 0..2,
	// and the final slot switches the EC into the static sequence mode that
	// actually displays the colour. Writing slot 0 alone only loads the EC
	// register and leaves the panel unchanged.
	const auto slotCount = (m_kbType == KeyboardType::SINGLE_ZONE) ? 3u : 1u;

	for (uint32_t slot = 0; slot < slotCount; ++slot)
	{
		const auto slotCode = (code & 0x00fffffful) | ((0xF0ul | slot) << 24ul);

		KB_DEBUG(L"Writing keyboard colour slot %u (code=0x%08X)", slot, static_cast<unsigned>(slotCode));

		if (!m_dchu.SetData(0x67, xstd::adjust_ptr<const uint8_t>(&slotCode, 0), static_cast<uint32_t>(sizeof(slotCode))))
		{
			KB_ERROR(L"Failed to write the keyboard colour to the EC.");
			return false;
		}
	}

	return true;
}

DeviceChannelType InsydeDeviceChannel::QueryType() const
{
	return DeviceChannelType::Insyde;
}
