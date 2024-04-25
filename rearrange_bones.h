#pragma once

#include <map>
#include "Skeleton.h"
#include "Clip.h"
#include "Mesh.h"

typedef std::map<int, int> BoneMap;

BoneMap RearrangeSkeleton(Skeleton& skeleton);
void RearrangeMesh(Mesh* mesh, BoneMap& boneMap);
void RearrangeClip(Clip& clip, BoneMap& boneMap);
void RearrangeFastClip(FastClip& fastClip, BoneMap& boneMap);