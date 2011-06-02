#define GLEW_STATIC

#include <GL/glew.h>
#include <GL/glfw.h>

#include "MD5Model.h"
#include "Math.h"
#include "Material.h"
#include "SceneHandler.h"
#include <iostream>

MD5Model::MD5Model()
: m_iMD5Version(-1)
, m_iNumJoints(0)
, m_iNumMeshes(0)
, m_bHasAnimation(false)
, m_LocalToWorldMatrix(1)
{
}

MD5Model::~MD5Model()
{
}

bool MD5Model::LoadModel(const std::string& filename)
{
	std::string param;
	std::string junk; // Read junk from file.

	std::ifstream file;

	// try to open filename
	file.open(filename.c_str(), std::ifstream::in);

	if( file.fail() )
		return false;

	m_Joints.clear();
	m_Meshes.clear();

	file >> param;

	while(!file.eof())
	{
		if(param == "MD5Version")
		{
			file >> m_iMD5Version;
			assert(m_iMD5Version == 10);
		}
		else if(param == "commandline")
		{
			Ignore(file);
		}
		else if(param == "numJoints")
		{
			file >> m_iNumJoints;
			m_Joints.reserve(m_iNumJoints);
		}
		else if(param == "numMeshes")
		{
			file >> m_iNumMeshes;
			m_Meshes.reserve(m_iNumMeshes);
		}
		else if(param == "joints")
		{
			Joint joint;
			file >> junk; // Read the '{' character
			for(int i = 0; i < m_iNumJoints; i++)
			{
				file >> joint.m_Name >> joint.m_ParentID >> junk
					 >> joint.m_Pos.x >> joint.m_Pos.y >> joint.m_Pos.z >> junk >> junk
					 >> joint.m_Orient.x >> joint.m_Orient.y >> joint.m_Orient.z >> junk;

				RemoveQuotes(joint.m_Name);
				ComputeQuatW(joint.m_Orient);

				m_Joints.push_back(joint);
				// Ignore everything else on the line up to the end-of-line character
				Ignore(file);
				//IgnoreLine(file, fileLength);

			}
			file >> junk; // Read the '}' character
		}
		else if(param == "mesh")
		{
			MD5Mesh mesh;
			int numVerts, numTris, numWeights;

			file >> junk; // Read the '{' character
			file >> param;
			while(param != "}")
			{// Read until we get to the '}' character
				if(param == "shader"){
					file >> mesh.m_Shader;
					RemoveQuotes(mesh.m_Shader);

					Ignore(file);
				}
				else if(param == "numverts")
				{
					file >> numVerts; // Read in the vertices
					Ignore(file);
					for(int i = 0; i < numVerts; ++i)
					{
						Vertex vert;

						file >> junk >> junk >> junk // vert vertIndex (
							 >> vert.m_Tex0.x >> vert.m_Tex0.y >> junk // s t )
							 >> vert.m_StartWeight >> vert.m_WeightCount;
						Ignore(file);

						mesh.m_Verts.push_back(vert);


						mesh.m_Tex2DBuffer.push_back(vert.m_Tex0);
					}
				}
				else if(param == "numtris"){
					file >> numTris;
					Ignore(file);
					for(int i = 0; i < numTris; ++i){
						Triangle tri;
						file >> junk >> junk >> tri.m_Indices[0] >> tri.m_Indices[1] >> tri.m_Indices[2];
						Ignore(file);

						mesh.m_Tris.push_back(tri);
						mesh.m_IndexBuffer.push_back((GLuint)tri.m_Indices[0]);
						mesh.m_IndexBuffer.push_back((GLuint)tri.m_Indices[1]);
						mesh.m_IndexBuffer.push_back((GLuint)tri.m_Indices[2]);
					}
				}
				else if(param == "numweights"){
					file >> numWeights;
					std::cout<<"numWeights "<<numWeights<<std::endl;
					Ignore(file);
					for(int i = 0; i < numWeights; ++i){
						Weight weight;
						file >> junk >> junk >> weight.m_JointID >>weight.m_Bias >> junk
							>> weight.m_Pos.x >> weight.m_Pos.y >> weight.m_Pos.z >> junk;

						Ignore(file);
						mesh.m_Weights.push_back(weight);
					}
				}
				else{
					Ignore(file);
				}
				file >> param;
			}
			PrepareMesh(mesh);
			PrepareNormals(mesh);

            AddMesh(mesh);

			m_Meshes.push_back(mesh);
		}
		file >> param;
	}
	assert((int)m_Joints.size() == m_iNumJoints);
	assert((int)m_Meshes.size() == m_iNumMeshes);

	return true;
}
bool MD5Model::LoadAnim( const std::string& filename ){
    if ( m_Animation.LoadAnimation( filename ) ){
        // Check to make sure the animation is appropriate for this model
        m_bHasAnimation = CheckAnimation( m_Animation );

    }
    else
    {
        std::cout<<"FAIL";
    }

    return m_bHasAnimation;
}

bool MD5Model::CheckAnimation( const MD5Animation& animation ) const{
    if ( m_iNumJoints != animation.GetNumJoints() ){
        std::cout<<"num joints "<<animation.GetNumJoints()<<std::endl;
        return false;
    }

    // Check to make sure the joints match up
    for ( unsigned int i = 0; i < m_Joints.size(); ++i ){
        const Joint& meshJoint = m_Joints[i];
        const MD5Animation::JointInfo& animJoint = animation.GetJointInfo( i );

        if ( meshJoint.m_Name != animJoint.m_Name ||
             meshJoint.m_ParentID != animJoint.m_ParentID ){
            std::cout<<"NAME OR PARENTID WRONG "<<std::endl;
            return false;
        }
    }

    return true;
}

void MD5Model::AddMesh(const MD5Mesh& mesh)
{
    Mesh* m = new Mesh();
    m->init(mesh.m_Verts.size(),mesh.m_Tris.size());

    unsigned int i;
    for(i=0; i<mesh.m_Verts.size(); i++)
    {
        m->vertex[i].x = mesh.m_PositionBuffer.at(i).x;
        m->vertex[i].y = mesh.m_PositionBuffer.at(i).y;
        m->vertex[i].z = mesh.m_PositionBuffer.at(i).z;

        m->vertex[i].nx = mesh.m_NormalBuffer.at(i).x;
        m->vertex[i].ny = mesh.m_NormalBuffer.at(i).y;
        m->vertex[i].nz = mesh.m_NormalBuffer.at(i).z;

        m->vertex[i].u = mesh.m_Tex2DBuffer.at(i).x;
        m->vertex[i].v = mesh.m_Tex2DBuffer.at(i).y;
    }
}

bool MD5Model::PrepareMesh(MD5Mesh& mesh){
	mesh.m_PositionBuffer.clear();
	mesh.m_Tex2DBuffer.clear();
	mesh.m_TangentBuffer.clear();

	// Compute vertex positions
	for(unsigned int i = 0; i < mesh.m_Verts.size(); ++i){
		glm::vec3 finalPos(0);
		Vertex& vert = mesh.m_Verts[i];

		vert.m_Pos = glm::vec3(0);
		vert.m_Normal = glm::vec3(0);


		// Sum the position of the weights
		for(int j = 0; j < vert.m_WeightCount; ++j){
			Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
			Joint& joint = m_Joints[weight.m_JointID];

			// Convert the weight position from Joint local space to object space
			glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;

			vert.m_Pos += ( joint.m_Pos + rotPos ) * weight.m_Bias;
		}
		mesh.m_TangentBuffer.push_back(glm::vec3(0));
		mesh.m_PositionBuffer.push_back(vert.m_Pos);
		mesh.m_Tex2DBuffer.push_back(vert.m_Tex0);
	}
	PrepareMaterial(mesh);
    PrepareTangents(mesh);
	return true;
}

bool MD5Model::PrepareMesh( MD5Mesh& mesh, const MD5Animation::FrameSkeleton& skel ){
    for ( unsigned int i = 0; i < mesh.m_Verts.size(); ++i ){
        const Vertex& vert = mesh.m_Verts[i];
        glm::vec3& pos = mesh.m_PositionBuffer[i];
        glm::vec3& normal = mesh.m_NormalBuffer[i];

        pos = glm::vec3(0);
        normal = glm::vec3(0);

        for ( int j = 0; j < vert.m_WeightCount; ++j ){
            const Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
            const MD5Animation::SkeletonJoint& joint = skel.m_Joints[weight.m_JointID];

            glm::vec3 rotPos = joint.m_Orient * weight.m_Pos;
            pos += ( joint.m_Pos + rotPos ) * weight.m_Bias;
            normal += ( joint.m_Orient * vert.m_Normal ) * weight.m_Bias;
        }
    }
    PrepareTangents(mesh);
    return true;
}

bool MD5Model::PrepareTangents(MD5Mesh& mesh)
{
    mesh.m_TangentBuffer.clear();
    for ( unsigned int i = 0; i < mesh.m_Tris.size(); ++i)
    {
        glm::vec3 tangent;
        glm::vec2 st1, st2;
        glm::vec3 a, b;

        a = mesh.m_PositionBuffer[mesh.m_Tris[i].m_Indices[2]] - mesh.m_PositionBuffer[mesh.m_Tris[i].m_Indices[0]];
        b = mesh.m_PositionBuffer[mesh.m_Tris[i].m_Indices[1]] - mesh.m_PositionBuffer[mesh.m_Tris[i].m_Indices[0]];

        st1.x = mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[2]].x - mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[0]].x;
        st1.y = mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[2]].y - mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[0]].y;

        st2.x = mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[1]].x - mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[0]].x;
        st2.y = mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[1]].y - mesh.m_Tex2DBuffer[mesh.m_Tris[i].m_Indices[0]].y;

        float coef = 1.0/ (st1.x * st2.y - st2.x * st1.y);

        tangent.x = coef * ((a.x * st2.y)  + (b.x * -st1.y));
        tangent.y = coef * ((a.y * st2.y)  + (b.y * -st1.y));
        tangent.z = coef * ((a.z * st2.y)  + (b.z * -st1.y));
        tangent = glm::normalize(tangent);
        mesh.m_TangentBuffer.push_back(tangent);
    }
    return true;
}

bool MD5Model::PrepareMaterial(MD5Mesh& mesh)
{
    std::cout<<"loading "<<mesh.m_Shader<<std::endl;

    std::string s;

    s = mesh.m_Shader+"_diffuse.tga";
    if(FileExists(s))
    {
        std::cout<<"DIFFUSE";
        mesh.m_Mat.diffuseMap=SceneHandler::resources.loadTexture(s,GLFW_BUILD_MIPMAPS_BIT);

        glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.diffuseMap);

        // Specify trilinear interpolation
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        glBindTexture(GL_TEXTURE_2D, 0);

        mesh.m_Mat.type+=TEX_DIFFUSE;
    }

    s = mesh.m_Shader+"_normal.tga";
    if(FileExists(s))
    {
        std::cout<<"NORMAL";
        mesh.m_Mat.normalMap=SceneHandler::resources.loadTexture(s,GLFW_BUILD_MIPMAPS_BIT);

        glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.normalMap);

        // Specify trilinear interpolation
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        glBindTexture(GL_TEXTURE_2D, 0);

        mesh.m_Mat.type+=TEX_NORMAL;
    }

    s = mesh.m_Shader+"_height.tga";
    if(FileExists(s))
    {
        std::cout<<"HEIGHT";
        //mat.heightMap=SceneHandler::resources.loadTexture(s,0);

        //glBindTexture(GL_TEXTURE_2D, mat.heightMap);
        //glBindTexture(GL_TEXTURE_2D, 0);
        std::string normal = mesh.m_Shader+"_normal.tga";

        GLFWimage merged;
        merged = mergeRGB_A(&normal,&s);
        mesh.m_Mat.normalMap = SceneHandler::resources.createTextureFromImage(normal+s, &merged, GLFW_BUILD_MIPMAPS_BIT);
        glfwFreeImage(&merged);

        mesh.m_Mat.type+=TEX_HEIGHT;
    }

    s = mesh.m_Shader+"_specular.tga";
    if(FileExists(s))
    {
        std::cout<<"SPECULAR";
        mesh.m_Mat.normalMap=SceneHandler::resources.loadTexture(s,GLFW_BUILD_MIPMAPS_BIT);

        glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.specularMap);

        // Specify trilinear interpolation
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

        glBindTexture(GL_TEXTURE_2D, 0);

        mesh.m_Mat.type+=TEX_SPECULAR;
    }

    return true;
}

bool MD5Model::PrepareNormals(MD5Mesh& mesh){
	mesh.m_NormalBuffer.clear();
	// Loop trough all triangles and calculate the normal of each triangle
	for(unsigned int i = 0; i < mesh.m_Tris.size(); ++i){
		glm::vec3 v0 = mesh.m_Verts[mesh.m_Tris[i].m_Indices[0]].m_Pos;
		glm::vec3 v1 = mesh.m_Verts[mesh.m_Tris[i].m_Indices[1]].m_Pos;
		glm::vec3 v2 = mesh.m_Verts[mesh.m_Tris[i].m_Indices[2]].m_Pos;

		glm::vec3 normal = glm::cross(v2 - v0, v1 - v0);

		mesh.m_Verts[mesh.m_Tris[i].m_Indices[0]].m_Normal += normal;
		mesh.m_Verts[mesh.m_Tris[i].m_Indices[1]].m_Normal += normal;
		mesh.m_Verts[mesh.m_Tris[i].m_Indices[2]].m_Normal += normal;
	}
	// Now normalize all the normals.
	for(unsigned int i = 0; i < mesh.m_Verts.size(); ++i){
		Vertex& vert = mesh.m_Verts[i];

		glm::vec3 normal = glm::normalize(vert.m_Normal);
		mesh.m_NormalBuffer.push_back(normal);

		// Reset the normal to calculate the bind-pose normal in joint space
		vert.m_Normal = glm::vec3(0);

		// Put the bind-pose normal into joint-local space
		// so the animated normal can be computed faster later.
		for(int j = 0; j < vert.m_WeightCount; ++j){
			const Weight& weight = mesh.m_Weights[vert.m_StartWeight + j];
			const Joint& joint = m_Joints[weight.m_JointID];
			vert.m_Normal += (normal * joint.m_Orient) * weight.m_Bias;
		}
	}
	return true;
}

void MD5Model::Render(){
	glPushMatrix();
	glMultMatrixf(glm::value_ptr(m_LocalToWorldMatrix));

	// Render the meshes
	for(unsigned int i = 0; i < m_Meshes.size(); ++i){
		RenderMesh(m_Meshes[i]);
	}
	//m_Animation.Render();
//	for(unsigned int i = 0; i < m_Meshes.size(); ++i){
//		RenderNormals(m_Meshes[i]);
//	}
	glPopMatrix();
}

void MD5Model::RenderMesh(const MD5Mesh& mesh){

    GLuint shad=SceneHandler::shaderLib.GetShaderFromType(mesh.m_Mat.type);

    if(shad==0)
        return;

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.diffuseMap);
    setUniform1i(shad,0,"diffuseMap");

    glActiveTexture( GL_TEXTURE1 );
    glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.normalMap);
    setUniform1i(shad,1,"normalMap");

    glActiveTexture( GL_TEXTURE2 );
    glBindTexture(GL_TEXTURE_2D, mesh.m_Mat.specularMap);
    setUniform1i(shad,2,"specularMap");

    setAttributeTangent(shad, &mesh.m_TangentBuffer[0], "tangent");

    setUniform2f(shad,SceneHandler::near,SceneHandler::far,"cameraRange");

    Camera *cam = Camera::getActiveCamera();
    setUniform3f(shad,cam->pos.x,cam->pos.y,cam->pos.z,"cameraPos");
    setUniform3f(shad,cam->dir.x,cam->dir.y,cam->dir.z,"cameraDir");

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mesh.m_Mat.diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mesh.m_Mat.ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mesh.m_Mat.specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mesh.m_Mat.shininess);

    glUseProgram( shad );
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, &(mesh.m_PositionBuffer[0]));
        glNormalPointer(GL_FLOAT, 0, &(mesh.m_NormalBuffer[0]));
        glTexCoordPointer(2, GL_FLOAT, 0, &(mesh.m_Tex2DBuffer[0]));

        glDrawElements(GL_TRIANGLES, mesh.m_IndexBuffer.size(), GL_UNSIGNED_INT, &(mesh.m_IndexBuffer[0]));

        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    glUseProgram( 0 );

    glBindTexture(GL_TEXTURE_2D, 0);

}
void MD5Model::Update(float fDeltaTime){
	if(m_bHasAnimation){
		m_Animation.Update(fDeltaTime);
		const MD5Animation::FrameSkeleton& skeleton = m_Animation.GetSkeleton();

		for(unsigned int i = 0; i < m_Meshes.size(); ++i){
			PrepareMesh(m_Meshes[i], skeleton);
		}
	}
}

void MD5Model::RenderNormals(const MD5Mesh& mesh)
{

    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_LIGHTING );

    glColor3f( 1.0f, 1.0f, 0.0f );// Yellow

    glBegin( GL_LINES );
    {
        for ( unsigned int i = 0; i < mesh.m_PositionBuffer.size(); ++i )
        {
            glm::vec3 p0 = mesh.m_PositionBuffer[i];
            glm::vec3 p1 = ( mesh.m_PositionBuffer[i] + mesh.m_NormalBuffer[i] );

            glVertex3fv( glm::value_ptr(p0) );
            glVertex3fv( glm::value_ptr(p1) );
        }
    }
    glEnd();

//    glColor3f( 0.0f, 1.0f, 1.0f );
//
//    glBegin( GL_LINES );
//    {
//        for ( unsigned int i = 0; i < mesh.m_PositionBuffer.size(); ++i )
//        {
//            glm::vec3 p0 = mesh.m_PositionBuffer[i];
//            glm::vec3 p1 = ( mesh.m_PositionBuffer[i] + mesh.m_TangentBuffer[i] );
//
//            glVertex3fv( glm::value_ptr(p0) );
//            glVertex3fv( glm::value_ptr(p1) );
//        }
//    }
//    glEnd();

    glPopAttrib();
}

void MD5Model::RenderSkeleton( const JointList& joints )
{
    glPointSize( 5.0f );
    glColor3f( 1.0f, 0.0f, 0.0f );

    glPushAttrib( GL_ENABLE_BIT );

    glDisable(GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    // Draw the joint positions
    glBegin( GL_POINTS );{
        for ( unsigned int i = 0; i < joints.size(); ++i ){
            glVertex3fv( glm::value_ptr(joints[i].m_Pos) );
        }
    }
    glEnd();

    // Draw the bones
    glColor3f( 0.0f, 1.0f, 0.0f );
    glBegin( GL_LINES );{
        for ( unsigned int i = 0; i < joints.size(); ++i ){
            const Joint& j0 = joints[i];
            if ( j0.m_ParentID != -1 ){
                const Joint& j1 = joints[j0.m_ParentID];
                glVertex3fv( glm::value_ptr(j0.m_Pos) );
                glVertex3fv( glm::value_ptr(j1.m_Pos) );
            }
        }
    }
    glEnd();

    glPopAttrib();

}

void MD5Model::DrawFirstPass()
{
    Render();
}

