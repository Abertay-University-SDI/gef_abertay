#include <graphics/skinned_mesh_instance.h>

namespace gef
{
	SkinnedMeshInstance::SkinnedMeshInstance(const gef::Skeleton& skeleton)
	{
		bind_pose_.CreateBindPose(&skeleton);
		bone_matrices_.resize(skeleton.joints().size());
	}

	SkinnedMeshInstance::~SkinnedMeshInstance()
	{

	}

	void SkinnedMeshInstance::UpdateBoneMatrices(const gef::SkeletonPose& pose)
	{
		// calculate bone matrices that need to be passed to the shader
		// this should be the final pose if multiple animations are blended together
		std::vector<gef::Matrix44>::const_iterator pose_matrix_iter = pose.global_pose().begin();
		std::vector<gef::Joint>::const_iterator joint_iter = bind_pose_.skeleton()->joints().begin();

		for (std::vector<gef::Matrix44>::iterator bone_matrix_iter = bone_matrices_.begin(); bone_matrix_iter != bone_matrices_.end(); ++bone_matrix_iter, ++joint_iter, ++pose_matrix_iter)
			*bone_matrix_iter = (joint_iter->inv_bind_pose * *pose_matrix_iter);
	}

}

