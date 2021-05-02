// PitchShifterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "general.h"
#include "PitchShifter.h"
#include "PitchShifterDlg.h"
#include "WaveIn.h"
#include "WaveOut.h"
#include "delayTone.h"
#include <math.h>

#define PROGRAM_MAJRO_VERSION 1
#define PROGRAM_MINOR_VERSION 0

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define		PITCH_NORMAL_VALUE	4

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CPitchShifterDlg dialog




CPitchShifterDlg::CPitchShifterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPitchShifterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPitchShifterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DELAY_SLIDER, m_delaySlider);
	DDX_Control(pDX, IDC_DELAY_EDIT, m_delay_ms);
	DDX_Control(pDX, IDC_PITCH_FACTOR, m_pitchFactor);
	DDX_Control(pDX, IDC_PITCH_PARAM, m_pitchParam);
}

BEGIN_MESSAGE_MAP(CPitchShifterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_OPEN_DEVICES, &CPitchShifterDlg::OnBnClickedOpenDevices)
	ON_CBN_SELCHANGE(IDC_OUTPUT_DEVICES, &CPitchShifterDlg::OnCbnSelchangeOutputDevices)
	ON_CBN_SELCHANGE(IDC_INPUT_DEVICES, &CPitchShifterDlg::OnCbnSelchangeInputDevices)
	ON_BN_CLICKED(IDC_CLOSE_DEVICES, &CPitchShifterDlg::OnBnClickedCloseDevices)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_DELAY_SLIDER, &CPitchShifterDlg::OnNMCustomdrawDelaySlider)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PITCH_FACTOR, &CPitchShifterDlg::OnNMCustomdrawPitchFactor)
END_MESSAGE_MAP()


// CPitchShifterDlg message handlers

BOOL CPitchShifterDlg::OnInitDialog()
{
	USES_CONVERSION;
	char buffer[MAX_PATH];
	float v;
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	// TODO: Add extra initialization here

	pWaveIn = new CWaveIn();
	pWaveOut= new CWaveOut();
	if(FillInputDevices()<=0)
	{
		MessageBox(L"NO Input Devices Found..",L"Error",MB_ICONERROR);
		CDialog::OnOK();
	}

	if(FillOutputDevices()<=0)
	{
//		AfxMessageBox("NO Input Devices Found..",MB_ICONERROR);
		CDialog::OnOK();
	}
	m_delaySlider.SetRange(3,20);
	m_delaySlider.SetPos(3);
	m_pitchFactor.SetRange(-PITCH_NORMAL_VALUE,PITCH_NORMAL_VALUE);
	m_pitchFactor.SetPos(-1);
	m_pitchFactor.SetPos(0);
	UpdateDelay();
	started= false;
	v = PROGRAM_MAJRO_VERSION+ (float)PROGRAM_MINOR_VERSION/100;
	sprintf(buffer,"Pitch Shifter Program version %2.2f ",v);
	SetWindowText(A2T(buffer));
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPitchShifterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPitchShifterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPitchShifterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



int CPitchShifterDlg::OpenInputDevice(void)
{
	USES_CONVERSION;
	int			deviceNo	;
	int			nT1	=	0	;
	int			iT1 =	0   ;
	CString		csT1		;
	double		dT1=	0.0	;
	MMRESULT	mRes=	0	;
	DWORD		sps			;
	WORD		channels    ;
	WORD		bps			;
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS);

	deviceNo = pDevices->GetCurSel();
	if(deviceNo == -1)
		return FAILURE;
	nT1=pFormats->GetCurSel();
	if(nT1==-1)
		return FAILURE;
	pFormats->GetLBText(nT1,csT1);
	sscanf_s((PCHAR)T2A(csT1),"%lf",&dT1);
	dT1=dT1*1000;
	sps = (int)dT1;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	if(csT1.Find(L"mono")!=-1)
		channels=1;
	if(csT1.Find(L"stereo")!=-1)
		channels=2;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	sscanf_s((PCHAR)T2A(csT1),"%d",&iT1);
	bps = iT1;
	mRes = pWaveIn->OpenDevice(deviceNo,sps,channels,bps);
	if(mRes!=MMSYSERR_NOERROR)
	{
		return FAILURE;
	}
	return SUCCESS;
}

int CPitchShifterDlg::OpenOutputDevice(void)
{
	int			deviceNo	;
	int			nT1	=	0	;
	int			iT1 =	0   ;
	CString		csT1		;
	double		dT1=	0.0	;
	MMRESULT	mRes=	0	;
	DWORD		sps			;
	WORD		channels    ;
	WORD		bps			;
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_OUTPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_OUTPUT_FORMATS);

	deviceNo = pDevices->GetCurSel();
	if(deviceNo == -1)
		return FAILURE;

	nT1=pFormats->GetCurSel();
	if(nT1==-1)
		return FAILURE;
	pFormats->GetLBText(nT1,csT1);
	sscanf_s((PCHAR)(LPCTSTR)csT1,"%lf",&dT1);
	dT1=dT1*1000;
	sps = (int)dT1;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	if(csT1.Find(L"mono")!=-1)
		channels=1;
	if(csT1.Find(L"stereo")!=-1)
		channels=2;
	csT1=csT1.Right(csT1.GetLength()-csT1.Find(',')-1);
	csT1.Trim();
	sscanf_s((PCHAR)(LPCTSTR)csT1,"%d",&iT1);
	bps = iT1;
	mRes = pWaveOut->OpenDevice(deviceNo,sps,channels,bps);
	if(mRes!=MMSYSERR_NOERROR)
	{
		return FAILURE;
	}
	CDelayTone * pDelayTone = CDelayTone::GetInstance();
	pDelayTone->SetBPS(bps/8)	;
	pDelayTone->SetSPS(sps)	;
	return		SUCCESS		; 
}

void CPitchShifterDlg::OnBnClickedOpenDevices()
{
	if(!started)
	{
		//Check to see if input and output devices have the same format.
		if(CheckInputOutputFormat()==false)
		{
			MessageBox(L"Input and output format must be same, returning...",L"Error");
			return;
		}
		if(CheckIfMono()==false)
		{
			MessageBox(L"only mono mode is supported for this program, returning...",L"Error");
			return;		
		}
		if(OpenInputDevice()==FAILURE)
		{
			MessageBox(L"Problem opening input device, returning...",L"Error");
			return;
		}
		if(pWaveIn->StartCapture(CHUNK_RATIO)!= SUCCESS)
		{
			MessageBox(L"Problem starting input device, returning...",L"Error");
			return;			
		}

		if(OpenOutputDevice()==FAILURE)
		{
			MessageBox(L"Problem opening output device, returning...",L"Error");
			return;
		}
		if(pWaveOut->StartPlay(CHUNK_RATIO)!= SUCCESS)
		{
			MessageBox(L"Problem starting input device, returning...",L"Error");
			return;			
		}
		pWaveOut->pReaderQ->Connect(pWaveIn->pWriterQ);
		started = true;
		GetDlgItem(IDC_INPUT_DEVICES)->EnableWindow(false);
		GetDlgItem(IDC_INPUT_FORMATS)->EnableWindow(false);
		GetDlgItem(IDC_OUTPUT_DEVICES)->EnableWindow(false);
		GetDlgItem(IDC_OUTPUT_FORMATS)->EnableWindow(false);
		GetDlgItem(IDC_OPEN_DEVICES)->EnableWindow(false);
	}
	else
	{
		MessageBox(L"Program is active now",L"Warning");
	}
}

int CPitchShifterDlg::FillInputDevices(void)
{
	CComboBox		*pBox=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	UINT			nDevices,nC1;
	WAVEINCAPS		stWIC={0};
	MMRESULT		mRes;

	pBox->ResetContent();
	nDevices = pWaveIn->GetDeviceCount();

	for(nC1=0;nC1<nDevices;++nC1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes = pWaveIn->GetDeviceCaps(nC1,&stWIC);
		if(mRes==0)
			pBox->AddString(stWIC.szPname);
	}
	if(pBox->GetCount())
	{
		pBox->SetCurSel(0);
		OnCbnSelchangeInputDevices();
	}
	return nDevices;
}

int CPitchShifterDlg::FillOutputDevices(void)
{
	UINT			nDevices,nC1;
	MMRESULT		mRes;

	CComboBox *pBox=(CComboBox*)GetDlgItem(IDC_OUTPUT_DEVICES);
	WAVEOUTCAPS stWOC={0};
	pBox->ResetContent();
	nDevices=pWaveOut->GetDeviceCount();
	for(nC1=0;nC1<nDevices;++nC1)
	{
		ZeroMemory(&stWOC,sizeof(WAVEOUTCAPS));
		mRes=pWaveOut->GetDeviceCaps(nC1,&stWOC);
		if(mRes==0)
			pBox->AddString(stWOC.szPname);
	}
	if(pBox->GetCount())
	{
		pBox->SetCurSel(0);
		OnCbnSelchangeOutputDevices();
	}
	return nDevices;
}

void CPitchShifterDlg::OnCbnSelchangeOutputDevices()
{
	// TODO: Add your control notification handler code here
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_OUTPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_OUTPUT_FORMATS);
	int nSel;
	WAVEOUTCAPS stWOC={0};
	MMRESULT	mRes;

	pFormats->ResetContent();
	nSel=pDevices->GetCurSel();
	if(nSel!=-1)
	{
		ZeroMemory(&stWOC,sizeof(WAVEOUTCAPS));
		mRes=pWaveOut->GetDeviceCaps(nSel,&stWOC);
		if(mRes==0)
		{
			if(WAVE_FORMAT_1M08==(stWOC.dwFormats&WAVE_FORMAT_1M08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 8-bit"),WAVE_FORMAT_1M08);
			if(WAVE_FORMAT_1M16==(stWOC.dwFormats&WAVE_FORMAT_1M16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
			if(WAVE_FORMAT_1S08==(stWOC.dwFormats&WAVE_FORMAT_1S08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
			if(WAVE_FORMAT_1S16==(stWOC.dwFormats&WAVE_FORMAT_1S16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
			if(WAVE_FORMAT_2M08==(stWOC.dwFormats&WAVE_FORMAT_2M08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
			if(WAVE_FORMAT_2M16==(stWOC.dwFormats&WAVE_FORMAT_2M16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
			if(WAVE_FORMAT_2S08==(stWOC.dwFormats&WAVE_FORMAT_2S08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
			if(WAVE_FORMAT_2S16==(stWOC.dwFormats&WAVE_FORMAT_2S16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
			if(WAVE_FORMAT_4M08==(stWOC.dwFormats&WAVE_FORMAT_4M08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
			if(WAVE_FORMAT_4M16==(stWOC.dwFormats&WAVE_FORMAT_4M16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
			if(WAVE_FORMAT_4S08==(stWOC.dwFormats&WAVE_FORMAT_4S08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
			if(WAVE_FORMAT_4S16==(stWOC.dwFormats&WAVE_FORMAT_4S16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
			if(WAVE_FORMAT_96M08==(stWOC.dwFormats&WAVE_FORMAT_96M08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
			if(WAVE_FORMAT_96S08==(stWOC.dwFormats&WAVE_FORMAT_96S08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
			if(WAVE_FORMAT_96M16==(stWOC.dwFormats&WAVE_FORMAT_96M16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
			if(WAVE_FORMAT_96S16==(stWOC.dwFormats&WAVE_FORMAT_96S16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
			if(pFormats->GetCount())
				pFormats->SetCurSel(0);
		}
	}
}

void CPitchShifterDlg::OnCbnSelchangeInputDevices()
{
	// TODO: Add your control notification handler code here
	CComboBox *pDevices=(CComboBox*)GetDlgItem(IDC_INPUT_DEVICES);
	CComboBox *pFormats=(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS);
	int nSel;
	WAVEINCAPS stWIC={0};
	MMRESULT mRes;

	pFormats->ResetContent();
	nSel=pDevices->GetCurSel();
	if(nSel!=-1)
	{
		ZeroMemory(&stWIC,sizeof(WAVEINCAPS));
		mRes = pWaveIn->GetDeviceCaps(nSel,&stWIC);
		if(mRes==0)
		{
			if(WAVE_FORMAT_1M08==(stWIC.dwFormats&WAVE_FORMAT_1M08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 8-bit"),WAVE_FORMAT_1M08);
			if(WAVE_FORMAT_1M16==(stWIC.dwFormats&WAVE_FORMAT_1M16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, mono, 16-bit"),WAVE_FORMAT_1M16);
			if(WAVE_FORMAT_1S08==(stWIC.dwFormats&WAVE_FORMAT_1S08))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 8-bit"),WAVE_FORMAT_1S08);
			if(WAVE_FORMAT_1S16==(stWIC.dwFormats&WAVE_FORMAT_1S16))
				pFormats->SetItemData(pFormats->AddString(L"11.025 kHz, stereo, 16-bit"),WAVE_FORMAT_1S16);
			if(WAVE_FORMAT_2M08==(stWIC.dwFormats&WAVE_FORMAT_2M08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 8-bit"),WAVE_FORMAT_2M08);
			if(WAVE_FORMAT_2M16==(stWIC.dwFormats&WAVE_FORMAT_2M16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, mono, 16-bit"),WAVE_FORMAT_2M16);
			if(WAVE_FORMAT_2S08==(stWIC.dwFormats&WAVE_FORMAT_2S08))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 8-bit"),WAVE_FORMAT_2S08);
			if(WAVE_FORMAT_2S16==(stWIC.dwFormats&WAVE_FORMAT_2S16))
				pFormats->SetItemData(pFormats->AddString(L"22.05 kHz, stereo, 16-bit"),WAVE_FORMAT_2S16);
			if(WAVE_FORMAT_4M08==(stWIC.dwFormats&WAVE_FORMAT_4M08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 8-bit"),WAVE_FORMAT_4M08);
			if(WAVE_FORMAT_4M16==(stWIC.dwFormats&WAVE_FORMAT_4M16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, mono, 16-bit"),WAVE_FORMAT_4M16);
			if(WAVE_FORMAT_4S08==(stWIC.dwFormats&WAVE_FORMAT_4S08))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 8-bit"),WAVE_FORMAT_4S08);
			if(WAVE_FORMAT_4S16==(stWIC.dwFormats&WAVE_FORMAT_4S16))
				pFormats->SetItemData(pFormats->AddString(L"44.1 kHz, stereo, 16-bit"),WAVE_FORMAT_4S16);
			if(WAVE_FORMAT_96M08==(stWIC.dwFormats&WAVE_FORMAT_96M08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 8-bit"),WAVE_FORMAT_96M08);
			if(WAVE_FORMAT_96S08==(stWIC.dwFormats&WAVE_FORMAT_96S08))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 8-bit"),WAVE_FORMAT_96S08);
			if(WAVE_FORMAT_96M16==(stWIC.dwFormats&WAVE_FORMAT_96M16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, mono, 16-bit"),WAVE_FORMAT_96M16);
			if(WAVE_FORMAT_96S16==(stWIC.dwFormats&WAVE_FORMAT_96S16))
				pFormats->SetItemData(pFormats->AddString(L"96 kHz, stereo, 16-bit"),WAVE_FORMAT_96S16);
			if(pFormats->GetCount())
				pFormats->SetCurSel(0);
		}
	}
}

void CPitchShifterDlg::OnBnClickedCloseDevices()
{
	// TODO: Add your control notification handler code here
	if(started)
	{
		pWaveOut->StopPlay();
		pWaveIn->StopCapture();
		started = false;
	}
	OnOK();
}

void CPitchShifterDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	OnBnClickedCloseDevices();
	if(pWaveOut)
		delete pWaveOut;

	if(pWaveIn)
		delete pWaveIn;
	CDelayTone * pDelayTone = CDelayTone::GetInstance();
	delete pDelayTone;
}

bool CPitchShifterDlg::CheckInputOutputFormat(void)
{
	int nT1,nT2;
	CString csT1,csT2;
	CComboBox *pInFormats  =(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS );
	CComboBox *pOutFormats =(CComboBox*)GetDlgItem(IDC_OUTPUT_FORMATS);

	nT1=pInFormats->GetCurSel();
	if(nT1==-1)
		return false;
	pInFormats->GetLBText(nT1,csT1);

	nT2=pOutFormats->GetCurSel();
	if(nT1==-1)
		return false;
	pInFormats->GetLBText(nT2,csT2);

	if(csT1.Compare(csT2)==0)
		return true;
	return false;
}

void CPitchShifterDlg::UpdateDelay(void)
{
	USES_CONVERSION;
	int			curDelay;
	float		curPitch;
	CDelayTone* pDelayTone = CDelayTone::GetInstance();
	char str[20];
	curDelay = m_delaySlider.GetPos()*250;
	itoa(curDelay,str,10)				;
	m_delay_ms.SetWindowText(A2T(str))	;
	pDelayTone->SetDelay(curDelay)		;

	curPitch = m_pitchFactor.GetPos()	;
	curPitch /= PITCH_NORMAL_VALUE		;
	curPitch  = pow(2,curPitch)			;
	sprintf(str,"%2.2f",curPitch);
	m_pitchParam.SetWindowText(A2T(str));
	pDelayTone->SetPitchFactor(curPitch);
	delay_changed = true				;
}

void CPitchShifterDlg::OnNMCustomdrawDelaySlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	UpdateDelay();
}


void CPitchShifterDlg::OnNMCustomdrawPitchFactor(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
	UpdateDelay();
}

bool CPitchShifterDlg::CheckIfMono(void)
{
	int nT1,nT2;
	CString csT1,csT2;
	CComboBox *pInFormats  =(CComboBox*)GetDlgItem(IDC_INPUT_FORMATS );
	CComboBox *pOutFormats =(CComboBox*)GetDlgItem(IDC_OUTPUT_FORMATS);

	nT1=pInFormats->GetCurSel();
	if(nT1==-1)
		return false;
	pInFormats->GetLBText(nT1,csT1);

	nT2=pOutFormats->GetCurSel();
	if(nT1==-1)
		return false;
	pInFormats->GetLBText(nT2,csT2);

	if(csT1.Find(L"mono") < 0 )
		return false;

	if(csT2.Find(L"mono") < 0 )
		return false;


	return true;
}
