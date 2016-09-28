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

// LOCAL (relative to parent joint) pose information for a single joint
struct Pose {
	Mat4 orientationMat;
	Quat orientation;
	Vec3 trans;
	float scale;
};

struct Skeleton {
	// id?
	unsigned int numJoints;
	Joint *joints;	// joints and information at bind pose
};


// Skeleton pose contains the local Poses (relative to parent)
// as well as the global pose matrices to convert From joint space to model space (following hierarchy).
//struct SkeletonPose {
//	Skeleton *skeleton;	// skeleton has size of other arrays
//	Pose *poses;	// local joint poses (relative to the immediate parent)
//	Mat4* globalPoses;	// Concatenated poses along the skeleton heirarchy (Pose joint -> model space mat4)
//};


// Given an animation Clip, skeleton poses are sampled at some frame rate.
// ex: 30fps animation has 30 samples per second.  A 3 second long clip will therefore have 90 samples.
struct AnimationSample {
	Pose * poses;	// local poses (relative to parent)
	Mat4* globalPoses;	// Concatenated poses along the skeleton heirarchy (Pose joint -> model space mat4)
	Mat4* skinningPalette; // Model space skinning matrix for a joint.  global poses combined with the bind pose tfrm (Bj->m)^-1 )
};

// A single animation of a skeleton which includes a set of samples consisting of poses for each joint
struct AnimationClip{
	char *name;
	Skeleton *skeleton;	// skeleton has size of other arrays
	float fps;	// sample rate
	unsigned int numFrames;
	AnimationSample *samples;
	bool loop;
};



