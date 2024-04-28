//======== Copyright (c) Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//
#include "recipientfilter.h"
#include "game/shared/ipredictionsystem.h"
#include "util_server.h"

CRecipientFilter::CRecipientFilter()
{
	Reset();
}

CRecipientFilter::~CRecipientFilter()
{
}

void CRecipientFilter::Reset()
{
	m_bReliable		= false;
	m_bInitMessage	= false;
	m_Recipients.RemoveAll();
	m_bUsingPredictionRules = false;
	m_bIgnorePredictionCull = false;

	memset(m_gapA, 0, sizeof(m_gapA));
}

bool CRecipientFilter::IsReliable(void) const
{
	return m_bReliable;
}

void CRecipientFilter::MakeReliable( void )
{
	m_bReliable = true;
}

bool CRecipientFilter::IsInitMessage( void ) const
{
	return m_bInitMessage;
}

int CRecipientFilter::GetRecipientCount( void ) const
{
	return m_Recipients.Count();
}

int CRecipientFilter::GetRecipientIndex( int nSlot ) const
{
	if ( nSlot < 0 || nSlot >= GetRecipientCount() )
		return -1;

	return m_Recipients[nSlot].m_nIndex;
}

int	CRecipientFilter::FindSlotForIndex( int nIndex ) const
{
	FOR_EACH_VEC(m_Recipients, slot)
	{
		if (m_Recipients[slot].m_nIndex == nIndex)
		{
			return slot;
		}
	}

	return m_Recipients.InvalidIndex();
}

void CRecipientFilter::AddRecipient(const CPlayer* pPlayer)
{
	Assert(pPlayer);

	int nIndex = pPlayer->GetEdict();

	Recipient recipient{
		nIndex,
		false
	};

	m_Recipients.AddToTail(recipient);

	// If we're predicting and this is not the first time we've predicted this sound
	// then don't send it to the local player again.
	if (m_bUsingPredictionRules)
	{
		int nSlot = FindSlotForIndex(nIndex);

		// Only add local player if this is the first time doing prediction
		if (g_pRecipientFilterPredictionSystem->GetSuppressHost() == pPlayer)
		{
			//Have seen this set to false in a CProjectile method (sub_140FB0D90), applied across all IPredictionSystem instances
			if (g_pRecipientFilterPredictionSystem->m_bAllowRemovals_not_sure)
			{
				if (nSlot != m_Recipients.InvalidIndex())
					m_Recipients.Remove(nSlot);
			}
			else
			{
				if (nSlot != m_Recipients.InvalidIndex())
					m_Recipients[nSlot].m_bIsIgnoredMaybe = true;
			}
		}
	}
}

bool CRecipientFilter::IsIgnored( int nSlot ) const
{
	Assert( nSlot >= 0 && nSlot < GetRecipientCount() );
	return m_Recipients[nSlot].m_bIsIgnoredMaybe;
}

int CRecipientFilter::DistTo( int nSlot, const Vector3D& pos ) const
{
	Assert(nSlot >= 0 && nSlot < GetRecipientCount());

	const CPlayer* pPlayer = UTIL_PlayerByIndex(m_Recipients[nSlot].m_nIndex);
	const Vector3D origin = pPlayer->GetViewOffset() + pPlayer->GetVecPrevAbsOrigin();
	const vec_t distance = origin.DistTo(pos);

	return distance > 65535.f ? 65535ll : static_cast<int>(distance);
}
