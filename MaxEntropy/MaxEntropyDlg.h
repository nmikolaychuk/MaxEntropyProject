
// MaxEntropyDlg.h: файл заголовка
//

#pragma once
#include "stdafx.h"

// Диалоговое окно CMaxEntropyDlg
class CMaxEntropyDlg : public CDialogEx
{
	// Создание
public:
	CMaxEntropyDlg(CWnd* pParent = NULL);	// стандартный конструктор

// Данные диалогового окна
	enum { IDD = IDD_MAXENTROPY_DIALOG };
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

	double s(int t);		//функция сигнала
	void Mashtab(double arr[], int dim, double* mmin, double* mmax);		//функция масштаба
	double MHJ(int kk, float* x);		//метод Хука-Дживса
	double function(float* x);
	double Psi();
	void Graph1(double*, CDC*, CRect, CPen*, double);
	void Graph2(double*, CPen*, double*, CPen*, CDC*, CRect, double);

	DWORD dwThread;
	HANDLE hThread;

	CWnd* PicWnd;				//области рисования
	CDC* PicDc;
	CRect Pic;

	CWnd* PicWndImp;
	CDC* PicDcImp;
	CRect PicImp;

	CWnd* PicWndSvrk;
	CDC* PicDcSvrk;
	CRect PicSvrk;

	CPen osi_pen;				//ручки
	CPen setka_pen;
	CPen signal_pen;
	CPen impulse_pen;
	CPen svertka_pen;
	CPen vosstanovl_pen;

	int Length;				//длина сигнала

	double* sign;			//глобальный массив для сигнала
	double* imp;				//глобальный массив для имп.характеристики
	double* svert;			//глобальный массив для свертки
	float* lambda;			//глобальный массив лямбд(неопр.мн-ль Лагранжа)
	double* Deconv;
	int iter;

	double Pi = 3.141592653589;
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
	bool SignalFlag, ImpulseFlag, SvertkaFlag, DeconvFlag;
	char err[100];
	char func[100];
	char znach[1000];
	double otkl;

	afx_msg void OnBnClickedDraw();				//обработчик кнопки рисования анимации
	afx_msg void OnBnClickedDrawSignal();		//обработчик кнопки рисования сигнала
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	CEdit error;

	double Min, Max, Min1, Max1, Min2, Max2;
	double xx0, xxmax, yy0, yymax, xxi, yyi;

	CButton StartStopOptimization;
	BOOL bRunTh = false;		// переменная, показывающая, запущен ли процесс
	CString start = L"Запуск";		// строки с именем кнопки для каждого случая
	CString stop = L"Остановка";
	afx_msg void OnBnClickedButtonDropAnim();
	CEdit functional;
};
