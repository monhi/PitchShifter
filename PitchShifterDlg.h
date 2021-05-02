// PitchShifterDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CPitchShifterDlg dialog
class CWaveIn;
class CWaveOut;
class CPitchShifterDlg : public CDialog
{
// Construction
public:
	CPitchShifterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PitchShifter_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	bool		delay_changed;
	bool		started;
	CWaveIn*	pWaveIn;
	CWaveOut*	pWaveOut;
	int			OpenInputDevice(void);
	int			OpenOutputDevice(void);
	afx_msg void OnBnClickedOpenDevices();
	int FillInputDevices(void);
	int FillOutputDevices(void);
	afx_msg void OnCbnSelchangeOutputDevices();
	afx_msg void OnCbnSelchangeInputDevices();
	afx_msg void OnBnClickedCloseDevices();
	afx_msg void OnDestroy();
	bool CheckInputOutputFormat(void);
	CSliderCtrl m_delaySlider;
	CEdit m_delay_ms;
	void UpdateDelay(void);
	afx_msg void OnNMCustomdrawDelaySlider(NMHDR *pNMHDR, LRESULT *pResult);
	CSliderCtrl m_pitchFactor;
	afx_msg void OnNMCustomdrawPitchFactor(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_pitchParam;
	bool CheckIfMono(void);
};
