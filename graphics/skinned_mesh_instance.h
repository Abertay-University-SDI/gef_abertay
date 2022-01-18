#ifndef skinned_mesh_instance_h__
#define skinned_mesh_instance_h__

#include <graphics/mesh_instance.h>
#include <animation/skeleton.h>

#include <vector>

namespace gef
{
	class SkinnedMeshInstance : public MeshInstance
	{
	public:
		SkinnedMeshInstance(const gef::Skeleton& skeleton);
		~SkinnedMeshInstance();

		void UpdateBoneMatrices(const gef::SkeletonPose& pose);

		inline std::vector<gef::Matrix44>& bone_matrices() { return bone_matrices_; }
		inline const gef::SkeletonPose& bind_pose() const { return bind_pose_; }
	protected:
		std::vector<gef::Matrix44> bone_matrices_;
		gef::SkeletonPose bind_pose_;
	};

}

#endif // skinned_mesh_instance_h__
