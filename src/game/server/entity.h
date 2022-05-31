/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITY_H
#define GAME_SERVER_ENTITY_H

#include <base/vmath.h>

#include "alloc.h"
#include "gamecontext.h"
#include "gameworld.h"

class CCollision;

/*
	Class: Entity
		Basic entity class.
*/
class CEntity
{
	MACRO_ALLOC_HEAP()

private:
	friend class CGameWorld; // entity list handling
	CEntity *m_pPrevTypeEntity;
	CEntity *m_pNextTypeEntity;

	/* Identity */
	class CGameWorld *m_pGameWorld;
	CCollision *m_pCCollision;

	int m_ID;
	int m_ObjType;

	/*
		Variable: m_ProximityRadius
			Contains the physical size of the entity.
	*/
	float m_ProximityRadius;

protected:
	/* State */
	bool m_MarkedForDestroy;

public: // TODO: Maybe make protected
	/*
		Variable: m_Pos
			Contains the current posititon of the entity.
	*/
	vec2 m_Pos;

	/* Getters */
	int GetID() const { return m_ID; }

	/* Constructor */
	CEntity(CGameWorld *pGameWorld, int Objtype, vec2 Pos = vec2(0, 0), int ProximityRadius = 0);

	/* Destructor */
	virtual ~CEntity();

	/* Objects */
	class CGameWorld *GameWorld() { return m_pGameWorld; }
	class CConfig *Config() { return m_pGameWorld->Config(); }
	class CGameContext *GameServer() { return m_pGameWorld->GameServer(); }
	class IServer *Server() { return m_pGameWorld->Server(); }
	CCollision *Collision() { return m_pCCollision; }

	/* Getters */
	CEntity *TypeNext() { return m_pNextTypeEntity; }
	CEntity *TypePrev() { return m_pPrevTypeEntity; }
	const vec2 &GetPos() const { return m_Pos; }
	float GetProximityRadius() const { return m_ProximityRadius; }

	/* Other functions */

	/*
		Function: Destroy
			Destroys the entity.
	*/
	virtual void Destroy() { delete this; }

	/*
		Function: Reset
			Called when the game resets the map. Puts the entity
			back to its starting state or perhaps destroys it.
	*/
	virtual void Reset() {}

	/*
		Function: Tick
			Called to progress the entity to the next tick. Updates
			and moves the entity to its new state and position.
	*/
	virtual void Tick() {}

	/*
		Function: TickDefered
			Called after all entities Tick() function has been called.
	*/
	virtual void TickDefered() {}

	/*
		Function: TickPaused
			Called when the game is paused, to freeze the state and position of the entity.
	*/
	virtual void TickPaused() {}

	/*
		Function: Snap
			Called when a new snapshot is being generated for a specific
			client.

		Arguments:
			SnappingClient - ID of the client which snapshot is
				being generated. Could be -1 to create a complete
				snapshot of everything in the game for demo
				recording.
	*/
	virtual void Snap(int SnappingClient) {}

	/*
		Function: SwapClients
			Called when two players have swapped their client ids.

		Arguments:
			Client1 - First client ID
			Client2 - Second client ID
	*/
	virtual void SwapClients(int Client1, int Client2) {}

	/*
		Function: NetworkClipped
			Performs a series of test to see if a client can see the
			entity.

		Arguments:
			SnappingClient - ID of the client which snapshot is
				being generated. Could be -1 to create a complete
				snapshot of everything in the game for demo
				recording.

		Returns:
			True if the entity doesn't have to be in the snapshot.
	*/
	bool NetworkClipped(int SnappingClient) const;
	bool NetworkClipped(int SnappingClient, vec2 CheckPos) const;
	bool NetworkClippedLine(int SnappingClient, vec2 StartPos, vec2 EndPos) const;

	bool GameLayerClipped(vec2 CheckPos);

	// DDRace

	bool GetNearestAirPos(vec2 Pos, vec2 PrevPos, vec2 *pOutPos);
	bool GetNearestAirPosPlayer(vec2 PlayerPos, vec2 *OutPos);

	int m_Number;
	int m_Layer;
};

bool NetworkClipped(const CGameContext *pGameServer, int SnappingClient, vec2 CheckPos);
bool NetworkClippedLine(const CGameContext *pGameServer, int SnappingClient, vec2 StartPos, vec2 EndPos);

#endif
