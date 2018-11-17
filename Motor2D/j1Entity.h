#ifndef __Entity__
#define __Entity__

#include "j1EntityManager.h"
#include "j1App.h"
#include <string>
#include <list>


enum class EntityType
{
	NONE = -1,
	PLAYER,
	ENEMY_FLYING,
	ENEMY_WWALKING,
};

class j1Entity
{
public:
	//Constructor
	j1Entity(EntityType type) : type(type) {}

	virtual ~j1Entity() {}

	virtual bool Awake(pugi::xml_node&) { return true; }

	virtual bool Start() { return true; }

	virtual bool PreUpdate() { return true; }

	virtual bool UpdateLogic(float dt) { return true; }

	virtual bool Update(float dt) { return true; }

	virtual bool Update() { return true; }

	virtual bool PostUpdate() { return true; }

	virtual bool CleanUp() { return true; }

	virtual bool Load(pugi::xml_node&) { return true; }
	virtual bool Save(pugi::xml_node&) const { return true; };

	virtual void Input() {};
	virtual void Draw() {};

	virtual void OnCollision(Collider*, Collider*) {}

public:
	p2SString name;
	bool PushedRightWall;
	bool PushesRightWall;

	bool PushedLeftWall;
	bool PushesLeftWall;

	bool WasOnGround;
	bool OnGround;

	bool WasAtCeiling;
	bool AtCeiling;

private:
	EntityType type;

};

#endif //__Entity__