
// MaxEntropy.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CMaxEntropyApp:
// Сведения о реализации этого класса: MaxEntropy.cpp
//

class CMaxEntropyApp : public CWinApp
{
public:
	CMaxEntropyApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CMaxEntropyApp theApp;
