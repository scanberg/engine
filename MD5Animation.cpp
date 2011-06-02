#include "MD5Animation.h"
#include <iostream>

MD5Animation::MD5Animation()
: m_iMD5Version( 0 )
, m_iNumFrames( 0 )
, m_iNumJoints( 0 )
, m_iFramRate( 0 )
, m_iNumAnimatedComponents( 0 )
, m_fAnimDuration( 0 )
, m_fFrameDuration( 0 )
, m_fAnimTime( 0 )
{

}

MD5Animation::~MD5Animation()
{}

bool MD5Animation::LoadAnimation(const std::string& filename){
	//if(!fs::existes(filename)){
	//	std::cerr << "MD5Animation::LoadAnimation: Failed to find file: " << filename << std::endl;
	//	return false;
	//}
	//fs::path filePath = filename;

	std::string param;
	std::string junk; // Read junk from the file;

	//fs::ifstream file(filename);
	//int fileLenght = GetFileLength(file);
	//assert(fileLength > 0);

	std::ifstream file;
	// try to open filename
	file.open(filename.c_str(), std::ifstream::in);

	if( file.fail() )
		return false;


	m_JointInfos.clear();
	m_Bounds.clear();
	m_BaseFrames.clear();
	m_Frames.clear();
	m_AnimatedSkeleton.m_Joints.clear();
	m_iNumFrames = 0;

	file >> param;

	while(!file.eof()){
		if(param == "MD5Version"){
			file >> m_iMD5Version;
			assert(m_iMD5Version == 10);
		}
		else if(param == "commandline"){
			Ignore(file);
			//file.ignore(fileLength, '\n'); // Ignore everything else on the line
		}
		else if(param == "numFrames"){
			file >> m_iNumFrames;
			Ignore(file);
		//	file.ignore(fileLength, '\n');
		}
        else if ( param == "numJoints" )
        {
            file >> m_iNumJoints;
            Ignore(file);
        }
        else if ( param == "frameRate" )
        {
            file >> m_iFramRate;
            Ignore(file);
        }
		else if(param == "numAnimatedComponents"){
			file >> m_iNumAnimatedComponents;
			Ignore(file);
			//file.ignore(fileLength, '\n');
		}
		else if(param == "hierarchy"){
			file >> junk; // Read in the '{' character
			for(int i = 0; i < m_iNumJoints; ++i){
				JointInfo joint;
				file >> joint.m_Name >> joint.m_ParentID >> joint.m_Flags >> joint.m_StartIndex;
				RemoveQuotes(joint.m_Name);

				m_JointInfos.push_back(joint);

				Ignore(file);
			//	file.ignore(fileLength, '\n');
			}
			file >> junk; // Read in '}'
		}
		else if(param == "bounds"){
			file >> junk; // read {
			Ignore(file);
			//file.ignore(fileLength, '\n');
			for(int i = 0; i < m_iNumFrames; ++i){
				Bound bound;
				file >> junk; // Rean '('
				file >> bound.m_Min.x >> bound.m_Min.y >> bound.m_Min.z;
				file >> junk >> junk; // ) and (
				file >> bound.m_Max.x >> bound.m_Max.y >> bound.m_Max.z;

				m_Bounds.push_back(bound);

				Ignore(file);
			//	file.ignore(fileLength, '\n');
			}
			file >> junk; // read in }
			Ignore(file);
			//file.ignore(fileLenght, '\n');
		}
		else if(param == "baseframe"){
			file >> junk; // {
			Ignore(file);
			//file.ignore(fileLength, '\n');

			for(int i = 0; i < m_iNumJoints; ++i){
				BaseFrame baseFrame;
				file >> junk;
				file >> baseFrame.m_Pos.x >> baseFrame.m_Pos.y >> baseFrame.m_Pos.z;
				file >> junk >> junk;
				file >> baseFrame.m_Orient.x >>baseFrame.m_Orient.y >> baseFrame.m_Orient.z;
				Ignore(file);
				//file.ignore(fileLength, '\n');

				m_BaseFrames.push_back(baseFrame);
			}
			file >> junk; // Read }
			Ignore(file);
			//file.ignore(fileLength, '\n');
		}
		else if(param == "frame"){
			FrameData frame;
			file >> frame.m_iFrameID >> junk; // Read {
			Ignore(file);
			//file.ignore(fileLength, '\n');

			for(int i = 0; i < m_iNumAnimatedComponents; ++i){
				float frameData;
				file >> frameData;
				frame.m_FrameData.push_back(frameData);
			}
			m_Frames.push_back(frame);

			// Build a skeleton for this frame
			BuildFrameSkeleton(m_Skeletons, m_JointInfos, m_BaseFrames, frame);
			file >> junk; // Read in the }
			Ignore(file);
			//file.ignore(fileLength, '\n');
		}
		//std::cout<<param<<std::endl;
		file >> param;
	} // while (!file.eof)
	// Make Sure there are enough joints for the animated skeleton
	m_AnimatedSkeleton.m_Joints.assign(m_iNumJoints, SkeletonJoint());

	m_fFrameDuration = 1.0f / (float)m_iFramRate;
	m_fAnimDuration = (m_fFrameDuration * (float)m_iNumFrames);
	m_fAnimTime = 0.0f;

	assert((int)m_JointInfos.size() == m_iNumJoints);
	assert((int)m_Bounds.size() == m_iNumFrames);
	assert((int)m_BaseFrames.size() == m_iNumJoints);
	assert((int)m_Frames.size() == m_iNumFrames);
	assert((int)m_Skeletons.size() == m_iNumFrames);

	return true;
}

void MD5Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, const JointInfoList& jointInfos, const BaseFrameList& baseFrames, const FrameData& frameData){
	FrameSkeleton skeleton;

	for(unsigned int i = 0; i < jointInfos.size(); ++i){
		unsigned int j = 0;

		const JointInfo& jointInfo = jointInfos[i];
		// Start with the base frame position and orientation.
		SkeletonJoint animatedJoint = baseFrames[i];

		animatedJoint.m_Parent = jointInfo.m_ParentID;

		if(jointInfo.m_Flags & 1){// pos.x
			animatedJoint.m_Pos.x = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}
		if(jointInfo.m_Flags & 2){// pos.y
			animatedJoint.m_Pos.y = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}
		if(jointInfo.m_Flags & 4){// pos.z
			animatedJoint.m_Pos.z = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}
		if(jointInfo.m_Flags & 8){// Orient.x
			animatedJoint.m_Orient.x = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}
		if(jointInfo.m_Flags & 16){// Orient.y
			animatedJoint.m_Orient.y = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}
		if(jointInfo.m_Flags & 32){// Orient.z
			animatedJoint.m_Orient.z = frameData.m_FrameData[jointInfo.m_StartIndex + j++];
		}

		ComputeQuatW(animatedJoint.m_Orient);

		if(animatedJoint.m_Parent >= 0){ // Has parent joint
			SkeletonJoint& parentJoint = skeleton.m_Joints[animatedJoint.m_Parent];
			glm::vec3 rotPos = parentJoint.m_Orient * animatedJoint.m_Pos;

			animatedJoint.m_Pos = parentJoint.m_Pos + rotPos;
			animatedJoint.m_Orient = parentJoint.m_Orient * animatedJoint.m_Orient;

			animatedJoint.m_Orient = glm::normalize(animatedJoint.m_Orient);
		}
		skeleton.m_Joints.push_back(animatedJoint);
	}
	skeletons.push_back(skeleton);
}
void MD5Animation::Update(float fDeltaTime){
	if(m_iNumFrames < 1) return;

	m_fAnimTime += fDeltaTime;

	while(m_fAnimTime > m_fAnimDuration) m_fAnimTime -= m_fAnimDuration;
	while(m_fAnimTime < 0.0f) m_fAnimTime += m_fAnimDuration;
	// Figure out wich frame we're on
	float fFramNum = m_fAnimTime * (float)m_iFramRate;
	int iFrame0 = (int)floorf(fFramNum);
	int iFrame1 = (int)ceilf(fFramNum);
	iFrame0 = iFrame0 % m_iNumFrames;
	iFrame1 = iFrame1 % m_iNumFrames;

	float fInterpolate = fmodf(m_fAnimTime, m_fFrameDuration) / m_fFrameDuration;

	InterpolateSkeletons(m_AnimatedSkeleton, m_Skeletons[iFrame0], m_Skeletons[iFrame1], fInterpolate);
}
void MD5Animation::InterpolateSkeletons(FrameSkeleton& finalSkeleton, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float fInterpolate){
	for(int i = 0; i < m_iNumJoints; ++i){
		SkeletonJoint& finalJoint = finalSkeleton.m_Joints[i];
		const SkeletonJoint& joint0 = skeleton0.m_Joints[i];
		const SkeletonJoint& joint1 = skeleton1.m_Joints[i];

		finalJoint.m_Parent = joint0.m_Parent;

		finalJoint.m_Pos = glm::lerp(joint0.m_Pos, joint1.m_Pos, fInterpolate);
		finalJoint.m_Orient = glm::mix(joint0.m_Orient, joint1.m_Orient, fInterpolate);
	}
}
void MD5Animation::Render(){
    glPointSize( 5.0f );
    glColor3f( 1.0f, 0.0f, 0.0f );

    glPushAttrib( GL_ENABLE_BIT );

    glDisable(GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );

    const SkeletonJointList& joints = m_AnimatedSkeleton.m_Joints;

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
            const SkeletonJoint& j0 = joints[i];
            if ( j0.m_Parent != -1 ){
                const SkeletonJoint& j1 = joints[j0.m_Parent];
                glVertex3fv( glm::value_ptr(j0.m_Pos) );
                glVertex3fv( glm::value_ptr(j1.m_Pos) );
            }
        }
    }
    glEnd();

    glPopAttrib();
}
