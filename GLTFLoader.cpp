#include "GLTFLoader.h"
#include <iostream>
#include "Transform.h"
#include <glm/gtc/type_ptr.hpp>

namespace GLTFHelpers {
	// ��ȡĳ�����洢�ı任
	Transform GetLocalTransform(cgltf_node& node) {
		Transform result;

		if (node.has_matrix) {
			float matrixArray[16];
			for (int i = 0; i < 16; ++i) {
				matrixArray[i] = static_cast<float>(node.matrix[i]);
			}

			// Construct a glm::mat4 from the array
			glm::mat4 mat = glm::make_mat4(matrixArray);

			result = Transform::mat4ToTransform(mat);
		}

		if (node.has_translation) {
			result.SetPosition(node.translation[0], node.translation[1], node.translation[2]);
		}
		if (node.has_rotation) {
			result.SetRotation(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));
			// result.SetRotation(glm::quat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
		}
		if (node.has_scale) {
			result.SetScale(node.scale[0], node.scale[1], node.scale[2]);
		}

		return result;
	}

	int GetNodeIndex(cgltf_node* target, cgltf_node* allNodes, unsigned int numNodes) {
		if (target == 0) { // 0��ʾ��ָ�룬�Ƿ�ָ�벻���в���
			return -1;
		}

		for (unsigned int i = 0; i < numNodes; i++) {
			if (&allNodes[i] == target) return (int)i;
		}

		return -1; // û���ҵ�
	}

	// compCount: һ������ɼ���floatֵ���
	void GetScalarValues(std::vector<float>& out, unsigned int compCount, const cgltf_accessor& inAccessor) {
		out.resize(inAccessor.count * compCount); // inAccessor.count��ʾ����Ԫ�صĸ�������vec3�ĸ�����quat�ĸ�����ÿ������Ԫ�ؿɱ�ʾvalue��in��out��
		for (cgltf_size i = 0; i < inAccessor.count; ++i) {
			cgltf_accessor_read_float(&inAccessor, i, &out[i * compCount], compCount);
		}
	}

	template<typename T, int N>
	void TrackFromChannel(Track<T, N>& inOutTrack, const cgltf_animation_channel& inChannel) {
		cgltf_animation_sampler& sampler = *inChannel.sampler;

		// ���ò�ֵ����
		Interpolation interpolation = Interpolation::Constant;
		if (inChannel.sampler->interpolation == cgltf_interpolation_type_linear) {
			interpolation = Interpolation::Linear;
		}
		else if (inChannel.sampler->interpolation == cgltf_interpolation_type_cubic_spline) {
			interpolation = Interpolation::Cubic;
		}
		bool isSamplerCubic = interpolation == Interpolation::Cubic; // ����ȷ�������Ƿ�������������
		inOutTrack.SetInterpolation(interpolation);

		std::vector<float> timelineFloats;
		GetScalarValues(timelineFloats, 1, *sampler.input); // sampler�������Ƕ�ʱ����ķ�����

		std::vector<float> valueFloats;
		GetScalarValues(valueFloats, N, *sampler.output); // sampler������Ǹ��ݺ�ʱ�����Ӧ��ֵ����������ת��λ�ơ�����

		unsigned int numFrames = (unsigned int)sampler.input->count; // ����ʱ����ĸ�������֡��
		unsigned int numberOfPerFrame = valueFloats.size() / timelineFloats.size(); // ÿ������Ԫ�صĸ�����timelineFloats.size()Ӧ��Ҳ���Ի���֡��
		inOutTrack.Resize(numFrames);
		// ��ȡÿ֡������
		for (unsigned int i = 0; i < numFrames; i++) {
			int baseIndex = i * numberOfPerFrame;
			Frame<N>& frame = inOutTrack[i];
			int offset = 0;

			frame.mTime = timelineFloats[i];
			for (int component = 0; component < N; component++) {
				frame.mIn[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
			}
			for (int component = 0; component < N; component++) {
				frame.mValue[component] = valueFloats[baseIndex + offset++];
			}
			for (int component = 0; component < N; component++) {
				frame.mOut[component] = isSamplerCubic ? valueFloats[baseIndex + offset++] : 0.0f;
			}
		}
	}


	// ����Mesh
	void MeshFromAttribute(Mesh& outMesh, cgltf_attribute& attribute, cgltf_skin* skin, cgltf_node* nodes, unsigned int nodeCount) {
		// ��attribute������(pos, norm, uv, weight, influence)
		cgltf_attribute_type attribType = attribute.type;
		// attribute�ķ�����
		cgltf_accessor& accessor = *attribute.data;

		unsigned int componentCount = 0; // �����Ǽ���
		if (accessor.type == cgltf_type_vec2) {
			componentCount = 2;
		}
		else if (accessor.type == cgltf_type_vec3) {
			componentCount = 3;
		}
		else if (accessor.type == cgltf_type_vec4) {
			componentCount = 4;
		}
		std::vector<float> values; // ������ȡֵ
		GetScalarValues(values, componentCount, accessor);
		unsigned int accessorCount = (unsigned int)accessor.count; // ����Ԫ�ظ���

		std::vector<Vertex>& vertices = outMesh.GetVertices();

		std::vector<ivec4> influences;
		std::vector<vec4> weights;
		influences.clear();
		weights.clear();

		for (unsigned int i = 0; i < accessorCount; i++) {
			int index = i * componentCount;
			switch (attribType) {
				// Ӱ�쵱ǰ�����Ȩ��
			case cgltf_attribute_type_weights:
				weights.push_back(vec4(values[index], values[index + 1], values[index + 2], values[index + 3]));
				break;
			// Ӱ�쵱ǰ����Ĺ���������
			case cgltf_attribute_type_joints:
			{
				// values�Ƕ���Ĺؽ�����
				ivec4 joints((int)(values[index] + 0.5f), (int)(values[index + 1] + 0.5f), (int)(values[index + 2] + 0.5f), (int)(values[index + 3] + 0.5f));
				// ���ؽ������Ƥ��������ת��Ϊ�ڵ��������Ϊ�˼���ؽھ�����Ҫ֪���ؽ��ڽڵ����е�λ�úͷ��򣬶������ڹǼ��е�˳��
				joints.x = std::max(0, GetNodeIndex(skin->joints[joints.x], nodes, nodeCount));
				joints.y = std::max(0, GetNodeIndex(skin->joints[joints.y], nodes, nodeCount));
				joints.z = std::max(0, GetNodeIndex(skin->joints[joints.z], nodes, nodeCount));
				joints.w = std::max(0, GetNodeIndex(skin->joints[joints.w], nodes, nodeCount));

				influences.push_back(joints);
			}
			break;
			default:
				break;
			}
		}

		bool weight_flag = vertices.size() == 0;
		for (int i = 0; i < weights.size(); i++) {
			if (weight_flag) {
				Vertex vert;
				vert.Weights[0] = weights[i].x;
				vert.Weights[1] = weights[i].y;
				vert.Weights[2] = weights[i].z;
				vert.Weights[3] = weights[i].w;
				vertices.push_back(vert);
			}
			else {
				vertices[i].Weights[0] = weights[i].x;
				vertices[i].Weights[1] = weights[i].y;
				vertices[i].Weights[2] = weights[i].z;
				vertices[i].Weights[3] = weights[i].w;
			}
		}

		bool influence_flag = vertices.size() == 0;
		for (int i = 0; i < influences.size(); i++) {
			if (influence_flag) {
				Vertex vert;
				vert.Influences[0] = influences[i].x;
				vert.Influences[1] = influences[i].y;
				vert.Influences[2] = influences[i].z;
				vert.Influences[3] = influences[i].w;
				vertices.push_back(vert);
			}
			else {
				vertices[i].Influences[0] = influences[i].x;
				vertices[i].Influences[1] = influences[i].y;
				vertices[i].Influences[2] = influences[i].z;
				vertices[i].Influences[3] = influences[i].w;
			}
		}
	}
}

cgltf_data* LoadGLTFFile(const char* path) {
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options)); // �Ƚ��ṹ���ڵ�Ԫ��ȫ���Ϊ0

	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, path, &data);
	if (result != cgltf_result_success) {
		std::cout << "Could not load input file: " << path << "\n";
		return 0;
	}
	result = cgltf_load_buffers(&options, data, path);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Could not load buffer for: " << path << "\n";
		return 0;
	}
	result = cgltf_validate(data);
	if (result != cgltf_result_success) {
		cgltf_free(data);
		std::cout << "Invalid gltf file: " << path << "\n";
		return 0;
	}
	return data;
}

void FreeGLTFFile(cgltf_data* handle) {
	if (handle == 0) {
		std::cout << "WARNING: Can't free null data\n";
	}
	else {
		cgltf_free(handle);
	}
}

Pose LoadRestPose(cgltf_data* data) {
	unsigned int boneCount = data->nodes_count;
	Pose result(boneCount);

	for (unsigned int i = 0; i < boneCount; i++) {
		cgltf_node* node = &(data->nodes[i]);

		// ���ù����ı任
		Transform transform = GLTFHelpers::GetLocalTransform(data->nodes[i]);
		glm::mat4 rotate = glm::mat4(1.0f);
		float angle = glm::radians(90.0f);
		rotate = glm::rotate(rotate, angle, glm::vec3(0.0f, 1.0f, 0.0f));
		Transform rotate_trans = Transform::mat4ToTransform(rotate);
		transform = Transform::combine(transform, rotate_trans);

		result.SetLocalTransform(i, transform);

		//glm::mat4 mInvBindPose;
		//mInvBindPose = transform.GetTransformMatrix();
		//std::cout << mInvBindPose[0][0] << " " << mInvBindPose[0][1] << " " << mInvBindPose[0][2] << " " << mInvBindPose[0][3] << std::endl;

		// �����丸�ڵ�
		int parent = GLTFHelpers::GetNodeIndex(node->parent, data->nodes, boneCount);
		result.SetParent(i, parent);
	}

	return result;
}

std::vector<std::string> LoadJointNames(cgltf_data* data) {
	unsigned int boneCount = (unsigned int)data->nodes_count;
	std::vector<std::string> result(boneCount, "Not Set");

	for (unsigned int i = 0; i < boneCount; i++) {
		cgltf_node* node = &(data->nodes[i]);

		if (node->name == 0) { // û�����֣�ָ��Ϊ��
			result[i] = "EMPTY NODE";
		}
		else {
			result[i] = node->name;
		}
	}

	return result;
}

std::vector<Clip> LoadAnimationClips(cgltf_data* data) {
	unsigned int numClips = (unsigned int)data->animations_count;
	unsigned int numNodes = (unsigned int)data->nodes_count; // �����н��ĸ���

	std::vector<Clip> result;
	result.resize(numClips);

	for (unsigned int i = 0; i < numClips; i++) {
		// Ϊÿ��Ƭ����������
		result[i].SetName(data->animations[i].name);
		unsigned int numChannels = (unsigned int)data->animations[i].channels_count;
		for (unsigned int j = 0; j < numChannels; j++) { // ͨ�������˶�����Ŀ��ڵ�����ԣ���ƽ�ơ���ת������)
			cgltf_animation_channel& channel = data->animations[i].channels[j];
			cgltf_node* target = channel.target_node;
			int nodeId = GLTFHelpers::GetNodeIndex(target, data->nodes, numNodes);
			// �������;�������TransformTrack�е��ĸ���Ա
			if (channel.target_path == cgltf_animation_path_type_translation) {
				VectorTrack& track = result[i][nodeId].GetPositionTrack();
				GLTFHelpers::TrackFromChannel<glm::vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_scale) {
				VectorTrack& track = result[i][nodeId].GetScaleTrack();
				GLTFHelpers::TrackFromChannel<glm::vec3, 3>(track, channel);
			}
			else if (channel.target_path == cgltf_animation_path_type_rotation) {
				QuaternionTrack& track = result[i][nodeId].GetRotationTrack();
				GLTFHelpers::TrackFromChannel<glm::quat, 4>(track, channel);
			}
		}

		result[i].RecalculateDuration();
	}

	return result;
}

Pose LoadBindPose(cgltf_data* data) {
	// �Ȼ��restPose��ΪĬ��ֵ��������ļ�δ����bindPose�����´���
	Pose restPose = LoadRestPose(data);
	unsigned int numBones = restPose.Size();
	std::vector<Transform> worldBindPose(numBones);
	for (unsigned int i = 0; i < numBones; i++) {
		worldBindPose[i] = restPose.GetGlobalTransform(i); // ����ÿ���ؽڵ�Ĭ��ֵ

		// glm::mat4 mInvBindPose;
		//mInvBindPose = restPose.GetLocalTransform(i).GetTransformMatrix();
		//std::cout << mInvBindPose[0][0] << " " << mInvBindPose[0][1] << " " << mInvBindPose[0][2] << " " << mInvBindPose[0][3] << std::endl;
	}

	unsigned int numSkins = data->skins_count;
	for (unsigned int i = 0; i < numSkins; i++) { // ���ÿ��Ƥ������󶨾���(ȫ�־���)
		cgltf_skin* skin = &(data->skins[i]);
		std::vector<float> invBindAccessor;
		GLTFHelpers::GetScalarValues(invBindAccessor, 16, *skin->inverse_bind_matrices); // һ��Ԫ����һ��4*4�ľ��󹹳�
	
		// ��ø���Ƥ��ÿ���ؽڵ���󶨾���
		unsigned int numJoints = skin->joints_count;
		for (unsigned int j = 0; j < numJoints; j++) {
			// ��øùؽڵ� inverse bind matrix
			float* matrix = &invBindAccessor[j * 16];
			glm::mat4 invBindMatrix = glm::make_mat4(matrix);
			// ��ð󶨾���
			glm::mat4 bindMatrix = glm::inverse(invBindMatrix);
			Transform bindTransform = Transform::mat4ToTransform(bindMatrix);

			//std::cout << "i" << std::endl;
			//glm::mat4 mInvBindPose;
			//mInvBindPose = bindTransform.GetTransformMatrix();
			//std::cout << mInvBindPose[0][0] << " " << mInvBindPose[0][1] << " " << mInvBindPose[0][2] << " " << mInvBindPose[0][3] << std::endl;
			//std::cout << bindTransform.Position().x << " " << bindTransform.Position().y << " " << bindTransform.Position().z << std::endl;
			//std::cout << bindTransform.Scale().x << " " << bindTransform.Scale().y << " " << bindTransform.Scale().z << std::endl;
			// ���ݽ�����ð󶨱任
			cgltf_node* jointNode = skin->joints[j];
			int jointIndex = GLTFHelpers::GetNodeIndex(jointNode, data->nodes, numBones);
			worldBindPose[jointIndex] = bindTransform;
		}
	}

	// ������任תΪ����ڸ��ڵ�ľֲ��任
	Pose bindPose = restPose;
	for (unsigned int i = 0; i < numBones; i++) {
		Transform current = worldBindPose[i];

		int p = bindPose.GetParent(i);
		if (p >= 0) {
			Transform parent = worldBindPose[p];
			current = Transform::combine(Transform::inverse(parent), current); // ���ڵ�������˸ý��ı任�Ϳ���ȡ�����ڵ�ı任

			//std::cout << "i" << std::endl;
			//glm::mat4 mInvBindPose;
			//mInvBindPose = current.GetTransformMatrix();
			//std::cout << mInvBindPose[0][0] << " " << mInvBindPose[0][1] << " " << mInvBindPose[0][2] << " " << mInvBindPose[0][3] << std::endl;
		}
		bindPose.SetLocalTransform(i, current);
	}

	return bindPose;
}

Skeleton LoadSkeleton(cgltf_data* data) {
	return Skeleton(LoadRestPose(data), LoadBindPose(data), LoadJointNames(data));
}


// �ú�����֧��ֻ��һ��ģ�͵����
std::vector<Mesh> LoadMeshes(cgltf_data* data) {
	std::vector<Mesh> result;
	cgltf_node* nodes = data->nodes;
	unsigned int nodeCount = (unsigned int)data->nodes_count;

	for (unsigned int i = 0; i < nodeCount; i++) {
		cgltf_node* node = &nodes[i];
		// ����ýڵ�û��mesh��skin��������
		if (node->mesh == 0 || node->skin == 0) {
			continue;
		}

		// gltf��һ��mesh��Ӧһ��primitive��
		unsigned int numPrims = (unsigned int)node->mesh->primitives_count;
		for (unsigned int j = 0; j < numPrims; j++) {
			result.push_back(Mesh());
			Mesh& mesh = result[result.size() - 1];

			cgltf_primitive* primitive = &node->mesh->primitives[j];

			unsigned int numAttributes = (unsigned int)primitive->attributes_count;
			for (unsigned int k = 0; k < numAttributes; k++) {
				cgltf_attribute* attribute = &primitive->attributes[k];
				GLTFHelpers::MeshFromAttribute(mesh, *attribute, node->skin, nodes, nodeCount);
			}
			std::cout << "another mesh" << std::endl;
		}
	}

	return result;
}