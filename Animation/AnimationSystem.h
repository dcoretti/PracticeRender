#pragma once
#include "Skeleton.h"
#include "../Model/SmdLoader.h"

Skeleton * generateSkeleton(const SMDModel &smdModel) {
	Skeleton *skeleton = new Skeleton();

	skeleton->joints = new Joint[smdModel.skeleton.bones.size()];
	skeleton->numJoints = (unsigned int) smdModel.skeleton.bones.size();
	int* prevIds = new int[smdModel.skeleton.bones.size()];

	for (unsigned int i = 0; i < skeleton->numJoints; i++) {
		Joint *joint = &skeleton->joints[i];

		SMDBone bone = smdModel.skeleton.bones[i];
		joint->name = bone.name;		// TODO copy don't reuse
		DBG_ASSERT(bone.id == i, "Expected bone id to be sequential, instead of %d, got %d", i, bone.id);
		joint->parentIndex = bone.parentId;


		// Calculate Bj-m to convert a vertex in joint space (for the bind pose) into model space
		Vec3 trans = smdModel.skeleton.bindPose.bonePositions[bone.id].pos;
		float xr = smdModel.skeleton.bindPose.bonePositions[bone.id].rotX;
		float yr = smdModel.skeleton.bindPose.bonePositions[bone.id].rotY;
		float zr = smdModel.skeleton.bindPose.bonePositions[bone.id].rotZ;

		// We have to construct by following the hierarchy until we hit a -1.  Bones with no parent are in 
		// model space!
		int cur = bone.parentId;
		while (cur != -1) {
			trans += smdModel.skeleton.bindPose.bonePositions[cur].pos;
			xr += smdModel.skeleton.bindPose.bonePositions[cur].rotX;
			yr += smdModel.skeleton.bindPose.bonePositions[cur].rotY;
			zr += smdModel.skeleton.bindPose.bonePositions[cur].rotZ;

			cur = smdModel.skeleton.bones[cur].parentId;
		}

		// Get (Bj->m)^-1 in order to be able to transform a model space coord into joint space
		// This can then be multiplied by a matrix to move the joint into the current pose for a clip 
		// thereby transforming that vertex into the proper location for the clip
		joint->invBindPose = invRotTrans(trans, Vec3(xr, yr, zr));




	}

	return skeleton;
}

// TODO switch animationNum to something not shit
AnimationClip * extractSMDPoses(const SMDModel &smdModel, Skeleton *skeleton, int animationNum) {
	SMDAnimation curAnimation = smdModel.animations[animationNum];
	AnimationClip *clip = new AnimationClip();
	clip->skeleton = skeleton;
	clip->numFrames = (unsigned int) curAnimation.animationFrames.size();

	clip->samples = new AnimationSample[clip->numFrames];
	for (unsigned int i = 0; i < clip->numFrames; i++) {
		AnimationSample * sample = &(clip->samples)[i];
		// TODO, add an id array so that the pose can involve a subset of joints
		sample->poses = new Pose[skeleton->numJoints];
		DBG_ASSERT(curAnimation.animationFrames[i].bonePositions.size() == skeleton->numJoints,
			"Expected animation frame with %d joints but got %d", 
			skeleton->numJoints, 
			curAnimation.animationFrames[i].bonePositions.size());

		
		for (unsigned int j = 0; j < skeleton->numJoints; j++) {
			SMDBoneAnimationFrame bonePos = curAnimation.animationFrames[i].bonePositions[j];
			// convert bone position/rotation into scale, translation, quaternion
			Pose * p = &(sample->poses)[j];
			p->scale = 1.0f;
			p->trans = bonePos.pos;
			p->orientation = Quat(bonePos.rotX, bonePos.rotY, bonePos.rotZ);

		}
	}

	return clip;
}