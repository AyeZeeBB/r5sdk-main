//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef RECIPIENTFILTER_H
#define RECIPIENTFILTER_H
#include "player.h"
#include "irecipientfilter.h"
#include "game/shared/ipredictionsystem.h"

class CRecipientFilter;

inline IPredictionSystem* g_pRecipientFilterPredictionSystem;
inline void(*CRecipientFilter__RemoveRecipient)( CRecipientFilter* thisp, const CPlayer* pPlayer );

class CRecipientFilter : public IRecipientFilter
{
	struct Recipient
	{
		int m_nIndex;
		bool m_bIsIgnoredMaybe;
	};

	static_assert( sizeof( Recipient ) == 0x8 );

public:
					CRecipientFilter();
	virtual			~CRecipientFilter();

	virtual bool	IsReliable( void ) const;
	virtual void	MakeReliable( void );

	virtual bool	IsInitMessage( void ) const;

	virtual int		GetRecipientCount( void ) const;
	virtual int		GetRecipientIndex( int nSlot ) const;

	virtual bool	IsIgnored( int nSlot ) const;
	virtual int		DistTo( int nSlot, const Vector3D& pos ) const;

	void			Reset( void );

	int				FindSlotForIndex( int nIndex ) const;

	void			AddRecipient( const CPlayer* pPlayer );

	void			RemoveRecipient( const CPlayer* pPlayer )
	{
		CRecipientFilter__RemoveRecipient(this, pPlayer);
	}

private:
	bool m_bReliable;
	bool m_bInitMessage;
	//[Robotic]:
	//Could just be padding, havent seen this used yet
	char m_gapA[6];
	CUtlVector<Recipient> m_Recipients;
	bool m_bUsingPredictionRules;
	bool m_bIgnorePredictionCull;
};

static_assert(sizeof(CRecipientFilter) == 0x38);

class CSingleUserRecipientFilter : public CRecipientFilter
{
public:
	CSingleUserRecipientFilter(const CPlayer* pPlayer)
	{
		AddRecipient(pPlayer);
	}
};

class VRecipientFilter : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("CRecipientFilter::RemoveRecipient", CRecipientFilter__RemoveRecipient);
		LogVarAdr("g_pRecipientFilterPredictionSystem", g_pRecipientFilterPredictionSystem);
	}
	virtual void GetFun(void) const 
	{
		g_GameDll.FindPatternSIMD("44 0F BF 42 ? 33 C0").GetPtr(CRecipientFilter__RemoveRecipient);
	};
	virtual void GetVar(void) const
	{
		CMemory(CRecipientFilter__RemoveRecipient).FindPatternSelf("38 05").ResolveRelativeAddressSelf(0x2, 0x6)
			.OffsetSelf(-(ptrdiff_t)offsetof(IPredictionSystem, m_bAllowRemovals_not_sure)).GetPtr(g_pRecipientFilterPredictionSystem);
	}
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const { };
};

#endif