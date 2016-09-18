#pragma once
#include "../Types/Quaternion.h"
#include "../Types/Math.h"

// Each skeleton has a number of joints established in a hierarchy.  Joints can then be posed
// in an animation clip
struct Joint {
	// inv bind pose?
	Mat4 invBindPose;	// Bind pose model space -> Joint space
	int parentIndex;	// -1 = root		TODO move to 8bit?
	char *name;
};

// A single sampled pose of an animation for one joint
struct Pose {
	Quat orientation;
	Vec3 trans;
	float scale;
};

struct Skeleton {
	// id?
	unsigned int numJoints;
	Joint *joints;
};

// constructed from global AnimationClip->AnimationSample->pose data?
struct SkeletonPose {
	Skeleton *skeleton;	// skeleton has size of other arrays
	Pose *poses;	// local joint poses (relative to the immediate parent)
	Mat4* globalPoses;	// Concatenated poses along the skeleton heirarchy
};


// Given an animation Clip, skeleton poses are sampled at some frame rate.
// ex: 30fps animation has 30 samples per second.  A 3 second long clip will therefore have 90 samples.
struct AnimationSample {
	Pose * poses;	// # poses stored in skeleton of clip

};

// A single animation of a skeleton which includes a set of samples consisting of poses for each joint
struct AnimationClip{
	Skeleton *skeleton;	// skeleton has size of other arrays
	float fps;	// sample rate
	unsigned int numFrames;
	AnimationSample *samples;
	bool loop;
};