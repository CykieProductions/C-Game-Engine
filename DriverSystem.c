#include <Windows.h>
#include "DriverSystem.h"
#include "Renderer.h"
#include "Application.h"
#include "Cytools.h"
//#include "Physics.h"
#include "Include/cute_c2.h"

void* DS_RegisterComponent(void* component_, void* allOfType[], size_t listSize)
{
	for (size_t i = 0; i < listSize / sizeof(component_); i++)
	{
		if (allOfType[i] == NULL)
		{
			allOfType[i] = component_;
			break;
		}
	}
}
//Maybe unneeded
/*void* DS_DeregisterComponent(void* component_, void* allOfType[], size_t listSize)
{
	for (size_t i = 0; i < listSize / sizeof(void*); i++)
	{
		if (allOfType[i] == component_)
		{
			allOfType[i] = NULL;
			break;
		}
	}
}*/

/// <summary>
/// 
/// </summary>
/// <param name="id">Determines the component type</param>
/// <param name="entity">Target entity's pointer</param>
/// <returns>The added component</returns>
void* AddComponent(DSID id, Entity* entity)
{
	void* component = NULL;
	//size_t length = 0;
	size_t compSize = 0;

	//static size_t lastSpriteRendererIndex = 0;//Could be used to avoid looping

	switch (id)
	{
	case DSID_Transform: {
		compSize = sizeof(Transform);
		if ((component = malloc(compSize)) == NULL)
			goto Exit;

		Transform* trans_ = (Transform*)component;

		trans_->position.x = 0;
		trans_->position.y = 0;
		trans_->positionZ = 0;
		trans_->scaleX = 1;
		trans_->scaleY = 1;

		DS_RegisterComponent(component, &gGameManager.allTransforms, sizeof(gGameManager.allTransforms));
		break;}

	case DSID_SpriteRenderer: {
		compSize = sizeof(SpriteRenderer);
		if ((component = malloc(compSize)) == NULL)
			goto Exit;

		SpriteRenderer* renderer_ = (SpriteRenderer*)component;

		//(*(SpriteRenderer*)component).entityInfo = entity;
		renderer_->transparencyPercent = 0;
		renderer_->defaultSprite = new_SpriteSliceData(
			SCE_PlayerSS,
			false,
			false,
			16, 16,
			16, 16,
			0, 0);

		renderer_->sortingLayer = SortLayer_YZ;
		renderer_->orderInLayer = 0;

		renderer_->flipX = false;
		renderer_->flipY = false;

		DS_RegisterComponent(component, &gGameManager.allSpriteRenderers, sizeof(gGameManager.allSpriteRenderers));
		/*length = sizeof(gGameManager.allSpriteRenderers) / sizeof(void*);
		for (size_t i = 0; i < length; i++)
		{
			if (gGameManager.allSpriteRenderers[i] == NULL)
			{
				gGameManager.allSpriteRenderers[i] = renderer_;
				break;
			}
		}*/
		break;}

	case DSID_Animator: {
		compSize = sizeof(Animator);
		if ((component = malloc(compSize)) == NULL)
			goto Exit;
		Animator* anim_ = (Animator*)component;

		anim_->renderer_ = GetComponent(DSID_SpriteRenderer, entity);

		if (anim_->renderer_ == NULL)
			MessageBox(NULL, L"Please add a renderer before adding an animator. The program may crash.", L"Warning!", MB_ICONINFORMATION | MB_OK);

		anim_->flipFrames = false;
		anim_->curFrame = 0;
		anim_->curClip_ = NULL;
		anim_->timer = 0;
		anim_->curPriority = -1;

		DS_RegisterComponent(component, &gGameManager.allAnimators, sizeof(gGameManager.allAnimators));

		break;}

	case DSID_Collider: {
		compSize = sizeof(Collider);
		if ((component = malloc(compSize)) == NULL)
			goto Exit;
		Collider* collider_ = (Collider*)component;

		collider_->isTrigger = false;
		/*collider_->type = CT_Rectangle;

		collider_->Circle.radius = 16;
		collider_->Rectangle.height = 16;
		collider_->Rectangle.width = 16;

		collider_->Polygon.numOfPoints = 4;
		collider_->Polygon.points[0] = (Vector2){ 0, collider_->Rectangle.height };//TL
		collider_->Polygon.points[1] = (Vector2){ collider_->Rectangle.width, collider_->Rectangle.height };//TR
		collider_->Polygon.points[2] = (Vector2){ collider_->Rectangle.width, 0 };//BR
		collider_->Polygon.points[3] = (Vector2){ 0, 0 };//BL*/

		collider_->trans_ = GetComponent(DSID_Transform, entity);
		Rigidbody* rb_ = GetComponent(DSID_Rigidbody, entity);
		if (rb_ != NULL)
		{
			for (size_t i = 0; i < sizeof(rb_->colliders_) / sizeof(Collider*); i++)
			{
				if (rb_->colliders_[i] == NULL)
				{
					rb_->colliders_[i] = collider_;
					break;
				}
			}
		}

		DS_RegisterComponent(component, &gGameManager.allColliders, sizeof(gGameManager.allColliders));

		break;}

	case DSID_Rigidbody: {
		compSize = sizeof(Rigidbody);
		if ((component = malloc(compSize)) == NULL)
			goto Exit;
		Rigidbody* rb_ = (Rigidbody*)component;

		FillArrayWithValue(rb_->colliders_, sizeof(rb_->colliders_), NULL, sizeof(NULL));
		rb_->isStatic = false;
		rb_->trans_ = GetComponent(DSID_Transform, entity);

		DS_RegisterComponent(component, &gGameManager.allRigidbodies, sizeof(gGameManager.allRigidbodies));

		break;}

	default:
		MessageBox(NULL, L"Attempted to register an invalid component!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(GetLastError());//idk if this always works
	}

	//No longer working (I don't know why)
	//Assign a parent entity to the component
	//EntityInfo MUST aways be the first member of the component struct for this to work
	//memcpy_s((EntityInfo*)component, sizeof(component), entity, sizeof(entity));
	
	//Since "Entity" will always be first no matter the component, this should work
	((SpriteRenderer*)component)->entity = entity;

	//Give the entity info on this new component
	for (size_t i = 0; i < sizeof(entity->components) / sizeof(ComponentInfo); i++)
	{
		if (entity->components[i].memory == NULL)
		{
			entity->components[i].dsID = id;
			entity->components[i].size = compSize;//For copying memory
			entity->components[i].memory = component;
			break;
		}
	}

Exit:
	return component;
}

//Get component moved to Component.c

//Filters what data will be copied
void DS_CopyComponent(void* dest_, Entity* entity_, ComponentInfo* original_)
{
	if (original_->memory == NULL || original_->size <= 0)
		return;

	memcpy_s(dest_, original_->size, original_->memory, original_->size);//copy everything

	switch (original_->dsID)
	{
	case DSID_SpriteRenderer: {
		SpriteRenderer* renderer_ = (SpriteRenderer*)dest_;
		renderer_->transparencyPercent = 0;

		LoadSpriteFromSheet(&renderer_->sprite, &gGameManager.spriteCache[renderer_->defaultSprite.sourceIndex],
			renderer_->defaultSprite.slicePosX, renderer_->defaultSprite.slicePosY,
			renderer_->defaultSprite.sliceWidth, renderer_->defaultSprite.sliceHeight,
			renderer_->defaultSprite.pivotX, renderer_->defaultSprite.pivotY, renderer_->defaultSprite.flipSprite);
		//renderer_->sprite.Memory = malloc(sizeof(void*));//Toggle this for a glitch effect!

		break;}

	case DSID_Animator: {
		Animator* anim_ = (Animator*)dest_;
		anim_->renderer_ = GetComponent(DSID_SpriteRenderer, entity_);//Revert the renderer to the right one
		anim_->curClip_ = NULL;//Don't copy the cur anim?
		anim_->curFrame = 0;//Restarts the current animation for no real reason

		break;}

	case DSID_Collider: {
		Collider* col_ = dest_;
		col_->trans_ = GetComponent(DSID_Transform, entity_);//Revert the Transform to the right one

		break;}

	case DSID_Rigidbody: {
		Rigidbody* rb_ = dest_;
		rb_->trans_ = GetComponent(DSID_Transform, entity_);
		//This may be pointless. Colliders add themselves to Rigidbody automatically
		for (size_t i = 0; i < sizeof(rb_->colliders_) / sizeof(Collider*); i++)
		{
			if (rb_->colliders_[i] != NULL)
				rb_->colliders_[i] = GetComponent(DSID_Collider, entity_);
		}

		break;}

	default:
		break;
	}

	((SpriteRenderer*)dest_)->entity = entity_;//Set the entity
}

void DS_Draw(void)
{
	//!/*Sort
	if (gTime.totalFramesRendered % 2 == 0)
	{
		size_t length = sizeof(gGameManager.allSpriteRenderers) / sizeof(void*);
		for (size_t i = 0; i < length; ++i)
		{
			if (gGameManager.allSpriteRenderers[i] == NULL)
				continue;
			SpriteRenderer* sp_ = NULL;
			for (size_t j = i + 1; j < length; ++j)//Search from i onward
			{
				if (gGameManager.allSpriteRenderers[j] == NULL)
					continue;

				//TODO This works, but it may need to be redone for better z sorting
				if (gGameManager.allSpriteRenderers[i]->sortingLayer == SortLayer_YZ)
				{
					Transform* Itrans = GetComponent(DSID_Transform, gGameManager.allSpriteRenderers[i]->entity);
					gGameManager.allSpriteRenderers[i]->orderInLayer = Itrans->position.y * (Itrans->positionZ == 0 ? 1 : Itrans->positionZ);
				}
				if (gGameManager.allSpriteRenderers[j]->sortingLayer == SortLayer_YZ)
				{
					Transform* Jtrans = GetComponent(DSID_Transform, gGameManager.allSpriteRenderers[j]->entity);
					gGameManager.allSpriteRenderers[j]->orderInLayer = Jtrans->position.y * (Jtrans->positionZ == 0 ? 1 : Jtrans->positionZ);
				}

				if (gGameManager.allSpriteRenderers[i]->sortingLayer > gGameManager.allSpriteRenderers[j]->sortingLayer ||
					(gGameManager.allSpriteRenderers[i]->sortingLayer == gGameManager.allSpriteRenderers[j]->sortingLayer &&
						gGameManager.allSpriteRenderers[i]->orderInLayer > gGameManager.allSpriteRenderers[j]->orderInLayer))
				{
					//Swap [i] and [j]
					sp_ = gGameManager.allSpriteRenderers[i];
					gGameManager.allSpriteRenderers[i] = gGameManager.allSpriteRenderers[j];
					gGameManager.allSpriteRenderers[j] = sp_;
					break;
				}
			}
		}
	}
	//*/

	//!Draw
	for (size_t i = 0; i < sizeof(gGameManager.allSpriteRenderers) / sizeof(void*); i++)
	{
		if (gGameManager.allSpriteRenderers[i] == NULL)
			continue;

		Entity* entity_ = gGameManager.allSpriteRenderers[i]->entity;//gGameManager.allSpriteRenderers[i]->entity;
		Transform* trans_ = GetComponent(DSID_Transform, entity_);

		if (trans_ != NULL && gGameManager.allSpriteRenderers[i] != NULL)
			DrawSprite(gGameManager.allSpriteRenderers[i], trans_);
	}
}

void DS_Update(const short MODE)
{
	//Animator
	for (size_t i = 0; i < sizeof(gGameManager.allAnimators) / sizeof(void*); i++)
	{
		if (gGameManager.allAnimators[i] == NULL)
			continue;
		if (gGameManager.allAnimators[i]->entity == NULL)
		{
			free(gGameManager.allAnimators[i]);
			gGameManager.allAnimators[i] = NULL;
			continue;
		}
		Component_OnUpdate(gGameManager.allAnimators[i], DSID_Animator, MODE);
	}
	//Rigidbody
	for (size_t i = 0; i < sizeof(gGameManager.allRigidbodies) / sizeof(void*); i++)
	{
		if (gGameManager.allRigidbodies[i] == NULL)
			continue;
		if (gGameManager.allRigidbodies[i]->entity == NULL)
		{
			free(gGameManager.allRigidbodies[i]);
			gGameManager.allRigidbodies[i] = NULL;
			continue;
		}
		Component_OnUpdate(gGameManager.allRigidbodies[i], DSID_Rigidbody, MODE);
	}
	//Collider
	for (size_t i = 0; i < sizeof(gGameManager.allColliders) / sizeof(void*); i++)
	{
		if (gGameManager.allColliders[i] == NULL)
			continue;
		if (gGameManager.allColliders[i]->entity == NULL)
		{
			free(gGameManager.allColliders[i]);
			gGameManager.allColliders[i] = NULL;
			continue;
		}
		Component_OnUpdate(gGameManager.allColliders[i], DSID_Collider, MODE);
	}
}

/// <summary>
/// Updates the component based on the given ID and mode.
/// </summary>
/// <param name="component">A pointer to the component to update.</param>
/// <param name="id">The ID of the component type.</param>
/// <param name="MODE">The update mode (start, normal, late, or fixed).</param>
void Component_OnUpdate(void* component, DSID id, const short MODE)
{
	if (component == NULL)
		return;

	Entity* entity_ = ((Transform*)component)->entity;
	switch (id)
	{
	case DSID_Animator: {
		Animator* anim_ = (Animator*)component;

		if (anim_->renderer_ == NULL)
			anim_->renderer_ = GetComponent(DSID_SpriteRenderer, entity_);

		if (MODE == UPDATE_MODE_NORMAL)
		{
			if (anim_->curClip_ != NULL)
			{
				if (anim_->curClip_->delays[anim_->curFrame] <= 0)//use default
					anim_->curClip_->delays[anim_->curFrame] = anim_->curClip_->defaultDelay;

				anim_->timer += gTime.deltaTime;
				if (anim_->timer > anim_->curClip_->delays[anim_->curFrame])
				{
					SpriteSliceData* curFrame = &anim_->curClip_->frames[anim_->curFrame];
					LoadSpriteFromSheet(&anim_->renderer_->sprite, &gGameManager.spriteCache[curFrame->sourceIndex],
						curFrame->slicePosX, curFrame->slicePosY, curFrame->sliceWidth, curFrame->sliceHeight,
						curFrame->pivotX, curFrame->pivotY, anim_->flipFrames);// ^ curFrame->flipSprite);

					anim_->curFrame++;
					anim_->timer = 0;

					//End of Animation
					if (anim_->curFrame >= anim_->curClip_->frameCount)
					{
						anim_->prevPriority = anim_->curPriority;
						if (anim_->curClip_ != NULL)//keep a reference to the last clip played
							anim_->prevClip_ = anim_->curClip_;

						//Don't loop
						if (!anim_->nowLooping)
						{
							anim_->curClip_ = NULL;
							anim_->flipFrames = false;
						}
						anim_->curFrame = 0;
					}
				}
			}
		}
		else if (MODE == UPDATE_MODE_LATE)
		{

		}
		else if (MODE == UPDATE_MODE_FIXED)
		{

		}
		break;}

	case DSID_Collider: {
		Collider* collider_ = component;
		if (MODE == UPDATE_MODE_NORMAL || MODE == UPDATE_MODE_START)
		{
			/*float totalX = 0;
			float totalY = 0;
			for (size_t i = 0; i < collider_->Polygon.numOfPoints; i++)
			{
				collider_->truePoints[i] = Vec2Add(collider_->Polygon.points[i], (Vector2)
				{
					collider_->trans_->position.x, collider_->trans_->position.y
				});

				totalX += collider_->truePoints[i].x;
				totalY += collider_->truePoints[i].y;
			}

			float avgX = totalX / collider_->Polygon.numOfPoints;
			float avgY = totalY / collider_->Polygon.numOfPoints;

			collider_->center = (Vector2){ avgX,avgY };*/
			
			//! Will likely have to manually factor in the scale
			Collider_UpdateTrueBounds(collider_, NULL);

		}
		else if (MODE == UPDATE_MODE_LATE)
		{

		}
		else if (MODE == UPDATE_MODE_FIXED)
		{

		}
		break;}

	case DSID_Rigidbody: {
		Rigidbody* rb_ = component;
		if (MODE == UPDATE_MODE_START)
		{
			rb_->prevPos = Vec2INTToVec2(rb_->trans_->position);
		}
		else if (MODE == UPDATE_MODE_NORMAL)
		{
			
		}
		else if (MODE == UPDATE_MODE_LATE)
		{
			rb_->prevPos = Vec2INTToVec2(rb_->trans_->position);
		}
		else if (MODE == UPDATE_MODE_FIXED)
		{
			//Check if we've moved since the last frame
			if (rb_->trans_->position.x != rb_->prevPos.x || rb_->trans_->position.y != rb_->prevPos.y)
			{
				//! Collision Detection
				//Rigidbody_CollisionCheck(rb_);
			}
		}
		break;}

	default: {
		if (MODE == UPDATE_MODE_NORMAL)
		{

		}
		else if (MODE == UPDATE_MODE_LATE)
		{

		}
		else if (MODE == UPDATE_MODE_FIXED)
		{

		}
		break;}
	}
}