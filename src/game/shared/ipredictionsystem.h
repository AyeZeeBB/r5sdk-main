//========= Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IPREDICTIONSYSTEM_H
#define IPREDICTIONSYSTEM_H

class IPredictionSystem
{
public:
	virtual ~IPredictionSystem() {};

	CBaseEntity const* GetSuppressHost( void )
	{
		if ( DisableFiltering() )
		{
			return nullptr;
		}

		return m_pSuppressHost;
	}

private:
	bool DisableFiltering( void ) const
	{
		return ( m_nStatusPushed > 0 ) ? true : false;
	}

	IPredictionSystem* m_pNextSystem;
	bool m_bSuppressEvent;
	class CBaseEntity* m_pSuppressHost;
	//[Robotic]:
	//See comment in CRecipientFilter::AddRecipient
	bool m_bAllowRemovals_not_sure;
	//
	int m_nStatusPushed;
};

static_assert(sizeof(IPredictionSystem) == 0x28);
#endif