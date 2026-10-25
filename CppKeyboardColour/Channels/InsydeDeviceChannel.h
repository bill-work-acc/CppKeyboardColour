// Created by DeviceIoControl

#pragma once

#include "IDeviceChannel.h"
#include "../ColourFactory.h"
#include "../Common/KeyboardType.h"
#include "DchuApi.h"

class InsydeDeviceChannel
	: public IDeviceChannel
{
public:
	explicit InsydeDeviceChannel(KeyboardType kbType);
	~InsydeDeviceChannel() override = default;

	bool SendCode(uint32_t code) override;
	DeviceChannelType QueryType() const override;

private:
	KeyboardType m_kbType;
	ColourFactory m_colourFactory{};
	DchuApi m_dchu{};

	bool WriteKeyboardColour(uint32_t code);
};
