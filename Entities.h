#pragma once
#include "Components.h"

void DS_RegisterEntity(Entity* entity_);
Entity* InitEntity(ObjectType type);
Entity* CloneEntity(Entity* original);

void InitPlayer(int sPosX, int sPosY, int sSclX, int sSclY);

bool DestroyEntity(Entity* entity_);

//Returns the new brush's transform
Transform* new_TileBrush(GAMEBITMAP* sprite_, SpriteSliceData sliceData);
Transform* PaintTile(Entity* brush_);