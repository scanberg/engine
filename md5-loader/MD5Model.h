#ifndef MD5MODEL_H
#define MD5MODEL_H
#include "MD5ModelLoaderPCH.h"
#include "Helpers.h"
#include "MD5Animation.h"

class MD5Model{
public:
	 MD5Model();
	virtual ~MD5Model();

	bool LoadModel(const std::string& filename);
	bool LoadAnim(const std::string& filename);
	void Update(float fDeltaTime);
	void Render();

protected:
	typedef std::vector<glm::vec3> PositionBuffer;
	typedef std::vector<glm::vec3> NormalBuffer;
	typedef std::vector<glm::vec2> Tex2DBuffer;
	typedef std::vector<GLuint> IndexBuffer;

	struct Vertex{
		glm::vec3 m_Pos;
		glm::vec3 m_Normal;
		glm::vec2 m_Tex0;
		int m_StartWeight;
		int m_WeightCount;
	};
	typedef std::vector<Vertex> VertexList;

	struct Triangle{
		int m_Indices[3];
	};
	typedef std::vector<Triangle> TriangleList;

	struct Weight{
		int m_JointID;
		float m_Bias;
		glm::vec3 m_Pos;
	};
	typedef std::vector<Weight> WeightList;

	struct Joint{
		std::string m_Name;
		int m_ParentID;
		glm::vec3 m_Pos;
		glm::quat m_Orient;
	};
	typedef std::vector<Joint> JointList;

	struct Mesh{
		std::string m_Shader;
		// This vertex list stores the vertices in the bind pose.
		VertexList m_Verts;
		TriangleList m_Tris;
		WeightList m_Weights;

		// A texture ID for the material
		GLuint m_TexID;

		// These buffers are sed for rendering the animated mesh
		PositionBuffer m_PositionBuffer; // Vertex position stream
		NormalBuffer m_NormalBuffer;	// Vertex normals stream
		Tex2DBuffer m_Tex2DBuffer;		// Texture coordinate set
		IndexBuffer m_IndexBuffer;		// Vertex index buffer
	};
	typedef std::vector<Mesh> MeshList;

	// Prepare the mesh for rendering
	// Compute vertex positions and normals
	bool PrepareMesh(Mesh& mesh);
	bool PrepareMesh(Mesh& mesh, const MD5Animation::FrameSkeleton& skel);
	bool PrepareNormals(Mesh& mesh);

	// Render a singlemesh of the model
	void RenderMesh(const Mesh& mesh);
	void RenderNormals(const Mesh& mesh);

	// Draw the skeleton of the mesh for debugging purposes.
	void RenderSkeleton(const JointList& joints);

	bool CheckAnimation(const MD5Animation& animation) const;

private:
	int m_iMD5Version;
	int m_iNumJoints;
	int m_iNumMeshes;

	bool m_bHasAnimation;

	JointList m_Joints;
	MeshList m_Meshes;

	MD5Animation m_Animation;

	glm::mat4x4 m_LocalToWorldMatrix;
};

#endif
