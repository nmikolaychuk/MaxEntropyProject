
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

void CMaxEntropyDlg::Mashtab(double arr[], int dim, double *mmin, double *mmax)		//определяем функцию масштабирования
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
	, err_value(_T(""))
	, precision(1.e-6)
	, anim_time(200)
	, energ_noise(0)
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
	DDX_Text(pDX, IDC_STATIC_ERR, err_value);
	DDX_Text(pDX, IDC_EDIT_PRECISION, precision);
	DDX_Text(pDX, IDC_ANIM_TIME, anim_time);
	DDX_Text(pDX, IDC_EDIT_NOISE, energ_noise);
}

BEGIN_MESSAGE_MAP(CMaxEntropyDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DRAW, &CMaxEntropyDlg::OnBnClickedDraw)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_DRAW_SIGNAL, &CMaxEntropyDlg::OnBnClickedDrawSignal)
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

		RedrawAll();			//функция перерисовки
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CMaxEntropyDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMaxEntropyDlg::RedrawAll()		//функция отрисовки/перерисовки
{
	double *signal = new double[Length];
	memset(signal, 0, Length * sizeof(double));

	for (int i = 0; i < Length; i++)
	{
		signal[i] = s(i);
	}

	double d = energ_noise / 100;

	double mas_energsignal = 0;
	for (int t = 0; t < Length; t++)
	{
		mas_energsignal += signal[t] * signal[t];
	}

	double *mas_psi = new double[Length];
	memset(mas_psi, 0, Length * sizeof(double));
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

	double *mas_shum = new double[Length];
	memset(mas_shum, 0, Length * sizeof(double));
	for (int i = 0; i < Length; i++)
	{
		mas_shum[i] = signal[i] + alpha * mas_psi[i];
	}

	double *impulse = new double[Length];
	memset(impulse, 0, Length * sizeof(double));

	double *impulse_norm = new double[Length];
	memset(impulse_norm, 0, Length * sizeof(double));

	double sum_imp = 0;

	for (int i = 0; i < Length; i++)
	{
		impulse[i] = exp(-(i / dispersion_imp)*(i / dispersion_imp)) + exp(-((i - Length) / dispersion_imp)*((i - Length) / dispersion_imp));
	}

	for (int i = 0; i < Length; i++)
	{
		sum_imp += impulse[i];
	}

	for (int i = 0; i < Length; i++)
	{
		impulse_norm[i] = impulse[i] / sum_imp;
	}

	double *svertka = new double[Length];
	memset(svertka, 0, Length * sizeof(double));

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

	PicDc->FillSolidRect(&Pic, RGB(250, 250, 250));			//закрашиваю фон 

	PicDc->SelectObject(&osi_pen);		//выбираем перо

	mn = 0; mx = 0; mn_imp = 0; mx_imp = 0; mn_svrk = 0; mx_svrk = 0;

	Mashtab(signal, Length, &mn, &mx);
	Mashtab(impulse_norm, Length, &mn_imp, &mx_imp);
	Mashtab(svertka, Length, &mn_svrk, &mx_svrk);


	//область построения
	xmin = 0;			//минимальное значение х
	xmax = Length;			//максимальное значение х
	ymin = mn - 0.18*mx;			//минимальное значение y
	ymax = mx * 1.2;		//максимальное значение y

	double window_signal_width = Pic.Width();
	double window_signal_height = Pic.Height();
	xp = (window_signal_width / (xmax - xmin));			//Коэффициенты пересчёта координат по Х
	yp = -(window_signal_height / (ymax - ymin));			//Коэффициенты пересчёта координат по У

	//создаём Ось Y
	PicDc->MoveTo(DOTS(0, ymax));
	PicDc->LineTo(DOTS(0, ymin));
	//создаём Ось Х
	PicDc->MoveTo(DOTS(xmin, 0));
	PicDc->LineTo(DOTS(xmax, 0));

	PicDc->SelectObject(&setka_pen);

	//отрисовка сетки по y
	for (double x = 0; x <= xmax; x += Length / 5)
	{
		PicDc->MoveTo(DOTS(x, ymax));
		PicDc->LineTo(DOTS(x, ymin));
	}
	//отрисовка сетки по x
	for (double y = 0; y <= ymax; y += ymax / 5)
	{
		PicDc->MoveTo(DOTS(xmin, y));
		PicDc->LineTo(DOTS(xmax, y));
	}


	//подпись точек на оси
	CFont font;
	font.CreateFontW(16, 0, 0, 0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Times New Roman"));
	PicDc->SelectObject(font);

	//подпись осей
	PicDc->TextOutW(DOTS(Length / 10, ymax - 0.2), _T("X")); //Y
	PicDc->TextOutW(DOTS(xmax - 2, 0 + 0.8), _T("t")); //X

	//по Y с шагом 5
	for (double i = 0; i <= ymax; i += ymax / 5)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.1f"), i);
			PicDc->TextOutW(DOTS(1, i + 0.25), str);
		}
	}
	//по X с шагом 0.5
	for (double j = 0; j <= xmax; j += Length / 5)
	{
		CString str;
		str.Format(_T("%.1f"), j);
		PicDc->TextOutW(DOTS(j - 1, -0.1), str);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	PicDcImp->FillSolidRect(&PicImp, RGB(250, 250, 250));			//закрашиваю фон 

	PicDcImp->SelectObject(&osi_pen);		//выбираем перо

	//область построения
	xmin_imp = 0;			//минимальное значение х
	xmax_imp = Length;			//максимальное значение х
	ymin_imp = mn_imp - 0.18*mx_imp;			//минимальное значение y
	ymax_imp = 1.2 * mx_imp;		//максимальное значение y

	double window_impulse_width = PicImp.Width();
	double window_impulse_height = PicImp.Height();
	xp_imp = (window_impulse_width / (xmax_imp - xmin_imp));			//Коэффициенты пересчёта координат по Х
	yp_imp = -(window_impulse_height / (ymax_imp - ymin_imp));			//Коэффициенты пересчёта координат по У

	//создаём Ось Y
	PicDcImp->MoveTo(DOTS_IMP(0, ymax_imp));
	PicDcImp->LineTo(DOTS_IMP(0, ymin_imp));
	//создаём Ось Х
	PicDcImp->MoveTo(DOTS_IMP(xmin_imp, 0));
	PicDcImp->LineTo(DOTS_IMP(xmax_imp, 0));

	PicDcImp->SelectObject(&setka_pen);

	//отрисовка сетки по y
	for (double x = 0; x <= xmax_imp; x += Length / 5)
	{
		PicDcImp->MoveTo(DOTS_IMP(x, ymax_imp));
		PicDcImp->LineTo(DOTS_IMP(x, ymin_imp));
	}
	//отрисовка сетки по x
	for (double y = 0; y <= ymax_imp; y += ymax_imp / 5)
	{
		PicDcImp->MoveTo(DOTS_IMP(xmin_imp, y));
		PicDcImp->LineTo(DOTS_IMP(xmax_imp, y));
	}


	//подпись точек на оси
	CFont font1;
	font1.CreateFontW(16, 0, 0, 0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Times New Roman"));
	PicDcImp->SelectObject(font1);

	//подпись осей
	PicDcImp->TextOutW(DOTS_IMP(Length / 8, ymax_imp - 0.006), _T("h")); //Y
	PicDcImp->TextOutW(DOTS_IMP(xmax_imp - 12, 0 - 0.1), _T("t")); //X

	//по Y с шагом 5
	for (double i = 0; i <= ymax_imp; i += ymax_imp / 5)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.3f"), i);
			PicDcImp->TextOutW(DOTS_IMP(1, i + 0.006), str);
		}
	}

	//по X с шагом 0.5
	for (double j = 0; j <= xmax_imp; j += Length / 5)
	{
		CString str;
		str.Format(_T("%.1f"), j);
		PicDcImp->TextOutW(DOTS_IMP(j - 1, -0.002), str);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PicDcSvrk->FillSolidRect(&PicSvrk, RGB(250, 250, 250));			//закрашиваю фон 

	PicDcSvrk->SelectObject(&osi_pen);		//выбираем перо

	//область построения
	xmin_svrk = 0;			//минимальное значение х
	xmax_svrk = Length;			//максимальное значение х
	ymin_svrk = mn_svrk - 0.2*mx_svrk;			//минимальное значение y
	ymax_svrk = 1.2 * mx_svrk;		//максимальное значение y

	double window_svrk_width = PicSvrk.Width();
	double window_svrk_height = PicSvrk.Height();
	xp_svrk = (window_svrk_width / (xmax_svrk - xmin_svrk));			//Коэффициенты пересчёта координат по Х
	yp_svrk = -(window_svrk_height / (ymax_svrk - ymin_svrk));			//Коэффициенты пересчёта координат по У

	//создаём Ось Y
	PicDcSvrk->MoveTo(DOTS_SVRK(0, ymax_svrk));
	PicDcSvrk->LineTo(DOTS_SVRK(0, ymin_svrk));
	//создаём Ось Х
	PicDcSvrk->MoveTo(DOTS_SVRK(xmin_svrk, 0));
	PicDcSvrk->LineTo(DOTS_SVRK(xmax_svrk, 0));

	PicDcSvrk->SelectObject(&setka_pen);

	//отрисовка сетки по y
	for (double x = 0; x <= xmax_svrk; x += Length / 5)
	{
		PicDcSvrk->MoveTo(DOTS_SVRK(x, ymax_svrk));
		PicDcSvrk->LineTo(DOTS_SVRK(x, ymin_svrk));
	}
	//отрисовка сетки по x
	for (double y = 0; y <= ymax_svrk; y += ymax_svrk / 5)
	{
		PicDcSvrk->MoveTo(DOTS_SVRK(xmin_svrk, y));
		PicDcSvrk->LineTo(DOTS_SVRK(xmax_svrk, y));
	}


	//подпись точек на оси
	CFont font2;
	font2.CreateFontW(16, 0, 0, 0, FW_REGULAR, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS || CLIP_LH_ANGLES, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Times New Roman"));
	PicDcSvrk->SelectObject(font2);

	//подпись осей
	PicDcSvrk->TextOutW(DOTS_SVRK(Length / 13, ymax_svrk - 0.2), _T("Y")); //Y
	PicDcSvrk->TextOutW(DOTS_SVRK(xmax_svrk - 2, 0.5), _T("t")); //X

	//по Y с шагом 5
	for (double i = 0; i <= ymax_svrk; i += ymax_svrk / 5)
	{
		CString str;
		if (i != 0)
		{
			str.Format(_T("%.1f"), i);
			PicDcSvrk->TextOutW(DOTS_SVRK(1, i + 0.1), str);
		}
	}
	//по X с шагом 0.5
	for (double j = 0; j <= xmax_svrk; j += Length / 5)
	{
		CString str;
		str.Format(_T("%.1f"), j);
		PicDcSvrk->TextOutW(DOTS_SVRK(j - 1, -0.1), str);
	}

	delete signal;
	delete mas_psi;
	delete mas_shum;
	delete impulse_norm;
	delete svertka;
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

double CMaxEntropyDlg::function(double*x)		//функционал для МХД
{
	double* x_imp = new double[Length];
	memset(x_imp, 0, Length * sizeof(double));

	int buff = 0;
	for (int i = 0; i < Length; i++)
	{
		for (int k = 0; k < Length; k++)
		{
			if (i - k < 0)
			{
				buff = Length + (i - k);
				x_imp[i] += x[k] * imp[buff];
			}
			else
			{
				x_imp[i] += x[k] * imp[i - k];
			}
		}
	}

	double* xi = new double[Length];
	memset(xi, 0, Length * sizeof(double));

	for (int i = 0; i < Length; i++)
	{
		xi[i] = exp(-1 - x_imp[i]);
	}

	double* yi = new double[Length];
	memset(yi, 0, Length * sizeof(double));

	int buff1 = 0;
	for (int i = 0; i < Length; i++)
	{
		for (int k = 0; k < Length; k++)
		{
			if (i - k < 0)
			{
				buff1 = Length + (i - k);
				yi[i] += xi[k] * imp[buff1];
			}
			else
			{
				yi[i] += xi[k] * imp[i - k];
			}
		}
	}

	double E = 0;

	for (int i = 0; i < Length; i++)
	{
		E += (svert[i] - yi[i]) * (svert[i] - yi[i]);
	}

	return E;

	delete xi;
	delete yi;
}

double CMaxEntropyDlg::MHJ(int kk, double* x)
{
	// kk - количество параметров; x - массив параметров
	UpdateData(TRUE);
	double  TAU = precision; // Точность вычислений
	int i, j, bs, ps;
	double z, h, k, fi, fb;
	calc = 0;
	double *b = new double[kk];
	double *y = new double[kk];
	double *p = new double[kk];

	h = 1.;
	x[0] = 1.;
	for (i = 1; i < kk; i++)  x[i] = (double)rand() / RAND_MAX; // Задается начальное приближение

	k = h;
	for (i = 0; i < kk; i++)	y[i] = p[i] = b[i] = x[i];
	fi = function(x);
	ps = 0;   bs = 1;  fb = fi;

	j = 0;
	while (1)
	{
		x[j] = y[j] + k;
		z = function(x);
		if (z >= fi) {
			x[j] = y[j] - k;
			z = function(x);
			if (z < fi)   y[j] = x[j];
			else  x[j] = y[j];
		}
		else  y[j] = x[j];
		fi = function(x);

		if (j < kk - 1) { j++;  continue; }
		if (fi + 1e-8 >= fb) {
			if (ps == 1 && bs == 0) {
				for (i = 0; i < kk; i++) {
					p[i] = y[i] = x[i] = b[i];
				}
				z = function(x);
				bs = 1;   ps = 0;   fi = z;   fb = z;   j = 0;
				continue;
			}
			k /= 10.;
			if (k < TAU) break;
			j = 0;					   
			continue;
		}

		for (i = 0; i < kk; i++) {
			p[i] = 2 * y[i] - b[i];
			b[i] = y[i];
			x[i] = p[i];

			y[i] = x[i];
		}
		z = function(x);
		fb = fi;   ps = 1;   bs = 0;   fi = z;   j = 0;

		calc++; // Счетчик итераций. Можно игнорировать

		//Анимация(отрисовка на каждой итерации цикла)

		int buff = 0;
		double* lam_imp = new double[Length];
		memset(lam_imp, 0, Length * sizeof(double));

		for (int i = 0; i < Length; i++)
		{
			for (int k = 0; k < Length; k++)
			{
				if (i - k < 0)
				{
					buff = Length + (i - k);
					lam_imp[i] += p[k] * imp[buff];
				}
				else
				{
					lam_imp[i] += p[k] * imp[i - k];
				}
			}
		}

		double* xi = new double[Length];
		memset(xi, 0, Length * sizeof(double));

		for (int i = 0; i < Length; i++)
		{
			xi[i] = exp(-1 - lam_imp[i]);
		}

		RedrawAll();

		PicDc->SelectObject(&signal_pen);
		PicDc->MoveTo(DOTS(0, sign[0]));

		for (int i = xmin; i < xmax; i++)
		{
			PicDc->LineTo(DOTS(i, sign[i]));
		}


		PicDcImp->SelectObject(&impulse_pen);
		PicDcImp->MoveTo(DOTS_IMP(0, imp[0]));

		for (int i = xmin_imp; i < xmax_imp; i++)
		{
			PicDcImp->LineTo(DOTS_IMP(i, imp[i]));
		}


		PicDcSvrk->SelectObject(&svertka_pen);
		PicDcSvrk->MoveTo(DOTS_SVRK(0, svert[0]));

		for (int i = xmin_svrk; i < xmax_svrk; i++)
		{
			PicDcSvrk->LineTo(DOTS_SVRK(i, svert[i]));
		}

		PicDc->SelectObject(&vosstanovl_pen);
		PicDc->MoveTo(DOTS(0, xi[0]));
		for (int i = xmin; i < xmax; i++)
		{
			PicDc->LineTo(DOTS(i, xi[i]));
		}

		Sleep(anim_time);

		ofstream out("fb.txt");
		out << z << endl;
		out.close();

		CString err = NULL;
		err.Format(L"%.7f",fb);
		err_value = err;
		UpdateData(FALSE);

		delete lam_imp;
		delete xi;

	} //  end of while(1)

	iter = 0;
	iter = calc;
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
		result += A_mas[i] * exp(-((t - center_pos_mas[i]) / dispersion_mas[i])*((t - center_pos_mas[i]) / dispersion_mas[i]));
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
	RedrawAll();

	double *signal = new double[Length];
	memset(signal, 0, Length * sizeof(double));

	for (int i = 0; i < Length; i++)
	{
		signal[i] = s(i);
	}

	//генерация шума

	double d = energ_noise / 100;

	double mas_energsignal = 0;
	for (int t = 0; t < Length; t++)
	{
		mas_energsignal += signal[t] * signal[t];
	}

	double *mas_psi = new double[Length];
	memset(mas_psi, 0, Length * sizeof(double));
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

	double *mas_shum = new double[Length];
	memset(mas_shum, 0, Length * sizeof(double));
	for (int i = 0; i < Length; i++)
	{
		mas_shum[i] = signal[i] + alpha * mas_psi[i];
	}

	Mashtab(mas_shum, Length, &mn, &mx);

	memset(sign, 0, 50 * sizeof(double));
	for (int i = 0; i < Length; i++)
	{
		sign[i] = mas_shum[i];
	}

	PicDc->SelectObject(&signal_pen);
	PicDc->MoveTo(DOTS(0, mas_shum[0]));

	for (int i = xmin; i < xmax; i++)
	{
		PicDc->LineTo(DOTS(i, mas_shum[i]));
	}

	double *impulse = new double[Length];
	memset(impulse, 0, Length * sizeof(double));
	memset(imp, 0, 50 * sizeof(double));
	double *impulse_norm = new double[Length];
	memset(impulse, 0, Length * sizeof(double));
	double sum_imp = 0;

	for (int i = 0; i < Length; i++)
	{
		impulse[i] = exp(-(i / dispersion_imp)*(i / dispersion_imp)) + exp(-((i - Length) / dispersion_imp)*((i - Length) / dispersion_imp));
		sum_imp += impulse[i];
	}

	for (int i = 0; i < Length; i++)
	{
		impulse_norm[i] = impulse[i] / sum_imp;
		imp[i] = impulse_norm[i];
	}

	PicDcImp->SelectObject(&impulse_pen);
	PicDcImp->MoveTo(DOTS_IMP(0, impulse_norm[0]));

	for (int i = xmin_imp; i < xmax_imp; i++)
	{
		PicDcImp->LineTo(DOTS_IMP(i, impulse_norm[i]));
	}

	double *svertka = new double[Length];
	memset(svertka, 0, Length * sizeof(double));
	memset(svert, 0, 50 * sizeof(double));

	int buff = 0;
	for (int k = 0; k < Length; k++)
	{
		for (int i = 0; i < Length; i++)
		{
			if (k - i < 0)
			{
				buff = Length + (k - i);
				svertka[k] += mas_shum[i] * impulse_norm[buff];
			}
			else
			{
				svertka[k] += mas_shum[i] * impulse_norm[k - i];
			}
		}
		svert[k] = svertka[k];
	}

	PicDcSvrk->SelectObject(&svertka_pen);
	PicDcSvrk->MoveTo(DOTS_SVRK(0, svertka[0]));

	for (int i = xmin_svrk; i < xmax_svrk; i++)
	{
		PicDcSvrk->LineTo(DOTS_SVRK(i, svertka[i]));
	}

	delete signal;
	delete mas_psi;
	delete mas_shum;
	delete impulse;
	delete impulse_norm;
	delete svertka;
}

void CMaxEntropyDlg::OnBnClickedDraw()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(TRUE);
	RedrawAll();

	PicDc->SelectObject(&signal_pen);
	PicDc->MoveTo(DOTS(0, sign[0]));

	for (int i = xmin; i < xmax; i++)
	{
		PicDc->LineTo(DOTS(i, sign[i]));
	}

	PicDcImp->SelectObject(&impulse_pen);
	PicDcImp->MoveTo(DOTS_IMP(0, imp[0]));

	for (int i = xmin_imp; i < xmax_imp; i++)
	{
		PicDcImp->LineTo(DOTS_IMP(i, imp[i]));
	}

	PicDcSvrk->SelectObject(&svertka_pen);
	PicDcSvrk->MoveTo(DOTS_SVRK(0, svert[0]));

	for (int i = xmin_svrk; i < xmax_svrk; i++)
	{
		PicDcSvrk->LineTo(DOTS_SVRK(i, svert[i]));
	}

	int N = Length;
	memset(lambda, 0, 50 * sizeof(double));
	double sredniy_kvadrat_osh = MHJ(N, lambda);
}

void CMaxEntropyDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного

	if (nIDEvent == 100)
	{
		//можно было через таймер, но работает и без него
	}
	else CDialogEx::OnTimer(nIDEvent);
}