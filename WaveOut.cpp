#include "StdAfx.h"
#include "WaveOut.h"
#include "general.h"
#include "DelayTone.h"
#include "smbPitchShift.h"

#pragma comment(lib,"winmm.lib")

void CALLBACK waveOutFunc(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
	WAVEHDR *pHdr=NULL;
	switch(uMsg)
	{
		case WOM_CLOSE:
			break;

		case WOM_DONE:
			{
				CWaveOut *pDlg=(CWaveOut*)dwInstance;
				pDlg->ProcessHeader((WAVEHDR *)dwParam1);
			}
			break;

		case WOM_OPEN:
			break;

		default:
			break;
	}
}

CWaveOut::CWaveOut(void)
{
	read_size	= 0		;
	fBuffer		= NULL	;
	wBytesPerSample = 0 ;
	m_hWaveOut	= NULL	;
	pReaderQ	= NULL	;
	ZeroMemory(&m_stWFEX,sizeof(WAVEFORMATEX));
	ZeroMemory( m_stWHDR,MAX_OUTPUT_BUFFERS*sizeof(WAVEHDR));
}

CWaveOut::~CWaveOut(void)
{
	StopPlay();
	if(fBuffer)
		delete []fBuffer;
	fBuffer = NULL;
	if(pReaderQ)
		delete pReaderQ;
	pReaderQ = NULL;
}

int CWaveOut::GetDeviceCount(void)
{
	return waveOutGetNumDevs();
}

int CWaveOut::GetDeviceCaps(int deviceNo,PWAVEOUTCAPS pWaveParams)
{
	return waveOutGetDevCaps(deviceNo,pWaveParams,sizeof(WAVEOUTCAPS));
}

int CWaveOut::OpenDevice( WORD	 deviceNo,
						  DWORD	 samplesPerSecond,
						  WORD	 nChannels,
						  WORD   wBitsPerSample
					    )
{
	if(m_hWaveOut)// Device is open now.
		return FAILURE;

	MMRESULT mRes			 =	0					;
	m_stWFEX.nSamplesPerSec  =	samplesPerSecond	;
	m_stWFEX.nChannels	     =	nChannels			;
	m_stWFEX.wBitsPerSample  =	wBitsPerSample		;
	m_stWFEX.wFormatTag		 =	WAVE_FORMAT_PCM		;
	m_stWFEX.nBlockAlign	 =	m_stWFEX.nChannels*m_stWFEX.wBitsPerSample/8;
	m_stWFEX.nAvgBytesPerSec =	m_stWFEX.nSamplesPerSec*m_stWFEX.nBlockAlign;
	m_stWFEX.cbSize			 =	sizeof(WAVEFORMATEX);
	pReaderQ				 =  new CReaderQ()		;
	mRes = waveOutOpen(
						&m_hWaveOut,
						deviceNo,
						&m_stWFEX,
						(DWORD_PTR)waveOutFunc,
						(DWORD_PTR)this,
						CALLBACK_FUNCTION
					  );
	return mRes;
}

int CWaveOut::ProcessHeader(WAVEHDR * pHdr)
{
	CDelayTone * pDelayTone = CDelayTone::GetInstance()	;
	static int SilenceChunk		=	0					;
	MMRESULT mRes				=	0					;
	int8*	ptr8	;
	int16*	ptr16	;
	int fft_size;
//	TRACE("%d",pHdr->dwUser);
	if(WHDR_DONE==(WHDR_DONE &pHdr->dwFlags))
	{
		if(pDelayTone->GetChanged())
		{
			SilenceChunk = pDelayTone->GetDelay();
			SilenceChunk /= 250	;
			SilenceChunk -= 3	;
			pReaderQ->EmptyQueue();
		}
		if( SilenceChunk > 0 )
		{
			memset((uint8*)pHdr->lpData,0,read_size*wBytesPerSample);
			mRes=waveOutWrite(m_hWaveOut,pHdr,sizeof(WAVEHDR));
			SilenceChunk--;
		}
		else
		{
			if(pReaderQ->GetFilledSize() >= read_size)
			{
				pReaderQ->RemoveFromQueue((uint8*)pHdr->lpData,read_size);
				ptr8  = (int8*)pHdr->lpData;
				ptr16 = (int16*)pHdr->lpData;
				for(int i=0; i < read_size; i++)
				{
					switch(pDelayTone->GetBPS())
					{
					case 1:

							fBuffer[i] = ptr8[i];
							fBuffer[i] /= CHAR_NORM;
						break;
					case 2:
							fBuffer[i] = ptr16[i];
							fBuffer[i] /= SHORT_NORM;

						break;
					}
				}
/*
				if(pDelayTone->GetSPS() < 25000)
				{
					fft_size = 2048;
				}
				else if(pDelayTone->GetSPS() < 50000)
				{
					fft_size = 4096;
				}
				else
				{
					fft_size = 8192;
				}
*/
				fft_size = 2048;

				smbPitchShift(pDelayTone->GetPitchFactor(), 
				   read_size, 
				   fft_size, 
				   4, 
				   pDelayTone->GetSPS(), 
				   fBuffer, fBuffer);
				for(int i=0; i < read_size; i++)
				{
					switch(pDelayTone->GetBPS())
					{
					case 1:
							if(fBuffer[i]>1)
								fBuffer[i]=1;

							if(fBuffer[i]<-1)
								fBuffer[i]=-1;
							ptr8[i]= fBuffer[i] * CHAR_NORM ;
						break;
					case 2:
							if(fBuffer[i]>1)
								fBuffer[i]=1;

							if(fBuffer[i]<-1)
								fBuffer[i]=-1;

							ptr16[i] = fBuffer[i] * SHORT_NORM;
						break;
					}
				}

			}
			else
				memset(pHdr->lpData,0,read_size*wBytesPerSample);
			mRes=waveOutWrite(m_hWaveOut,pHdr,sizeof(WAVEHDR));
		}
		// Fill pHdr again and play it by using startWaveout;
	}
	return SUCCESS;
}

int CWaveOut::StartPlay(float ratio)
{
	MMRESULT res=0;
	PrepareBuffers(ratio);
	res=waveOutWrite(m_hWaveOut,&m_stWHDR[0],sizeof(WAVEHDR));
	res=waveOutWrite(m_hWaveOut,&m_stWHDR[1],sizeof(WAVEHDR));
	res=waveOutWrite(m_hWaveOut,&m_stWHDR[2],sizeof(WAVEHDR));


	if (res!=MMSYSERR_NOERROR)
		return FAILURE;
	return SUCCESS;
}

int CWaveOut::StopPlay(void)
{
	MMRESULT mRes=0;
	if(m_hWaveOut)
	{
		UnprepareBuffers();
		mRes=waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
		Sleep(500);
		return SUCCESS;
	}
	return FAILURE;
}

int CWaveOut::PrepareBuffers(float ratio)
{
	MMRESULT mRes=0;
	int nT1=0;
	int c_size;

	for(nT1=0;nT1<MAX_OUTPUT_BUFFERS;++nT1)
	{
		c_size = (SIZE_T)m_stWFEX.nAvgBytesPerSec*ratio;
		if(c_size %2 )
			c_size--;
		m_stWHDR[nT1].lpData=(LPSTR)HeapAlloc(GetProcessHeap(),8,c_size);
		m_stWHDR[nT1].dwBufferLength=c_size	;
		m_stWHDR[nT1].dwUser		=				nT1						;
		mRes=waveOutPrepareHeader(m_hWaveOut,&m_stWHDR[nT1],sizeof(WAVEHDR));
		if(mRes!=0)
		{
			return FAILURE;
		}
	}
	wBytesPerSample = m_stWFEX.wBitsPerSample/8;
	read_size = (SIZE_T)c_size/wBytesPerSample;
	if(fBuffer)
		delete []fBuffer;
	fBuffer = new float[read_size];
	return SUCCESS;
}

int CWaveOut::UnprepareBuffers(void)
{
	MMRESULT mRes=0;
	int nT1=0;

	if(m_hWaveOut)
	{
		//mRes=waveOutPause(m_hWaveOut);		
		//waveOutBreakLoop(m_hWaveOut);
		for(nT1=0;nT1<MAX_OUTPUT_BUFFERS;++nT1)
		{
			if(m_stWHDR[nT1].lpData)
			{
				mRes=waveOutUnprepareHeader(m_hWaveOut,&m_stWHDR[nT1],sizeof(WAVEHDR));
				HeapFree(GetProcessHeap(),0,m_stWHDR[nT1].lpData);
				ZeroMemory(&m_stWHDR[nT1],sizeof(WAVEHDR));
			}
		}
	}
	return SUCCESS;
}