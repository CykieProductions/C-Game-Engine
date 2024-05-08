#include "Components.h"
#include "Cytools.h"
//#include "DriverSystem.h"
#include "Renderer.h"

GameManager gGameManager = { 0 };

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
bool HasComponent(DSID id, Entity* entity_)
{
	if (entity_ == NULL)
		return false;

	for (size_t i = 0; i < sizeof(entity_->components) / (sizeof(void*) * 2); i++)
	{
		if (entity_->components[i].dsID == id)
			return true;
	}
	return false;
}
void RemoveComponent(ComponentInfo* component_)
{
	/*switch (component_->dsID)
	{
	default:
		break;
	}*/

	//free(component_->memory);
	memset(component_->memory, NULL, component_->size);
	memcpy_s(component_, sizeof(ComponentInfo), &(ComponentInfo){0}, sizeof(ComponentInfo));
}


SpriteSliceData new_SpriteSliceData(SpriteCacheEntries sourceIndex, bool useSourceDirectly, bool flipSprite,
	int slicePosX, int slicePosY, int sliceWidth, int sliceHeight, int pivotX, int pivotY)
{
	SpriteSliceData ssd = { 0 };
	
	ssd.sourceIndex = sourceIndex;
	ssd.useSourceDirectly = useSourceDirectly;
	ssd.flipSprite = flipSprite;
	ssd.slicePosX = slicePosX;
	ssd.slicePosY = slicePosY;
	ssd.sliceWidth = sliceWidth;
	ssd.sliceHeight = sliceHeight;
	ssd.pivotX = pivotX;
	ssd.pivotY = pivotY;

	return ssd;
}


DSMessage Animator_Play(Animator* anim_, char name[], int variant, bool looping, int priority, bool canInteruptSelf)
{
	if (anim_->curClip_ != NULL)
	{
		if ((priority < anim_->curPriority && priority < 1) || (priority <= anim_->curPriority && priority >= 1))//not enough priority to override
			return DSMSG_ANIM_FAILED;
		if (!canInteruptSelf && CompareStrings(anim_->curClip_->name, name) && anim_->curClip_->variant == variant)//curclip is trying to play over itself
			return DSMSG_ANIM_CONTINUED;//the anim is playing
	}

	anim_->prevPriority = anim_->curPriority;
	anim_->curPriority = -1;//if no curclip then reset priority

	for (size_t i = 0; i < sizeof(anim_->clips) / sizeof(AnimationClip); i++)
	{
		if (CompareStrings(anim_->clips[i].name, name) && anim_->clips[i].variant == variant)
		{
			if (anim_->curClip_ != NULL && CompareStrings(anim_->curClip_->name, name) && anim_->curClip_->variant != variant)//just a different variant
			{
			}
			else
				anim_->curFrame = 0;

			anim_->prevClip_ = anim_->curClip_;
			anim_->curClip_ = &anim_->clips[i];
			anim_->nowLooping = looping;
			anim_->curPriority = priority;
			return DSMSG_ANIM_PLAYED;
		}
	}
	return DSMSG_ANIM_FAILED;//Failed to find animation
}

void DrawSprite(SpriteRenderer* renderer, Transform* trans)
{
	if (trans->entity->disabled == true)
		return;

	GAMEBITMAP* sprite = &renderer->sprite;
	int startingScreenPixel = GetStartingPixel(RES_WIDTH, RES_HEIGHT,
		-trans->position.x + sprite->pivotX, trans->position.y + sprite->pivotY);//Why must X be (-)?
	int startingSpritePixel = GetStartingPixel(sprite->bitmapInfo.bmiHeader.biWidth, sprite->bitmapInfo.bmiHeader.biHeight, 0, 0);


	int memOffset = 0;
	int spriteOffset = 0;
	Color32 pixel = { 0 };
	Color32 backgroundPixel = { 0 };

	size_t yPixScreen = 0;
	size_t xPixScreen = 0;

	size_t loopStartY = 0;
	size_t yPix = loopStartY;
	size_t loopStartX = 0;
	size_t xPix = loopStartX;
	bool conditionY = loopStartY < sprite->bitmapInfo.bmiHeader.biHeight;
	bool conditionX = loopStartX < sprite->bitmapInfo.bmiHeader.biWidth;
	short dirY = 1;
	short dirX = 1;

	//Should the sprite be drawn upside down?
	if ((renderer->flipY == true && trans->scaleY > 0) || (renderer->flipY == false && trans->scaleY < 0))
	{
		loopStartY = sprite->bitmapInfo.bmiHeader.biHeight;
		dirY = -1;
	}
	//Should the sprite be drawn backwards
	if ((renderer->flipX == true && trans->scaleX > 0) || (renderer->flipX == false && trans->scaleX < 0))
	{
		loopStartX = sprite->bitmapInfo.bmiHeader.biWidth;//Am I missing a -1
		dirX = -1;
	}


	for (yPix = loopStartY; conditionY == true; yPix += 1 * dirY)
	{
		yPixScreen++;
		conditionY = yPix < sprite->bitmapInfo.bmiHeader.biHeight - 1;

		//Setup Row//
		xPixScreen = 0;
		xPix = loopStartX;
		if (dirX > 0)
			conditionX = xPix < sprite->bitmapInfo.bmiHeader.biWidth - 1;
		else
			conditionX = xPix > 0;

		for (xPix = loopStartX; conditionX == true; xPix += 1 * dirX)
		{
			xPixScreen++;
			//flipped?
			if (dirX > 0)//Not flipped
				conditionX = xPix < sprite->bitmapInfo.bmiHeader.biWidth - 1;
			else
				conditionX = xPix > 0;

			//Screen Wraping & Out of bounds protection///////////////////
			{
				if ((trans->position.x - sprite->pivotX) + xPixScreen < 0 || (trans->position.x - sprite->pivotX) + xPixScreen >= RES_WIDTH)
					continue;

				size_t pixPos = startingScreenPixel + xPixScreen - (RES_WIDTH * yPixScreen);
				if (pixPos < 0 || pixPos >= RES_WIDTH * RES_HEIGHT)
					continue;
			}
			//////////////////////////////////////////////////////////////

			memOffset = startingScreenPixel + xPixScreen - (RES_WIDTH * yPixScreen);
			spriteOffset = startingSpritePixel + xPix - (sprite->bitmapInfo.bmiHeader.biWidth * yPix);

			//Copy pixel data from sprite/////////////////////////////////
			memcpy_s(&pixel, sizeof(Color32), (Color32*)sprite->Memory + spriteOffset, sizeof(Color32));
			pixel.alpha *= (1.0f - renderer->transparencyPercent);

			//Extra Processing////////////////////////////////////////////
			//Skip transparent pixels
			if (/*&pixel == NULL || */ pixel.alpha == 0)
			{
				continue;
				/*pixel.blue = 255;
				pixel.green = 0;
				pixel.red = 255;*/
			}
			AlphaBlendColor32(&pixel, (Color32*)gBackBuffer.Memory + memOffset, AC_SrcOver);

			//Get the background pixel (for alpha blending)
			//memcpy_s(&backgroundPixel, sizeof(Pixel32), (Pixel32*)gBackBuffer.Memory + memOffset, sizeof(Pixel32));

			//Paste pixel into buffer/////////////////////////////////////
			memcpy_s((Color32*)gBackBuffer.Memory + memOffset, sizeof(Color32), &pixel, sizeof(Color32));
		}
	}
}

//todo implement rotation
c2x* TransformToC2X(Transform* trans_)
{
	c2x c2Transform = { 0 };
	c2Transform.p.x = trans_->position.x;
	c2Transform.p.y = trans_->position.y;

	c2Transform.r.s = 0;//trans->rotation.sin;
	c2Transform.r.c = 0;//trans->rotation.cos;
	
	return &c2Transform;
}
void Transform_TranslateNoCollision(Transform* trans_, int x, int y)
{
	if (x == 0 && y == 0)
		return;
	trans_->position.x += x;
	trans_->position.y -= y;
}
void Transform_Translate(Transform* trans_, int x, int y)
{
	if ((*trans_->entity).disabled == true)
		return;

	Collision collision = { 0 };

	//check for collisions
	if (x != 0 || y != 0)
	{
		Rigidbody* rb_ = NULL;
		if ((rb_ = GetComponent(DSID_Rigidbody, trans_->entity)) != NULL)
		{
			collision = Rigidbody_OffCollisionCheck(rb_, x, y);
		}
	}

	//if (!collision.collided)
	Transform_TranslateNoCollision(trans_, x - RoundUpAbsF(collision.resolve.x), y + RoundUpAbsF(collision.resolve.y));
}

Collision Rigidbody_CollisionCheck(Rigidbody* rb_)
{
	return Rigidbody_OffCollisionCheck(rb_, 0, 0);
}
Collision Rigidbody_OffCollisionCheck(Rigidbody* rb_, int xOff, int yOff)
{
	Collision result = { 0 };
	for (size_t i = 0; i < sizeof(gGameManager.allColliders) / sizeof(void*); i++)
	{
		if (gGameManager.allColliders[i] == NULL)
			continue;
		if (rb_->colliders_[0] == NULL)
			continue;
		if (rb_->colliders_[0] == gGameManager.allColliders[i])//current collider is the same as rb's collider
			continue;

		//Trigger detection
		if (rb_->colliders_[0]->isTrigger || gGameManager.allColliders[i]->isTrigger)
		{
			int collided = c2Collided(Collider_GetC2(rb_->colliders_[0], 0), TransformToC2X(rb_->trans_), rb_->colliders_[0]->type,
				Collider_GetC2(gGameManager.allColliders[i], 0),
				TransformToC2X(gGameManager.allColliders[i]->trans_), gGameManager.allColliders[i]->type);
			if (collided)
			{
				c2x* _ = TransformToC2X(gGameManager.allColliders[i]->trans_);
				//todo Implement trigger response
				result.collided = true;
			}
		}
		else
		{
			c2Manifold manifold = { 0 };
			int iterations = COL_ITERATION_LIMIT;

			do
			{
				Transform tmpT = { 
					.entity = rb_->entity,
					.position.x = rb_->trans_->position.x + xOff,
					.position.y = rb_->trans_->position.y - yOff, //y is reversed
					.positionZ = rb_->trans_->positionZ,
					.scaleX = rb_->trans_->scaleX,
					.scaleY = rb_->trans_->scaleY
				};
				//Cast the True Bounds ahead
				Collider_UpdateTrueBounds(rb_->colliders_[0], &tmpT);
				c2Collide(Collider_GetC2(rb_->colliders_[0], 0), TransformToC2X(&tmpT), rb_->colliders_[0]->type,
					Collider_GetC2(gGameManager.allColliders[i], 0),
					TransformToC2X(gGameManager.allColliders[i]->trans_), gGameManager.allColliders[i]->type, &manifold);

				if (manifold.count > 0)
				{
					float max_depth = 0;
					for (int j = 0; j < sizeof(manifold.depths) / sizeof(float); j++)
					{
						if (max_depth < manifold.depths[j])
							max_depth = manifold.depths[j];
					}
					if (max_depth != 0)
					{
						result.manifold = manifold;
						result.resolve = C2VToVec2(c2Mulvs(manifold.n, max_depth));
						if (result.collided == false)
							result.collided = true;
						//Transform_TranslateNoCollision(rb_->trans_, -(int)offset.x, (int)offset.y);
					}
					else
					{
						iterations = 0;
						break;
					}
				}
				else
				{
					iterations = 0;
					break;
				}
				iterations--;
			} while (iterations > 0);
		}
	}

	//Reset the collider's true bounds
	Collider_UpdateTrueBounds(rb_->colliders_[0], NULL);
	return result;
}

/// <summary>
/// Get the Collider's "cute_c2" bounds data
/// </summary>
/// <param name="collider_">Target collider's address</param>
/// <param name="memSize">Optional parameter for the c2 data size</param>
/// <returns>The address of the trueData bounds</returns>
void* Collider_GetC2(Collider* collider_, _Out_ int memSize)
{
	switch (collider_->type)
	{
	case C2_TYPE_AABB:
		memSize = sizeof(c2AABB);
		return &(collider_->trueData.aabb);

	case C2_TYPE_CIRCLE:
		memSize = sizeof(c2Circle);
		return &(collider_->trueData.circle);

	case C2_TYPE_CAPSULE:
		memSize = sizeof(c2Capsule);
		return &(collider_->trueData.capsule);

	case C2_TYPE_POLY:
		memSize = sizeof(c2Poly);
		return &(collider_->bounds.polygon);

	default:break;
	}
}
void Collider_SetAABB(Collider* collider_, int sizeX, int sizeY, int offsetX, int offsetY)
{
	collider_->type = C2_TYPE_AABB;
	collider_->bounds.aabb.min.x = offsetX + 0;
	collider_->bounds.aabb.min.y = offsetY + 0;
	collider_->bounds.aabb.max.x = offsetX + sizeX;
	collider_->bounds.aabb.max.y = offsetY + sizeY;
}
void Collider_SetCircle(Collider* collider_, int radius, int offsetX, int offsetY)
{
	collider_->type = C2_TYPE_CIRCLE;
	collider_->bounds.circle.r = radius;
	collider_->bounds.circle.p.x = radius + offsetX;
	collider_->bounds.circle.p.y = radius + offsetY;
}
void Collider_SetCapsule(Collider* collider_, int radius, c2v start, c2v end, int offsetX, int offsetY)
{
	collider_->type = C2_TYPE_CAPSULE;
	collider_->bounds.capsule.r = radius;
	collider_->bounds.capsule.a = c2V(start.x + offsetX, start.y + offsetY);
	collider_->bounds.capsule.b = c2V(end.x + offsetX, end.y + offsetY);
}


/// <summary>
/// Updates trueData with Transform offset
/// </summary>
/// <param name="collider_">Target collider</param>
/// <param name="trans_">Leave NULL to use the Entity's transform</param>
void Collider_UpdateTrueBounds(Collider* collider_, Transform* trans_)
{
	if (trans_ == NULL)
		trans_ = collider_->trans_;

	switch (collider_->type)
	{
	case C2_TYPE_AABB:
		collider_->trueData.aabb.min.x = collider_->bounds.aabb.min.x + trans_->position.x;
		collider_->trueData.aabb.min.y = collider_->bounds.aabb.min.y + trans_->position.y;
		collider_->trueData.aabb.max.x = collider_->bounds.aabb.max.x + trans_->position.x;
		collider_->trueData.aabb.max.y = collider_->bounds.aabb.max.y + trans_->position.y;
		break;

	case C2_TYPE_CIRCLE:
		collider_->trueData.circle.p.x = collider_->bounds.circle.p.x + trans_->position.x;
		collider_->trueData.circle.p.y = collider_->bounds.circle.p.y + trans_->position.y;
		collider_->trueData.circle.r = collider_->bounds.circle.r;
		break;

	case C2_TYPE_CAPSULE:
		collider_->trueData.capsule.a.x = collider_->bounds.capsule.a.x + trans_->position.x;
		collider_->trueData.capsule.a.y = collider_->bounds.capsule.a.y + trans_->position.y;
		collider_->trueData.capsule.b.x = collider_->bounds.capsule.b.x + trans_->position.x;
		collider_->trueData.capsule.b.y = collider_->bounds.capsule.b.y + trans_->position.y;
		break;

	default://no need for polygons
		break;
	}
}