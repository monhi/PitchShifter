#pragma once
#include "general.h"
class CDelayTone
{
private:
	CDelayTone(void)			;
	int		delay				;
	int		samplesPerSecond	;
	int		bytesPerSample		;
	float	pitchFactor			;
	bool	changed				;
	static CDelayTone* instance	;
	CRITICAL_SECTION	lock	;
public:
	~CDelayTone(void);
	static CDelayTone* GetInstance();
	void SetDelay(int d);
	int GetDelay();
	bool GetChanged(){return changed;}
	float GetPitchFactor(){return pitchFactor;}
	void  SetPitchFactor(float a) {pitchFactor = a;}
	void  SetSPS(int sps){samplesPerSecond = sps;}
	int   GetSPS(		){return samplesPerSecond;}
	void  SetBPS(int bps){bytesPerSample = bps;}
	int   GetBPS(		){return bytesPerSample;}
};
