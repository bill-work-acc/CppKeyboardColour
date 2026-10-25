// Created by DeviceIoControl

#include "stdafx.h"
#include "Console.h"
#include "ConsoleLogger.h"
#include "Translators/TranslatorFactory.h"

Console::Console(LogLevel level)
	: m_baseLoglevel(level)
{
	TranslatorFactory factory{};
	SetConsoleOutputCP(CP_UTF16);

	m_hWnd = GetConsoleWindow();
	m_pTranslator = factory.Create(GetUserDefaultLangID());
	m_pLogger = std::make_unique<ConsoleLogger>();
}

void Console::SetLogLevel(LogLevel level)
{
	m_baseLoglevel = level;
}

void Console::SetTitle(const std::wstring& title) 
{
	const auto translatedTitle = m_pTranslator->Translate(title).append(L"\n");
	SetConsoleTitleW(translatedTitle.c_str());

	m_pLogger->Write(LogLevel::None, translatedTitle);
}

void Console::WriteLine(const std::wstring& message)
{
	m_pLogger->Write(LogLevel::None, message);
}

void Console::Hide() 
{
	ShowWindow(m_hWnd, SW_HIDE);
}

void Console::Show() 
{
	ShowWindow(m_hWnd, SW_SHOW);
}

bool Console::IsVisible() const
{
	return !!(GetWindowLongPtrW(m_hWnd, GWL_STYLE) & WS_VISIBLE);
}

void Console::OnFatalMessage()
{
	std::printf("\n%s\n", "Press ENTER to exit.");
	std::getchar();
	std::quick_exit(GetLastError());
}

bool Console::CanLogOnLevel(LogLevel level) const
{
	return (level >= m_baseLoglevel) ? true : false;
}

SYSTEMTIME Console::GetCurrentSystemTime() const
{
	SYSTEMTIME currentSysTime = { 0 };
	GetSystemTime(&currentSysTime);
	return currentSysTime;
}
