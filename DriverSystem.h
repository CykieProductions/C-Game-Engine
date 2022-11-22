#pragma once
#include "Entities.h"
#include "Components.h"

#define ENTITY_LIMIT 3000//60fps 7000//30fps //9999//21fps
#define TILE_LIMIT 9999

#define UPDATE_MODE_START -1
#define UPDATE_MODE_NORMAL 0
#define UPDATE_MODE_LATE 1
#define UPDATE_MODE_FIXED 2

//Structs

typedef struct GameManager
{
	Entity* allEntities[ENTITY_LIMIT];
	long numOfEntities;

	Entity* allTiles[TILE_LIMIT];
	long numOfTiles;

	Transform* allTransforms[ENTITY_LIMIT];
	SpriteRenderer* allSpriteRenderers[ENTITY_LIMIT + TILE_LIMIT];
	Animator* allAnimators[ENTITY_LIMIT + TILE_LIMIT];
	Collider* allColliders[ENTITY_LIMIT + TILE_LIMIT];
	Rigidbody* allRigidbodies[ENTITY_LIMIT];

	//GAMEBITMAP playerSpriteSheet;
	
	//Grab sprite data from a more permanant source
	GAMEBITMAP spriteCache[128];

} GameManager;

/*typedef struct SpriteCacheEntry
{
	GAMEBITMAP sprite;
	bool isInitialized;
	char filePath[128];

} SpriteCacheEntry;*/

//Globals

extern GameManager gGameManager;

//Functions

//void* DS_RegisterComponent(void* component_, void* allOfType[], size_t listSize);//private function

void* AddComponent(DSID id, Entity* entity);
void* GetComponent(DSID id, Entity* entity);
void DS_CopyComponent(void* dest_, Entity* entity_, ComponentInfo* original_);

void DS_Draw(void);
void DS_Update(const short MODE);
void Component_OnUpdate(void* component, DSID id, const short MODE);