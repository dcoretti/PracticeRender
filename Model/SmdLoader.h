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
    int id;
    Vec3 pos;
    Vec3 rot;   // euler rotation in degrees
};

struct SMDAnimationFrame {
    int time;
    vector<SMDBoneAnimationFrame> bonePositions;
};

struct SMDModel {
    vector<Vec3> vertices;
    vector<Vec3> normals;
    vector<Vec2> uvs;
    vector<int> parentBones;
    vector<SMDGroup> materialGroups;

    vector<SMDBone> bones;
    vector<SMDAnimationFrame> animationFrames;
};

enum SMDParseState {
    TRIANGLES,
    SKELETON,
    NODES,
    NONE
};

struct SMDLoadContext {
    int curAnimationTime{ -1 };
};

void parseSMDTriangle(char * line, SMDModel &model) {
    // id pos normal uv
    Vec3 vertex;
    Vec3 normal;
    Vec2 uv;
    int parentBone;
    // TODO deal with these..
    int links, boneId;
    float weight;

    int res = sscanf(line, "%d %f %f %f %f %f %f %f %f %d %d %f",
        &parentBone,
        &vertex.x, &vertex.y, &vertex.z,
        &normal.x, &normal.y, &normal.z,
        &uv.x, &uv.y,
        &links, &boneId, &weight);
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
        model.parentBones.push_back(parentBone);
        size_t size = model.vertices.size();

        // todo support last three optional parts
    } else {
        DBG_ASSERT(0, "Expected vertex line with 8/11 elements but got %d.  line: %s", res, line);
    }


}

void parseSMDSkeleton(char *line, SMDModel &model, SMDLoadContext &context) {
    int time;
    if (1 == sscanf(line, "time %d", &time)) {
        context.curAnimationTime = time;
        SMDAnimationFrame frame;
        frame.time = time;
        model.animationFrames.push_back(frame);
    } else {
        DBG_ASSERT(model.animationFrames.size() > 0, "Found animation frame data before time line in skeleton block!  line: %s", line);
        SMDAnimationFrame &curFrame = model.animationFrames[model.animationFrames.size() - 1];
        DBG_ASSERT(curFrame.time == context.curAnimationTime, "Somehow non-matching time for frame vs context.  Expected %d but found %d",
            context.curAnimationTime, curFrame.time);
        SMDBoneAnimationFrame boneFrame;
        sscanf(line, "%d\t%f %f %f\t%f %f %f", 
            &boneFrame.id, 
            &boneFrame.pos.x, &boneFrame.pos.y, &boneFrame.pos.z,
            &boneFrame.rot.x, &boneFrame.rot.y, &boneFrame.rot.z);
        curFrame.bonePositions.push_back(boneFrame);
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
    model.bones.push_back(bone);
}



SMDModel *parseSMDFile(const char *fname) {
    FILE *fp;
    fp = fopen(fname, "r");
    DBG_ASSERT(fp != nullptr, "Unable to open file %s", fname);

    char *line = nullptr;
    int sz;
    SMDParseState curState = NONE;
    SMDModel *model = new SMDModel();
    SMDLoadContext context;
    while ((sz = getLine(&line, fp)) != 0) {
        if (curState != NONE) {
            if (sz >= 3 && strncmp(line, "end", 3) == 0) {
                curState = NONE;
                continue;
            } 

            switch (curState) {
            case TRIANGLES:
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

    return model;
}



RenderObject loadSMDToVao(SMDModel &m) {
    RenderObject renderObj;
    initAndSetVao(&renderObj.vao);
    loadBuffer(&renderObj.vertices, sizeof(Vec3) * (int)m.vertices.size(), &m.vertices[0], ShaderAttributeBinding::VERTICES, 3, 0);
    loadBuffer(&renderObj.normals, sizeof(Vec3) * (int)m.normals.size(), &m.normals[0], ShaderAttributeBinding::NORMALS, 3, 0);
    loadBuffer(&renderObj.uvs, sizeof(Vec2) * (int)m.uvs.size(), &m.uvs[0], ShaderAttributeBinding::UV, 2, 0);

    renderObj.numElements = (int)m.vertices.size();
    return renderObj;
}