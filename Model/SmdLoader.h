#pragma once
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include "../Util.h"
#include "../Common.h"
#include "../Render.h"
#include <vector>

using std::vector;

// Simple implementation of Valve's SMD format
// https://developer.valvesoftware.com/wiki/Studiomdl_Data

#include "../Types/Math.h"

const int MAX_BONE_LINKS = 4;

struct SMDBone {
    int id;
    char * name;
    int parentId;
};

struct SMDGroup {
    int startIndex; // start index of SMDModel for vertex, normals, uvs
    int num;    // number of elements
    char *material; // on delete clear all this crap
};

struct SMDBoneAnimationFrame {
    int id;	// bone id
    Vec3 pos;
	// euler rotation in degrees
    float rotX;   
	float rotY;
	float rotZ;
};

struct SMDAnimationFrame {
    int time;
    vector<SMDBoneAnimationFrame> bonePositions;
};

struct SMDSkeleton {
	vector<SMDBone> bones;
	SMDAnimationFrame bindPose;
};

struct SMDAnimation {
	char * name;
	vector<SMDAnimationFrame> animationFrames;
};

struct SMDModel {
	// SOA specified data for the model
    vector<Vec3> vertices;
    vector<Vec3> normals;
    vector<Vec2> uvs;
	// linkWeights and linkedBones contain MAX_BONE_LINKS per vertex.
	vector<float> linkWeights;
	vector<int> linkedBones;

    vector<SMDGroup> materialGroups;
	SMDSkeleton skeleton;
	vector<SMDAnimation> animations;
	// Bone name and skeleton structure.
//    vector<SMDBone> bones;
//    vector<SMDAnimationFrame> animationFrames;
};




enum SMDParseState {
    TRIANGLES,
    SKELETON,
    NODES,
    NONE
};

struct SMDLoadContext {
    int curAnimationTime{ -1 };
	int curAnimation{ -1 };	// -1 means bind pose, all others are named animations loaded separately.
};

void parseSMDTriangle(char * line, SMDModel &model) {
    // id pos normal uv
    Vec3 vertex;
    Vec3 normal;
    Vec2 uv;
	//float linkWeights[4] = {};
	//int linkedBones[4] = {};
    int parentBone;
    // TODO deal with these..
	int links;
	int offset;
	// material posx posy posz normx normy normz u v links"
    int res = sscanf(line, "%d %f %f %f %f %f %f %f %f %d%n",
        &parentBone,
        &vertex.x, &vertex.y, &vertex.z,
        &normal.x, &normal.y, &normal.z,
        &uv.x, &uv.y,
        &links,
		&offset);
    if (res == 0) {
        size_t len = strlen(line);
        if (model.materialGroups.size() == 0  || strncmp(model.materialGroups.back().material, line, len) != 0) {
            // line is a material name
            SMDGroup group;
            group.startIndex = (int)model.vertices.size();
            group.material = new char[len + 1];
            strcpy(group.material, line);
            if (model.materialGroups.size() > 0) {
                model.materialGroups.back().num = (int)model.vertices.size() - model.materialGroups.back().startIndex;
            }
            model.materialGroups.push_back(group);
        }
    } else if (res >= 9) {
        model.vertices.push_back(vertex);
        model.normals.push_back(normal);
        model.uvs.push_back(uv);
        size_t size = model.vertices.size();
		DBG_ASSERT(links <= MAX_BONE_LINKS, "Got too many bone links on line: \'%s\'", line);
		if (res == 10 && links > 1) {
			// override bone links
			for (int i = 0; i < links; i++) {
				int parentBone;
				float boneWeight;
				sscanf(&line[offset], "%d %f%n", parentBone, boneWeight, offset);
				model.linkedBones.push_back(parentBone);
				model.linkWeights.push_back(boneWeight);

			}
			// Fill out non-specified bones. Note that we use bone 0 rather than -1 (SMD "no parent").  
			// Bone 0 * 0.0 will be 0 anyway so it doesn't matter that bone0 is a real specified bone in the mesh.
			for (int i = links; i < MAX_BONE_LINKS; i++)  {
				model.linkedBones.push_back(0);		
				model.linkWeights.push_back(0.0f);
			}
		} else {
			model.linkedBones.push_back(parentBone);
			model.linkedBones.push_back(0);
			model.linkedBones.push_back(0);
			model.linkedBones.push_back(0);

			model.linkWeights.push_back(1.0f);
			model.linkWeights.push_back(0.0f);
			model.linkWeights.push_back(0.0f);
			model.linkWeights.push_back(0.0f);
		}
    } else {
        DBG_ASSERT(0, "Expected vertex line with 8/11 elements but got %d.  line: %s", res, line);
    }


}

void parseSMDSkeleton(char *line, SMDModel &model, SMDLoadContext &context) {
    int time;
    if (1 == sscanf(line, "time %d", &time)) {
		if (context.curAnimation == -1) {
			// model layout will have a t0 for the bind pose but who cares
			return;
		}
		SMDAnimation &curAnimation = model.animations[context.curAnimation];

        context.curAnimationTime = time;
        SMDAnimationFrame frame;
        frame.time = time;
        curAnimation.animationFrames.push_back(frame);
    } else {
		// Bind pose parsing!
		if (context.curAnimation == -1 ) {
			SMDBoneAnimationFrame boneFrame;
			sscanf(line, "%d\t%f %f %f\t%f %f %f",
				&boneFrame.id,
				&boneFrame.pos.x, &boneFrame.pos.y, &boneFrame.pos.z,
				&boneFrame.rotX, &boneFrame.rotY, &boneFrame.rotZ);
			model.skeleton.bindPose.time = 0;
			model.skeleton.bindPose.bonePositions.push_back(boneFrame);
		} else {
			SMDAnimation &curAnimation = model.animations[context.curAnimation];

			DBG_ASSERT(curAnimation.animationFrames.size() > 0, "Found animation frame data before time line in skeleton block!  line: %s", line);
			SMDAnimationFrame &curFrame = curAnimation.animationFrames[curAnimation.animationFrames.size() - 1];
			DBG_ASSERT(curFrame.time == context.curAnimationTime, "Somehow non-matching time for frame vs context.  Expected %d but found %d",
				context.curAnimationTime, curFrame.time);
			SMDBoneAnimationFrame boneFrame;
			sscanf(line, "%d\t%f %f %f\t%f %f %f",
				&boneFrame.id,
				&boneFrame.pos.x, &boneFrame.pos.y, &boneFrame.pos.z,
				&boneFrame.rotX, &boneFrame.rotY, &boneFrame.rotZ);
			curFrame.bonePositions.push_back(boneFrame);
		}
    }
}

void parseSMDNode(char * line, SMDModel &model) {
    SMDBone bone;
    DBG_ASSERT(sscanf(line, "%d", &bone.id) == 1, "expected first number to be bone id. line: %s", line);
    int wordStart = -1;
    int wordEnd;
    int i = 0;
    for (char *c = line; *c != '\0'; c++, i++) {
        if (*c == '\"' ) {
            if (wordStart == -1) {
                wordStart = i+1;
            } else {
                wordEnd = i-1;
                break;
            }
        }
    }
    DBG_ASSERT(wordStart != -1 && wordEnd != -1 && wordStart != wordEnd, "couldn't find expected bone name in quotes for line: %s", line);
    DBG_ASSERT(*(line + wordEnd) != '\0', "expected parent id, got end of line after name. line: %s", line);
    bone.name = new char[wordEnd - wordStart + 2];
    strncpy(bone.name, line + wordStart, wordEnd - wordStart + 1);
    bone.name[wordEnd - wordStart + 1] = '\0';
    DBG_ASSERT(1 == sscanf((line + wordEnd +2 ), "%d", &bone.parentId), "expected parent id after name.  line: %s", line);

	bool matched = false;
	for (unsigned int i = 0; i < model.skeleton.bones.size(); i++) {
		if (bone.id == model.skeleton.bones[i].id) {
			DBG_ASSERT(bone.parentId == model.skeleton.bones[i].parentId &&
				strcmp(bone.name, model.skeleton.bones[i].name) == 0,
				"Found matching bone id %d already in skeleton with differing components.  Expected parent %d and name %s but got %d %s",
				bone.id, model.skeleton.bones[i].parentId, model.skeleton.bones[i].name, bone.parentId, bone.name);
			matched = true;
			break;
		}
	}
	if (!matched) {
		model.skeleton.bones.push_back(bone);
	}
}


void parseSMDFile(const char *fname, SMDModel *model, SMDLoadContext context) {
    FILE *fp;
    fp = fopen(fname, "r");
    DBG_ASSERT(fp != nullptr, "Unable to open file %s", fname);

    char *line = nullptr;
    int sz;
    SMDParseState curState = NONE;
    while ((sz = getLine(&line, fp)) != 0) {
        if (curState != NONE) {
            if (sz >= 3 && strncmp(line, "end", 3) == 0) {
                curState = NONE;
                continue;
            } 

            switch (curState) {
            case TRIANGLES:
				DBG_ASSERT(context.curAnimation == -1, 
					"Expected to only find triangles on a primary model file, instead working on animation %d", 
					context.curAnimation);
                parseSMDTriangle(line, *model);
                break;
            case SKELETON:
                parseSMDSkeleton(line, *model, context);
                break;
            case NODES:
                parseSMDNode(line, *model);
                break;
            case NONE:
                DBG_ASSERT(0, "Bad line, no active state. Line: %s", line);
                break;
            };
        } else {
            if (sz >= 9 && strncmp(line, "triangles", 9) == 0) {
                curState = TRIANGLES;
            } else if (sz >= 9 && strncmp(line, "version 1", 9) == 0) {
                // do nothing
            } else if (sz >= 8 && strncmp(line, "skeleton", 8) == 0) {
                curState = SKELETON;
            } else if (sz >= 5 && strncmp(line, "nodes", 5) == 0) {
                curState = NODES;
            } else {
                DBG_ASSERT(0, "Unable to parse line %s", line);
            }
        }
        delete[] line;
    }
    DBG_ASSERT(curState == NONE, "Expected end statement at end of file.  Currently in state %d", curState);

    // fix last material group
    model->materialGroups.back().num = (int)model->vertices.size() - model->materialGroups.back().startIndex;
}

// Call to parse primary model file
SMDModel *parseSMDFile(const char *fname) {
	SMDModel *model = new SMDModel();
	parseSMDFile(fname, model, SMDLoadContext());
	return model;
}


// After calling paqrseSMDFile, call this for subsequent animations
void loadAnimationForExistingModel(char *fname, SMDModel *model) {
	SMDLoadContext context;
	SMDAnimation animation;
	animation.name = fname;
	context.curAnimation = (int)model->animations.size();
	model->animations.push_back(animation);

	parseSMDFile(fname, model, context);
}


RenderObject loadSMDToVao(SMDModel &m) {
    RenderObject renderObj;
    initAndSetVao(&renderObj.vao);
    loadBuffer(&renderObj.vertices, sizeof(Vec3) * (int)m.vertices.size(), &m.vertices[0], ShaderAttributeBinding::VERTICES, 3, 0);
    loadBuffer(&renderObj.normals, sizeof(Vec3) * (int)m.normals.size(), &m.normals[0], ShaderAttributeBinding::NORMALS, 3, 0);
    loadBuffer(&renderObj.uvs, sizeof(Vec2) * (int)m.uvs.size(), &m.uvs[0], ShaderAttributeBinding::UV, 2, 0);

	loadBuffer(&renderObj.jointIndices, sizeof(int) * (int)m.linkedBones.size(), &m.linkedBones[0], ShaderAttributeBinding::JOINT_INDICES, MAX_BONE_LINKS, 0);
	loadBuffer(&renderObj.jointWeights, sizeof(float) * (int)m.linkWeights.size(), &m.linkWeights[0], ShaderAttributeBinding::JOINT_WEIGHTS, MAX_BONE_LINKS, 0);

	renderObj.numElements = (int)m.vertices.size();
    return renderObj;
}
