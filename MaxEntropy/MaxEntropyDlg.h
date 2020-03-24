
// MaxEntropyDlg.h: файл заголовка
//

#pragma once


// Диалоговое окно CMaxEntropyDlg
class CMaxEntropyDlg : public CDialogEx
{
// Создание
public:
	CMaxEntropyDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAXENTROPY_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:

	CWnd * PicWnd;				//области рисования
	CDC * PicDc;
	CRect Pic;

	CWnd * PicWndImp;
	CDC * PicDcImp;
	CRect PicImp;

	CWnd * PicWndSvrk;
	CDC * PicDcSvrk;
	CRect PicSvrk;

	CPen osi_pen;				//ручки
	CPen setka_pen;
	CPen signal_pen;
	CPen impulse_pen;
	CPen svertka_pen;
	CPen vosstanovl_pen;

	double xp, yp,				//коэфициенты пересчета
		xmin, xmax,				//максисимальное и минимальное значение х 
		ymin, ymax,				//максисимальное и минимальное значение y
		mn, mx;					//коэффициенты масштабирования

	double xp_imp, yp_imp,		//коэфициенты пересчета
		xmin_imp, xmax_imp,		//максисимальное и минимальное значение х 
		ymin_imp, ymax_imp,		//максисимальное и минимальное значение y
		mn_imp, mx_imp;			//коэффициенты масштабирования

	double xp_svrk, yp_svrk,	//коэфициенты пересчета
		xmin_svrk, xmax_svrk,	//максисимальное и минимальное значение х 
		ymin_svrk, ymax_svrk,	//максисимальное и минимальное значение y
		mn_svrk, mx_svrk;		//коэффициенты масштабирования

	double sign[50];			//глобальный массив для сигнала
	double imp[50];				//глобальный массив для имп.характеристики
	double svert[50];			//глобальный массив для свертки
	double lambda[50];			//глобальный массив лямбд(неопр.мн-ль Лагранжа)
	int iter;

	double Pi = 3.141592653589;
	double Length;				//длина сигнала
	double amplitude_1;			//амплитуды сигнала
	double amplitude_2;
	double amplitude_3;
	double dispersion_1;		//ширина пиков(дисперсия)
	double dispersion_2;
	double dispersion_3;
	double center_pos_1;		//позиции пиков
	double center_pos_2;
	double center_pos_3;
	double dispersion_imp;		//ширина импульсной характеристики
	double precision;			//точность вычисления для MHJ(оптимизация)
	double anim_time;			//длительность каждого "кадра" анимации(мс) для MHJ
	double energ_noise;			//уровень шума в процентах
	int calc;
	CString err_value;
	
	afx_msg void OnBnClickedDraw();				//обработчик кнопки рисования анимации
	afx_msg void OnBnClickedDrawSignal();		//обработчик кнопки рисования сигнала
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	void RedrawAll();			//функция перерисовки диалога

	double CMaxEntropyDlg::s(int t);		//функция сигнала
	void CMaxEntropyDlg::Mashtab(double arr[], int dim, double *mmin, double *mmax);		//функция масштаба
	double CMaxEntropyDlg::MHJ(int kk, double* x);		//метод Хука-Дживса
	double CMaxEntropyDlg::function(double*x);
	double CMaxEntropyDlg::Psi();						//рандомизация для шума
};
