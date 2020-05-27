#define _CRT_SECURE_NO_WARNINGS
// MaxEntropyDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "MaxEntropy.h"
#include "MaxEntropyDlg.h"
#include "afxdialogex.h"

#include <math.h>
#include <iostream>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DOTS(x,y) (xp*((x)-xmin)),(yp*((y)-ymax))							// макрос перевода координат. график сигнала
#define DOTS_IMP(x,y) (xp_imp*((x)-xmin_imp)),(yp_imp*((y)-ymax_imp))		// макрос перевода координат. график импульсной хар-ки
#define DOTS_SVRK(x,y) (xp_svrk*((x)-xmin_svrk)),(yp_svrk*((y)-ymax_svrk))	// макрос перевода координат. график свертки

using namespace std;

// Диалоговое окно CMaxEntropyDlg

void CMaxEntropyDlg::Mashtab(double arr[], int dim, double* mmin, double* mmax)		//определяем функцию масштабирования
{
	*mmin = *mmax = arr[0];

	for (int i = 0; i < dim; i++)
	{
		if (*mmin > arr[i]) *mmin = arr[i];
		if (*mmax < arr[i]) *mmax = arr[i];
	}
}

CMaxEntropyDlg::CMaxEntropyDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MAXENTROPY_DIALOG, pParent)
	, Length(50)
	, amplitude_1(4)
	, amplitude_2(2)
	, amplitude_3(3.5)
	, dispersion_1(4)
	, dispersion_2(3)
	, dispersion_3(5)
	, center_pos_1(8)
	, center_pos_2(30)
	, center_pos_3(40)
	, dispersion_imp(4)
	, precision(1.e-6)
	, anim_time(200)
	, energ_noise(0)
	, SignalFlag(false)
	, ImpulseFlag(false)
	, SvertkaFlag(false)
	, DeconvFlag(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMaxEntropyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LENGTH, Length);
	DDX_Text(pDX, IDC_AMPL_1, amplitude_1);
	DDX_Text(pDX, IDC_AMPL_2, amplitude_2);
	DDX_Text(pDX, IDC_AMPL_3, amplitude_3);
	DDX_Text(pDX, IDC_DISP_1, dispersion_1);
	DDX_Text(pDX, IDC_DISP_2, dispersion_2);
	DDX_Text(pDX, IDC_DISP_3, dispersion_3);
	DDX_Text(pDX, IDC_CENTER_1, center_pos_1);
	DDX_Text(pDX, IDC_CENTER_2, center_pos_2);
	DDX_Text(pDX, IDC_CENTER_3, center_pos_3);
	DDX_Text(pDX, IDC_DISP_IMP, dispersion_imp);
	DDX_Text(pDX, IDC_EDIT_PRECISION, precision);
	DDX_Text(pDX, IDC_ANIM_TIME, anim_time);
	DDX_Text(pDX, IDC_EDIT_NOISE, energ_noise);
	DDX_Control(pDX, IDC_EDIT_ERROR, error);
	DDX_Control(pDX, IDC_DRAW, StartStopOptimization);
}

BEGIN_MESSAGE_MAP(CMaxEntropyDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DRAW, &CMaxEntropyDlg::OnBnClickedDraw)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_DRAW_SIGNAL, &CMaxEntropyDlg::OnBnClickedDrawSignal)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(ID_BUTTON_DROP_ANIM, &CMaxEntropyDlg::OnBnClickedButtonDropAnim)
END_MESSAGE_MAP()


// Обработчики сообщений CMaxEntropyDlg

BOOL CMaxEntropyDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	PicWnd = GetDlgItem(IDC_SIGNAL);			//связываем с ID окон
	PicDc = PicWnd->GetDC();
	PicWnd->GetClientRect(&Pic);

	PicWndImp = GetDlgItem(IDC_IMPULSE);
	PicDcImp = PicWndImp->GetDC();
	PicWndImp->GetClientRect(&PicImp);

	PicWndSvrk = GetDlgItem(IDC_SVERTKA);
	PicDcSvrk = PicWndSvrk->GetDC();
	PicWndSvrk->GetClientRect(&PicSvrk);

	// перья
	setka_pen.CreatePen(		//для сетки
		PS_DOT,					//пунктирная
		1,						//толщина 1 пиксель
		RGB(0, 0, 0));			//цвет  черный

	osi_pen.CreatePen(			//координатные оси
		PS_SOLID,				//сплошная линия
		3,						//толщина 2 пикселя
		RGB(0, 0, 0));			//цвет черный

	signal_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(0, 0, 255));			//цвет синий

	impulse_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(255, 0, 0));			//цвет красный

	svertka_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(0, 0, 255));			//цвет синий

	vosstanovl_pen.CreatePen(			//график
		PS_SOLID,				//сплошная линия
		-1,						//толщина -1 пикселя
		RGB(205, 55, 50));			//цвет красный

	UpdateData(false);
	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CMaxEntropyDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного

	CDialog::OnSysCommand(nID, lParam);

}
// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CMaxEntropyDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		UpdateData(false);

		if (SignalFlag == true)
		{
			Graph1(sign, PicDc, Pic, &signal_pen, Length);
		}
		if (ImpulseFlag == true)
		{
			Graph1(imp, PicDcImp, PicImp, &impulse_pen, Length);
		}
		if (SvertkaFlag == true)
		{
			Graph1(svert, PicDcSvrk, PicSvrk, &svertka_pen, Length);
		}
		if (DeconvFlag == true)
		{
			Graph2(sign, &signal_pen, Deconv, &vosstanovl_pen, PicDc, Pic, Length);
		}
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CMaxEntropyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD WINAPI MyProc(PVOID pv)
{
	CMaxEntropyDlg* p = (CMaxEntropyDlg*)pv;
	p->MHJ(p->Length, p->lambda);
	return 0;
}

void CMaxEntropyDlg::Graph1(double* Mass, CDC* WinDc, CRect WinPic, CPen* graphpen, double AbsMax)
{
	// поиск максимального и минимального значения
	Min = Mass[0];
	Max = Mass[0];
	for (int i = 1; i < Length; i++)
	{
		if (Mass[i] < Min)
		{
			Min = Mass[i];
		}
		if (Mass[i] > Max)
		{
			Max = Mass[i];
		}
	}
	// отрисовка
	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(WinDc);
	bmp.CreateCompatibleBitmap(WinDc, WinPic.Width(), WinPic.Height());
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(WinPic, RGB(255, 255, 255));
	// отрисовка сетки координат
	MemDc->SelectObject(&setka_pen);
	// вертикальные линии сетки координат
	for (double i = 0; i < WinPic.Width(); i += WinPic.Width() / 6)
	{
		MemDc->MoveTo(i, 0);
		MemDc->LineTo(i, WinPic.Height());
	}
	// горизонтальные линии сетки координат
	for (double i = WinPic.Height() / 10; i < WinPic.Height(); i += WinPic.Height() / 5)
	{
		MemDc->MoveTo(0, i);
		MemDc->LineTo(WinPic.Width(), i);
	}
	// отрисовка осей
	MemDc->SelectObject(&osi_pen);
	// отрисовка оси X
	//создаём Ось Y
	MemDc->MoveTo(0, WinPic.Height() * 9 / 10); MemDc->LineTo(WinPic.Width(), WinPic.Height() * 9 / 10);
	// отрисовка оси Y
	MemDc->MoveTo(WinPic.Width() * 1 / 15, WinPic.Height()); MemDc->LineTo(WinPic.Width() * 1 / 15, 0);
	// установка прозрачного фона текста
	MemDc->SetBkMode(TRANSPARENT);
	// установка шрифта
	CFont font;
	font.CreateFontW(14.5, 0, 0, 0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
	MemDc->SelectObject(&font);
	// подпись оси X
	MemDc->TextOut(WinPic.Width() * 14 / 15 + 4, WinPic.Height() * 9 / 10 + 2, CString("t"));
	// подпись оси Y
	MemDc->TextOut(WinPic.Width() * 1 / 15 + 10, 0, CString("A"));
	// выбор области для рисования
	xx0 = WinPic.Width() * 1 / 15; xxmax = WinPic.Width();
	yy0 = WinPic.Height() / 10; yymax = WinPic.Height() * 9 / 10;
	// отрисовка
	MemDc->SelectObject(graphpen);
	MemDc->MoveTo(xx0, yymax + (Mass[0] - Min) / (Max - Min) * (yy0 - yymax));
	for (int i = 0; i < Length; i++)
	{
		xxi = xx0 + (xxmax - xx0) * i / (Length - 1);
		yyi = yymax + (Mass[i] - Min) / (Max - Min) * (yy0 - yymax);
		MemDc->LineTo(xxi, yyi);
	}
	/* вывод числовых значений
	 по оси X*/
	MemDc->SelectObject(&font);
	for (int i = 1; i < 6; i++)
	{
		sprintf_s(znach, "%.1f", i * AbsMax / 6);
		MemDc->TextOut(i * (WinPic.Width() / 6) - AbsMax * 0.09, WinPic.Height() * 9 / 10 + 2, CString(znach));
	}
	// по оси Y
	for (int i = 1; i < 5; i++)
	{
		sprintf_s(znach, "%.2f", Min + i * (Max - Min) / 4);
		MemDc->TextOut(0 + AbsMax * 0.1, WinPic.Height() * (9 - 2 * i) / 10, CString(znach));
	}
	// вывод на экран
	WinDc->BitBlt(0, 0, WinPic.Width(), WinPic.Height(), MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

// отрисовка двух графиков
void CMaxEntropyDlg::Graph2(double* Mass1, CPen* graph1pen, double* Mass2, CPen* graph2pen, CDC* WinDc, CRect WinPic, double AbsMax)
{
	// поиск максимального и минимального значения
	Min1 = Mass1[0];
	Max1 = Mass1[0];
	Min2 = Mass2[0];
	Max2 = Mass2[0];
	for (int i = 1; i < Length; i++)
	{
		if (Mass1[i] < Min1)
		{
			Min1 = Mass1[i];
		}
		if (Mass1[i] > Max1)
		{
			Max1 = Mass1[i];
		}
		if (Mass2[i] < Min2)
		{
			Min2 = Mass2[i];
		}
		if (Mass2[i] > Max2)
		{
			Max2 = Mass2[i];
		}
	}
	if (Max2 > Max1)
	{
		Max = Max2;
	}
	else
	{
		Max = Max1;
	}
	if (Min2 < Min1)
	{
		Min = Min2;
	}
	else
	{
		Min = Min1;
	}
	// отрисовка
	// создание контекста устройства
	CBitmap bmp;
	CDC* MemDc;
	MemDc = new CDC;
	MemDc->CreateCompatibleDC(WinDc);
	bmp.CreateCompatibleBitmap(WinDc, WinPic.Width(), WinPic.Height());
	CBitmap* pBmp = (CBitmap*)MemDc->SelectObject(&bmp);
	// заливка фона графика белым цветом
	MemDc->FillSolidRect(WinPic, RGB(255, 255, 255));
	// отрисовка сетки координат
	MemDc->SelectObject(&setka_pen);
	// вертикальные линии сетки координат
	for (double i = 0; i < WinPic.Width(); i += WinPic.Width() / 6)
	{
		MemDc->MoveTo(i, 0);
		MemDc->LineTo(i, WinPic.Height());
	}
	// горизонтальные линии сетки координат
	for (double i = WinPic.Height() / 10; i < WinPic.Height(); i += WinPic.Height() / 5)
	{
		MemDc->MoveTo(0, i);
		MemDc->LineTo(WinPic.Width(), i);
	}
	// отрисовка осей
	MemDc->SelectObject(&osi_pen);
	// отрисовка оси X
	MemDc->MoveTo(0, WinPic.Height() * 9 / 10); MemDc->LineTo(WinPic.Width(), WinPic.Height() * 9 / 10);
	// отрисовка оси Y
	MemDc->MoveTo(WinPic.Width() * 1 / 15, WinPic.Height()); MemDc->LineTo(WinPic.Width() * 1 / 15, 0);
	// установка прозрачного фона текста
	MemDc->SetBkMode(TRANSPARENT);
	// установка шрифта
	CFont font;
	font.CreateFontW(14.5, 0, 0, 0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Century Gothic"));
	MemDc->SelectObject(&font);
	// подпись оси X
	MemDc->TextOut(WinPic.Width() * 14 / 15 + 4, WinPic.Height() * 9 / 10 + 2, CString("t"));
	// подпись оси Y
	MemDc->TextOut(WinPic.Width() * 1 / 15 + 10, 0, CString("A"));
	// выбор области для рисования
	xx0 = WinPic.Width() * 1 / 15; xxmax = WinPic.Width();
	yy0 = WinPic.Height() / 10; yymax = WinPic.Height() * 9 / 10;
	// отрисовка первого графика
	MemDc->SelectObject(graph1pen);
	MemDc->MoveTo(xx0, yymax + (Mass1[0] - Min) / (Max - Min) * (yy0 - yymax));
	for (int i = 0; i < Length; i++)
	{
		xxi = xx0 + (xxmax - xx0) * i / (Length - 1);
		yyi = yymax + (Mass1[i] - Min) / (Max - Min) * (yy0 - yymax);
		MemDc->LineTo(xxi, yyi);
	}
	// отрисовка второго графика
	MemDc->SelectObject(graph2pen);
	MemDc->MoveTo(xx0, yymax + (Mass2[0] - Min) / (Max - Min) * (yy0 - yymax));
	for (int i = 0; i < Length; i++)
	{
		xxi = xx0 + (xxmax - xx0) * i / (Length - 1);
		yyi = yymax + (Mass2[i] - Min) / (Max - Min) * (yy0 - yymax);
		MemDc->LineTo(xxi, yyi);
	}
	// вывод числовых значений
	// по оси X
	MemDc->SelectObject(&font);
	for (int i = 1; i < 6; i++)
	{
		sprintf_s(znach, "%.1f", i * AbsMax / 6);
		MemDc->TextOut(i * (WinPic.Width() / 6) - AbsMax * 0.09, WinPic.Height() * 9 / 10 + 2, CString(znach));
	}
	// по оси Y
	for (int i = 1; i < 5; i++)
	{
		sprintf_s(znach, "%.2f", Min + i * (Max - Min) / 4);
		MemDc->TextOut(0 + AbsMax * 0.1, WinPic.Height() * (9 - 2 * i) / 10, CString(znach));
	}
	// вывод на экран
	WinDc->BitBlt(0, 0, WinPic.Width(), WinPic.Height(), MemDc, 0, 0, SRCCOPY);
	delete MemDc;
}

double CMaxEntropyDlg::function(float* x)		//функционал для МХД
{
	double buff1 = 0;
	for (int i = 0; i < Length; i++)
	{
		buff1 = 0;
		for (int k = 0; k < Length; k++)
		{
			if (i - k < 0)
			{
				buff1 += x[k] * imp[Length + i - k];
			}
			else
			{
				buff1 += x[k] * imp[i - k];
			}
		}
		Deconv[i] = exp(-1 - buff1);
	}

	double value = 0;
	double buff2 = 0;
	for (int i = 0; i < Length; i++)
	{
		buff2 = 0;
		for (int k = 0; k < Length; k++)
		{
			if (i - k < 0)
			{
				buff2 += Deconv[k] * imp[Length + i - k];
			}
			else
			{
				buff2 += Deconv[k] * imp[i - k];
			}
		}
		value += (svert[i] - buff2) * (svert[i] - buff2);
	}
	return value;
}

double CMaxEntropyDlg::MHJ(int kk, float* x)
{
	// kk - количество параметров; x - массив параметров
	double  TAU = precision; // Точность вычислений 
	int i, j, bs, ps;
	double z, h, k, fi, fb;
	double* b = new double[kk];
	double* y = new double[kk];
	double* p = new double[kk];
	h = 1;
	x[0] = 1;
	for (i = 1; i < kk; i++)
	{
		x[i] = (double)rand() / RAND_MAX; // Задается начальное приближение
	}

	k = h;
	for (i = 0; i < kk; i++)
	{
		y[i] = p[i] = b[i] = x[i];
	}
	fi = function(x);
	ps = 0;
	bs = 1;
	fb = fi;
	j = 0;
	while (1)
	{
		x[j] = y[j] + k;
		z = function(x);
		sprintf_s(err, "%.10f", z);
		error.SetWindowTextW((CString)err);
		if (z >= fi)
		{
			x[j] = y[j] - k;
			z = function(x);
			if (z < fi)
			{
				y[j] = x[j];
			}
			else  x[j] = y[j];
		}
		else  y[j] = x[j];
		fi = function(x);
		if (j < kk - 1)
		{
			j++;
			continue;
		}
		if (fi + 1e-8 >= fb)
		{
			if (ps == 1 && bs == 0)
			{
				for (i = 0; i < kk; i++)
				{
					p[i] = y[i] = x[i] = b[i];
				}
				z = function(x);
				bs = 1;
				ps = 0;
				fi = z;
				fb = z;
				j = 0;
				continue;
			}
			k /= 10.;
			if (k < TAU)
			{
				break;
			}
			j = 0;
			continue;
		}
		for (i = 0; i < kk; i++)
		{
			p[i] = 2 * y[i] - b[i];
			b[i] = y[i];
			x[i] = p[i];
			y[i] = x[i];
		}
		z = function(x);
		fb = fi;
		ps = 1;
		bs = 0;
		fi = z;
		j = 0;
		Invalidate(0);

		Sleep(anim_time);
	} //  end of while(1)
	for (i = 0; i < kk; i++)
	{
		x[i] = p[i];
	}
	delete b;
	delete y;
	delete p;
	return fb;
}

double CMaxEntropyDlg::s(int t)
{
	double A_mas[] = { amplitude_1, amplitude_2, amplitude_3 };
	double dispersion_mas[] = { dispersion_1, dispersion_2, dispersion_3 };
	double center_pos_mas[] = { center_pos_1, center_pos_2, center_pos_3 };
	double result = 0;
	for (int i = 0; i < 3; i++)
	{
		result += A_mas[i] * exp(-((t - center_pos_mas[i]) / dispersion_mas[i]) * ((t - center_pos_mas[i]) / dispersion_mas[i]));
	}
	return result;
}

double CMaxEntropyDlg::Psi()		//рандомизация для шума
{
	float r = 0;
	for (int i = 1; i <= 12; i++)
	{
		r += ((rand() % 100) / (100 * 1.0) * 2) - 1;		// [-1;1]
	}
	return r / 12;
}

void CMaxEntropyDlg::OnBnClickedDrawSignal()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(TRUE);
	SignalFlag = true;
	ImpulseFlag = true;
	SvertkaFlag = true;

	sign = new double[Length];
	imp = new double[Length];
	svert = new double[Length];

	double* signal = new double[Length];
	double* impulse = new double[Length];
	double* impulse_norm = new double[Length];
	double* svertka = new double[Length];
	double* mas_psi = new double[Length];
	double* mas_shum = new double[Length];
	for (int i = 0; i < Length; i++)
	{
		sign[i] = 0;
		imp[i] = 0;
		svert[i] = 0;
		signal[i] = 0;
		impulse[i] = 0;
		impulse_norm[i] = 0;
		svertka[i] = 0;
		mas_psi[i] = 0;
		mas_shum[i] = 0;
	}

	for (int i = 0; i < Length; i++)
	{
		signal[i] = s(i);
		sign[i] = signal[i];
	}

	///*Mashtab(signal, Length, &ymin, &ymax);
	//RedrawSignal(-0.15 * ymax, ymax);
	//PicDc->SelectObject(&signal_pen);
	//PicDc->MoveTo(DOTS(0, signal[0]));*/

	//for (int i = 0; i < xmax; i++)
	//{
	//	PicDc->LineTo(DOTS(i, signal[i]));
	//}

	double sum_imp = 0;
	for (int i = 0; i < Length; i++)
	{
		impulse[i] = exp(-(i / dispersion_imp) * (i / dispersion_imp)) + exp(-((i - Length) / dispersion_imp) * ((i - Length) / dispersion_imp));
		sum_imp += impulse[i];
	}

	for (int i = 0; i < Length; i++)
	{
		impulse_norm[i] = impulse[i] / sum_imp;
		imp[i] = impulse_norm[i];
	}

	/*Mashtab(impulse_norm, Length, &ymin_imp, &ymax_imp);
	RedrawImp(-0.15 * ymax_imp, ymax_imp);
	PicDcImp->SelectObject(&impulse_pen);
	PicDcImp->MoveTo(DOTS_IMP(0, impulse_norm[0]));

	for (int i = 0; i < xmax_imp; i++)
	{
		PicDcImp->LineTo(DOTS_IMP(i, impulse_norm[i]));
	}*/

	int buff = 0;
	for (int k = 0; k < Length; k++)
	{
		for (int i = 0; i < Length; i++)
		{
			if (k - i < 0)
			{
				buff = Length + (k - i);
				svertka[k] += signal[i] * impulse_norm[buff];
			}
			else
			{
				svertka[k] += signal[i] * impulse_norm[k - i];
			}
		}
	}

	double d = energ_noise / 100;

	double mas_energsignal = 0;
	for (int t = 0; t < Length; t++)
	{
		mas_energsignal += svertka[t] * svertka[t];
	}

	for (int t = 0; t < Length; t++)
	{
		mas_psi[t] = Psi();
	}

	double qpsi = 0;
	for (int t = 0; t < Length; t++)
	{
		qpsi += mas_psi[t] * mas_psi[t];
	}

	double alpha = sqrt(d * mas_energsignal / qpsi);

	for (int i = 0; i < Length; i++)
	{
		mas_shum[i] = svertka[i] + alpha * mas_psi[i];
	}

	for (int i = 0; i < Length; i++)
	{
		svert[i] = mas_shum[i];
	}

	/*Mashtab(svert, Length, &ymin_svrk, &ymax_svrk);
	RedrawSvrk(-0.15 * ymax_svrk, ymax_svrk);
	PicDcSvrk->SelectObject(&svertka_pen);
	PicDcSvrk->MoveTo(DOTS_SVRK(0, svert[0]));

	for (int i = 0; i < xmax_svrk; i++)
	{
		PicDcSvrk->LineTo(DOTS_SVRK(i, svert[i]));
	}*/

	Graph1(sign, PicDc, Pic, &signal_pen, Length);
	Graph1(imp, PicDcImp, PicImp, &impulse_pen, Length);
	Graph1(svert, PicDcSvrk, PicSvrk, &svertka_pen, Length);

	delete[] signal;
	delete[] impulse;
	delete[] impulse_norm;
	delete[] svertka;
}

void CMaxEntropyDlg::OnBnClickedDraw()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(TRUE);
	SignalFlag = false;
	ImpulseFlag = false;
	SvertkaFlag = false;

	if (!bRunTh)
	{
		StartStopOptimization.SetWindowTextW(stop);
		if (hThread == NULL)
		{
			lambda = new float[Length];
			Deconv = new double[Length];

			for (int i = 0; i < Length; i++)
			{
				lambda[i] = 0;
				Deconv[i] = 0;
			}

			hThread = CreateThread(NULL, 0, MyProc, this, 0, &dwThread);
			DeconvFlag = true;
		}
		else
		{
			ResumeThread(hThread);
		}
		bRunTh = true;
	}
	else
	{
		StartStopOptimization.SetWindowTextW(start);
		bRunTh = false;

		SuspendThread(hThread);
	}
}


void CMaxEntropyDlg::OnBnClickedButtonDropAnim()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(TRUE);
	TerminateThread(hThread, 0);		//убиваем поток
	CloseHandle(hThread);
	hThread = NULL;

	StartStopOptimization.SetWindowTextW(start);
	bRunTh = false;

	DeconvFlag = false;
	Graph1(sign, PicDc, Pic, &signal_pen, Length);
	Graph1(imp, PicDcImp, PicImp, &impulse_pen, Length);
	Graph1(svert, PicDcSvrk, PicSvrk, &svertka_pen, Length);

	error.SetWindowTextW(L"");
}
