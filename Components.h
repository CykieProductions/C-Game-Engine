#pragma once

#include "Renderer.h"
#include "Cytools.h"
//#include "Entities.h"
//#include "DriverSystem.h"

//typedef unsigned short bool;
#define ANIM_DOWN	0
#define ANIM_UP		1
#define ANIM_LEFT	2
#define ANIM_RIGHT	3

typedef enum DSID
{
	DSID_Transform,
	DSID_SpriteRenderer,
	DSID_Animator,
	DSID_Collider,
	DSID_Rigidbody,

} DSID;

typedef enum DSMessage
{
	DSMSG_ANIM_FAILED,
	DSMSG_ANIM_PLAYED,
	DSMSG_ANIM_CONTINUED,

} DSMessage;

typedef enum SortingLayer
{
	SortLayer_Back		= -9,
	SortLayer_Minus1	= -1,
	SortLayer_YZ		= 0,
	SortLayer_Plus1		= 1,
	SortLayer_Front		= 9,
	SortLayer_UI		= 10,

} SortingLayer;

typedef enum SpriteCacheEntries
{
	SCE_PlayerSS,
	SCE_OverworldTileset1

} SpriteCacheEntries;

typedef enum ObjectType
{
	OT_Entity,
	OT_Tile,
	OT_Gizmo,

}ObjectType;

typedef enum ColliderType
{
	CT_Rectangle,
	CT_Circle,
	CT_Polygon,

}ColliderType;

/*
#define DSID_Transform			0
#define DSID_SpriteRenderer		1
#define DSID_Animator			2

typedef short DSMessage;
#define DSMSG_ANIM_FAILED		0
#define DSMSG_ANIM_PLAYED		1
#define DSMSG_ANIM_CONTINUED	2*/

typedef struct ComponentInfo
{
	void* memory;
	int dsID;
	int size;

} ComponentInfo;

typedef struct Entity
{
	long id;
	bool disabled;
	ObjectType TYPE;

	ComponentInfo components[64];
	//Transform* transform;//Transform needs a reference to Entity, so Entity can't reference Transform

	//Must be greater than 0 to work
	float lifespan;
	float aliveTimer;

	//tmp
	float baseSpeed;
	float speed;

} Entity;

typedef struct Transform
{
	Entity* entity;
	//Vector2 position;
	//Vector2 scale;

	//int positionX;
	//int positionY;
	Vector2INT position;
	int positionZ;

	int scaleX;
	int scaleY;

} Transform;


typedef struct SpriteSliceData
{
	//GAMEBITMAP* source_;
	//GAMEBITMAP source;

	SpriteCacheEntries sourceIndex;

	bool useSourceDirectly;
	bool flipSprite;
	int slicePosX;
	int slicePosY;
	int sliceWidth;
	int sliceHeight;
	int pivotX;
	int pivotY;
	//void* (*CallBack)(void* params[])

} SpriteSliceData;

//SpriteSliceData Constructor
SpriteSliceData new_SpriteSliceData(
	SpriteCacheEntries sourceIndex,
	bool useSourceDirectly,
	bool flipSprite,
	int slicePosX,
	int slicePosY,
	int sliceWidth,
	int sliceHeight,
	int pivotX,
	int pivotY);

typedef struct SpriteRenderer
{
	Entity* entity;
	//Transform* transform;

	GAMEBITMAP sprite;
	float transparencyPercent;

	SortingLayer sortingLayer;
	int orderInLayer;

	bool flipX;
	bool flipY;

	SpriteSliceData defaultSprite;

} SpriteRenderer;

typedef struct AnimationClip
{
	char name[32];
	short variant;//Use the same name for all directions/outfits
	//previously GAMEBITMAP frames[64];
	SpriteSliceData frames[64];
	float delays[64];
	float defaultDelay;

	unsigned int frameCount;

} AnimationClip;
typedef struct Animator
{
	Entity* entity;
	SpriteRenderer* renderer_;

	AnimationClip* curClip_;
	bool flipFrames;
	int curPriority;
	bool nowLooping;
	unsigned int curFrame;
	int prevPriority;
	AnimationClip* prevClip_;
	float timer;

	AnimationClip clips[64];

} Animator;

typedef struct Collider
{
	Entity* entity;
	Transform* trans_;

	ColliderType type;
	Vector2 center;

	struct _Rectangle
	{
		float width;
		float height;
	} Rectangle;

	struct _Circle
	{
		float radius;
	} Circle;

	struct _Polygon
	{
		Vector2 points[12];
		uint numOfPoints;
	} Polygon;

	Vector2 truePoints[12];
	bool isTrigger;

} Collider;

typedef struct Rigidbody
{
	Entity* entity;
	Transform* trans_;
	Collider* colliders_[8];

	Vector2 prevPos;

	bool isStatic;

} Rigidbody;

void RemoveComponent(ComponentInfo* component_);

void DrawSprite(SpriteRenderer* renderer, Transform* trans);

void Transform_Translate(Transform* trans, int x, int y);
DSMessage Animator_Play(Animator* anim_, char name[], int variant, bool looping, int priority, bool canInteruptSelf);
//void Animator_CreateClip(Animator* anim_, char* name, int variant, GAMEBITMAP sources[]);