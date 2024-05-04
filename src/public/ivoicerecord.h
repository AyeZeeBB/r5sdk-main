//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IVOICERECORD_H
#define IVOICERECORD_H

class IVoiceRecord
{
protected:
	virtual ~IVoiceRecord() {}
public:
	virtual void		Release() = 0;
	virtual bool		RecordStart() = 0;
	virtual void		RecordStop() = 0;
	virtual int			Idle() = 0;
	virtual void		Unk() = 0;
	virtual int			GetRecordedData(short* pOut, int nSamplesWanted, bool bUnk) = 0;
};

#endif