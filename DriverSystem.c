#include <Windows.h>
#include "DriverSystem.h"
#include "Renderer.h"
#include "Application.h"
#include "Cytools.h"
#include "Physics.h"

//Externs

GameManager gGameManager = { 0 };

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
		collider_->type = CT_Rectangle;

		collider_->Circle.radius = 16;
		collider_->Rectangle.height = 16;
		collider_->Rectangle.width = 16;

		collider_->Polygon.numOfPoints = 4;
		collider_->Polygon.points[0] = (Vector2){ 0, collider_->Rectangle.height };//TL
		collider_->Polygon.points[1] = (Vector2){ collider_->Rectangle.width, collider_->Rectangle.height };//TR
		collider_->Polygon.points[2] = (Vector2){ collider_->Rectangle.width, 0 };//BR
		collider_->Polygon.points[3] = (Vector2){ 0, 0 };//BL

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

void* GetComponent(DSID id, Entity* entity_)
{
	if (entity_ == NULL)
		return NULL;

	for (size_t i = 0; i < sizeof(entity_->components) / (sizeof(void*) * 2); i++)
	{
		if (entity_->components[i].dsID == id)
			return entity_->components[i].memory;
	}
	return NULL;
}

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
			float totalX = 0;
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

			collider_->center = (Vector2){ avgX,avgY };
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
			for (size_t i = 0; i < sizeof(gGameManager.allColliders) / sizeof(void*); i++)
			{
				if (rb_->colliders_[0] == gGameManager.allColliders[i])
					continue;
				if (gGameManager.allColliders[i] == NULL)
					continue;

				if (PolygonInPolyTrigger(rb_->colliders_[0]->truePoints, rb_->colliders_[0]->Polygon.numOfPoints,
					gGameManager.allColliders[i]->truePoints, gGameManager.allColliders[i]->Polygon.numOfPoints).collided)
				{
					Collision_Poly polyCol = PolygonInPolygon(rb_->colliders_[0]->truePoints, rb_->colliders_[0]->Polygon.numOfPoints,
						gGameManager.allColliders[i]->truePoints, gGameManager.allColliders[i]->Polygon.numOfPoints);
					Collision_Point collision = { 0 };

					//Get collision closeset to center
					float distToCenter = 0;
					for (size_t i = 0; i < polyCol.numOfCollisions; i++)
					{
						float curDist = Vec2Distance(polyCol.collisions[i].point, rb_->colliders_[0]->center);

						if (i == 0 || curDist < distToCenter)
						{
							distToCenter = curDist;
							collision = polyCol.collisions[i];
						}
					}
					//Transform* otherTrans_ = GetComponent(DSID_Transform, gGameManager.allColliders[i]->entity);


					float minDist = 0;
					int closestIndex = 0;
					Vector2 clamp = { 0 };
					for (size_t i = 0; i < collision.numOfEdges; i++)
					{
						clamp = (Vector2){
							ClampFloat(collision.point.x, collision.edges[i].start.x, 
								collision.edges[i].end.x),

							ClampFloat(collision.point.y, collision.edges[i].start.y, 
								collision.edges[i].end.y)
						};
						float curDist = Vec2Distance(clamp, collision.point);

						if (i == 0 || curDist < minDist)//Find the closest cross point
						{
							minDist = curDist;
							closestIndex = i;
						}
					}
					clamp = (Vector2){
							ClampFloat(collision.point.x, collision.edges[closestIndex].start.x,
								collision.edges[closestIndex].end.x),

							ClampFloat(collision.point.y, collision.edges[closestIndex].start.y,
								collision.edges[closestIndex].end.y)
					};

					if (minDist > 0)
						minDist = minDist;
					rb_->trans_->position = Vec2ToVec2INT(Vec2Add(Vec2INTToVec2(rb_->trans_->position), 
						Vec2MultiplyF(collision.edges[closestIndex].normal, minDist)));

					//Vector2 pntToCrossDir = Vec2Add(collision.point, 
						//Vec2MultiplyF(collision.crosses[closestIndex], -1));
						//Vec2MultiplyF(Vec2INTToVec2(rb_->trans_->position), -1));
					//Vec2Normalize(&pntToCrossDir);//These two lines cancel out
					//pntToCrossDir = Vec2MultiplyF(pntToCrossDir, minDist);

					Vector2 dir = { 1, 1 };
					//if (rb_->trans_->position.x > otherTrans_->position.x)
						//dir.x = -1;
					//pntToCrossDir = pntToCrossDir;
					Vector2 accel = Vec2Add(Vec2INTToVec2(rb_->trans_->position), Vec2MultiplyF(rb_->prevPos, -1));

					Component_OnUpdate(rb_->colliders_[0], DSID_Collider, UPDATE_MODE_NORMAL);
					
					//collision = PolygonInPolygon(rb_->colliders_[0]->truePoints, rb_->colliders_[0]->Polygon.numOfPoints,
						//gGameManager.allColliders[i]->truePoints, gGameManager.allColliders[i]->Polygon.numOfPoints);
				}//end of loop
			}
		}
		else if (MODE == UPDATE_MODE_LATE)
		{
			rb_->prevPos = Vec2INTToVec2(rb_->trans_->position);
		}
		else if (MODE == UPDATE_MODE_FIXED)
		{

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