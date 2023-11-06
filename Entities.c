#include "Application.h"
//#include "Renderer.h"
#include "Cytools.h"
#include "Entities.h"
#include "Components.h"

Transform* new_TileBrush(GAMEBITMAP* sprite_, SpriteSliceData sliceData)
{
	Entity* brush_ = malloc(sizeof(Entity));
	if (brush_ == NULL)
		return NULL;
	//brush_->TYPE = OT_Tile;
	brush_->TYPE = OT_Gizmo;
	DS_RegisterEntity(brush_);

	brush_->id = -(long)((gTime.totalFramesRendered + gTime.elapsedMicroSeconds) + rand());
	brush_->disabled = false;
	FillArrayWithValue(brush_->components, sizeof(brush_->components), NULL, sizeof(ComponentInfo));

	//DS_RegisterEntity(entity_);

	SpriteRenderer* rend_ = AddComponent(DSID_SpriteRenderer, brush_);
	rend_->defaultSprite = sliceData;
	if (sliceData.useSourceDirectly)
		rend_->sprite = *sprite_;
	else
	{
		LoadSpriteFromSheet(&rend_->sprite, sprite_, sliceData.slicePosX, sliceData.slicePosY,
			sliceData.sliceWidth, sliceData.sliceHeight,
			sliceData.pivotX, sliceData.pivotY, sliceData.flipSprite);
	}

	return AddComponent(DSID_Transform, brush_);
}

Transform* PaintTile(Entity* brush_)
{
	//SpriteRenderer* bRend_ = brush_->components[0].memory;
	//Transform* bTrans_ = brush_->components[1].memory;

	brush_->TYPE = OT_Tile;
	Entity* tile_ = CloneEntity(brush_);
	brush_->TYPE = OT_Gizmo;

	return tile_->components[1].memory;
}

void DS_RegisterTile(Entity* tile_)
{
	for (size_t i = 0; i < sizeof(gGameManager.allTiles) / sizeof(void*); i++)
	{
		if (gGameManager.allTiles[i] == NULL)
		{
			gGameManager.allTiles[i] = tile_;
			gGameManager.numOfTiles++;
			break;
		}
	}
}
void DS_RegisterEntity(Entity* entity_)
{
	if (entity_->TYPE == OT_Tile)
	{
		DS_RegisterTile(entity_);
		return;
	}
	if (entity_->TYPE == OT_Gizmo)
	{
		//DS_RegisterGizmo(entity_);
		return;
	}


	for (size_t i = 0; i < sizeof(gGameManager.allEntities) / sizeof(void*); i++)
	{
		if (gGameManager.allEntities[i] == NULL)
		{
			gGameManager.allEntities[i] = entity_;
			gGameManager.numOfEntities++;
			break;
		}
	}
}
bool DestroyEntity(Entity* entity_)
{
	if (entity_ == NULL)
		return false;

	for (size_t i = 0; i < sizeof(entity_->components) / sizeof(ComponentInfo); i++)
	{
		RemoveComponent(&entity_->components[i]);
	}
	//DS_DeregisterEntity
	if (entity_->TYPE == OT_Tile)
	{
		for (size_t i = 0; i < sizeof(gGameManager.allTiles) / sizeof(void*); i++)
		{
			if (entity_ == NULL || gGameManager.allTiles[i] == NULL)
				continue;

			if (gGameManager.allTiles[i]->id == entity_->id)
			{
				gGameManager.allTiles[i] = NULL;
				gGameManager.numOfTiles--;
				return true;//Successfully removed entity
			}
		}
	}
	else
	{
		for (size_t i = 0; i < sizeof(gGameManager.allEntities) / sizeof(void*); i++)
		{
			if (entity_ == NULL || gGameManager.allEntities[i] == NULL)
				continue;

			if (gGameManager.allEntities[i]->id == entity_->id)
			{
				gGameManager.allEntities[i] = NULL;
				gGameManager.numOfEntities--;
				return true;//Successfully removed entity
			}
		}
	}

	return false;
}


Entity* InitEntity(ObjectType type)
{
	if (gGameManager.numOfEntities >= ENTITY_LIMIT)
		return NULL;

	Entity* entity_ = malloc(sizeof(Entity));
	if (entity_ == NULL)
		return NULL;

	entity_->id = (gTime.totalFramesRendered + gTime.elapsedMicroSeconds) + rand();
	entity_->disabled = false;
	entity_->TYPE = type;
	FillArrayWithValue(entity_->components, sizeof(entity_->components), NULL, sizeof(ComponentInfo));

	DS_RegisterEntity(entity_);

	return entity_;
}

//Returns a pointer to the clone
Entity* CloneEntity(Entity* original_)
{
	if (original_->TYPE == OT_Entity && gGameManager.numOfEntities >= ENTITY_LIMIT)
		return NULL;
	if (original_->TYPE == OT_Tile && gGameManager.numOfTiles >= TILE_LIMIT)
		return NULL;

	Entity* clone_ = malloc(sizeof(*original_));
	memcpy_s(clone_, sizeof(*original_), original_, sizeof(*original_));//copy the data
	
	if (clone_ != NULL)
	{
		clone_->id = ((gTime.totalFramesRendered + gTime.elapsedMicroSeconds) / 5) + rand() + 1;//New id

		//Remove the cloned components
		FillArrayWithValue(clone_->components, sizeof(clone_->components), NULL, sizeof(ComponentInfo));
		//Re-add the components
		for (size_t i = 0; i < sizeof(clone_->components) / sizeof(ComponentInfo); i++)
		{
			if (original_->components[i].memory == NULL)
				continue;
			//clone_->components[i].memory = malloc(clone_->components[i].size);

			//TIP: if you store a struct's size in a variable, DO NOT put that variable into a "sizeof()"
			//I spent too long trying to fix that!

			AddComponent(original_->components[i].dsID, clone_);//Add a fresh component (for proper registry)
			DS_CopyComponent(clone_->components[i].memory, clone_, &original_->components[i]);//Copy needed data into fresh component

			/*Copy all the original component's data //Too much data got copied (ex: sprite ptr)
			memcpy_s(clone_->components[i].memory, original_->components[i].size,
				original_->components[i].memory, original_->components[i].size);*/
			
			//Revert the entity reference to the clone//Moved to DS_CopyComponent
			//if (clone_->components[i].memory != NULL)
				//((SpriteRenderer*)clone_->components[i].memory)->entity = clone_;
		}

		//if (!original_->IS_TILE)//Auto registers as a tile
			DS_RegisterEntity(clone_);//Register the clone
	}
	return clone_;
}

void InitPlayer(int sPosX, int sPosY, int sSclX, int sSclY)
{
	if (gGameManager.numOfEntities >= ENTITY_LIMIT)
		exit(1);//Not enough memory for player

	player.id = 420;
	player.TYPE = OT_Entity;
	DS_RegisterEntity(&player);

	(Transform*)AddComponent(DSID_Transform, &player);
	Transform* trans_ = GetComponent(DSID_Transform, &player);
	AddComponent(DSID_SpriteRenderer, &player);
	Animator* anim_ = AddComponent(DSID_Animator, &player);
	AddComponent(DSID_Rigidbody, &player);

	trans_->position.x = sPosX;
	trans_->position.y = sPosY;
	trans_->scaleX = sSclX;
	trans_->scaleY = sSclY;

	Collider* col_ = AddComponent(DSID_Collider, &player);
	col_->isTrigger = false;
	//Collider_SetAABB(col_, 16, 16, 0, 0);
	Collider_SetCircle(col_, 8, 0, 0);

	/*int colSize = 16;
	col_->bounds.circle.r = colSize / 2;
	col_->bounds.circle.p.x = colSize / 2;
	col_->bounds.circle.p.y = colSize / 2;

	col_->bounds.aabb.max.x = colSize;
	col_->bounds.aabb.max.y = colSize;
	col_->bounds.aabb.min.x = 0;
	col_->bounds.aabb.min.y = 0;

	col_->bounds.polygon.count = 4;
	col_->bounds.polygon.verts[0].x = 0;
	col_->bounds.polygon.verts[0].y = 0;
	col_->bounds.polygon.verts[1].x = colSize;
	col_->bounds.polygon.verts[1].y = 0;
	col_->bounds.polygon.verts[2].x = colSize;
	col_->bounds.polygon.verts[2].y = colSize;
	col_->bounds.polygon.verts[3].x = 0;
	col_->bounds.polygon.verts[3].y = colSize;*/

	//player.renderer.flipX = true;

	//char filePath[128];
	//ConcatStrings(filePath, sizeof(filePath), gDebug.defaultDataPath, "char_spritesheet.bmp");//"char_idle_00_d.bmp");
	//if (Load32BppBitmapFromFile(&((SpriteRenderer*)GetComponent(DSID_SpriteRenderer, &player.info))->sprite, filePath) != ERROR_SUCCESS)
	//Moved spritesheet setup to Main

	SpriteRenderer* renderer_ = (SpriteRenderer*)GetComponent(DSID_SpriteRenderer, &player);

	renderer_->defaultSprite = new_SpriteSliceData(
		SCE_PlayerSS,
		false,
		false,
		16 * 9, 16,
		16, 16,
		0, 0);

	LoadSpriteFromSheet(&renderer_->sprite, &gGameManager.spriteCache[SCE_PlayerSS],
		16 * 9, 16, 16, 16, 0, 0, false);

	player.baseSpeed = 100;

	//Create Animations
	FillArrayWithValue(&anim_->clips, sizeof(anim_->clips), NULL, sizeof(AnimationClip));
	int cIndex = 0;
	AnimationClip* clip_ = &anim_->clips[cIndex];

#pragma region Idle Anim
	SetString(clip_->name, "Idle", sizeof(clip_->name), sizeof("Idle"));
	clip_->variant = ANIM_DOWN;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		//frameData->source = gGameManager.playerSpriteSheet;
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 128 + (16 * i);
		frameData->slicePosY = 16;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Idle", sizeof(clip_->name), sizeof("Idle"));
	clip_->variant = ANIM_LEFT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 128 + (16 * i);
		frameData->slicePosY = 16 + 16;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Idle", sizeof(clip_->name), sizeof("Idle"));
	clip_->variant = ANIM_RIGHT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 128 + (16 * i);
		frameData->slicePosY = 16 + 32;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Idle", sizeof(clip_->name), sizeof("Idle"));
	clip_->variant = ANIM_UP;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 128 + (16 * i);
		frameData->slicePosY = 16 + 48;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}
#pragma endregion

#pragma region Walking Anim
	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Walking", sizeof(clip_->name), sizeof("Walking"));
	clip_->variant = ANIM_DOWN;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 16 + (16 * i);
		frameData->slicePosY = 16;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Walking", sizeof(clip_->name), sizeof("Walking"));
	clip_->variant = ANIM_LEFT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 16 + (16 * i);
		frameData->slicePosY = 16 + 16;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Walking", sizeof(clip_->name), sizeof("Walking"));
	clip_->variant = ANIM_RIGHT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 16 + (16 * i);
		frameData->slicePosY = 16 + 32;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Walking", sizeof(clip_->name), sizeof("Walking"));
	clip_->variant = ANIM_UP;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.1f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 16 + (16 * i);
		frameData->slicePosY = 16 + 48;
		frameData->sliceWidth = 16;
		frameData->sliceHeight = 16;
		frameData->pivotX = 0;
		frameData->pivotY = 0;
	}
#pragma endregion

#pragma region Attack Anim
	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Attack", sizeof(clip_->name), sizeof("Attack"));
	clip_->variant = ANIM_DOWN;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.032f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 0 + (48 * i);
		frameData->slicePosY = 208;
		frameData->sliceWidth = 48;
		frameData->sliceHeight = 40;
		frameData->pivotX = 16;
		frameData->pivotY = 0;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Attack", sizeof(clip_->name), sizeof("Attack"));
	clip_->variant = ANIM_RIGHT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.032f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 0 + (48 * i);
		frameData->slicePosY = 208 + 120;
		frameData->sliceWidth = 48;
		frameData->sliceHeight = 40;
		frameData->pivotX = 16;
		frameData->pivotY = -8;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Attack", sizeof(clip_->name), sizeof("Attack"));
	clip_->variant = ANIM_LEFT;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.032f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 0 + (48 * i);
		frameData->slicePosY = 208 + 64;
		frameData->sliceWidth = 48;
		frameData->sliceHeight = 40;
		frameData->pivotX = 16;
		frameData->pivotY = -16;
	}

	cIndex++;
	clip_ = &anim_->clips[cIndex];
	SetString(clip_->name, "Attack", sizeof(clip_->name), sizeof("Attack"));
	clip_->variant = ANIM_UP;
	clip_->frameCount = 6;
	clip_->defaultDelay = 0.032f;
	for (size_t i = 0; i < clip_->frameCount; i++)
	{
		SpriteSliceData* frameData = &clip_->frames[i];
		frameData->sourceIndex = SCE_PlayerSS;
		frameData->slicePosX = 0 + (48 * i);
		frameData->slicePosY = 208 + 32;
		frameData->sliceWidth = 48;
		frameData->sliceHeight = 40;
		frameData->pivotX = 16;
		frameData->pivotY = -16;
	}
#pragma endregion
}