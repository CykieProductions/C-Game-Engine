#include "Components.h"
#include "Cytools.h"
//#include "DriverSystem.h"
#include "Renderer.h"

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

void Transform_Translate(Transform* trans, int x, int y)
{
	if ((*trans->entity).disabled == true)
		return;

	trans->position.x += x;
	trans->position.y -= y;
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