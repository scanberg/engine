//arse.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <utility>
#include "AseReader.h"

using namespace std;

string extract_string (istream &is, string s)
{
	int lastspace;
	getline(is,s);
	string b;
	lastspace=s.find_last_of(" ");
	b = s.substr(lastspace);
	return (b);
}

int extract_int (istream &is, string s)
{
	int lastspace;
	getline(is,s);
	int b;
	lastspace=s.find_last_of(" ");
	b = atoi(s.substr(lastspace).c_str());
	return (b);
}

unsigned int extract_singleint (string s, int pos)
{
	int len;
	len = (s.find(" ",pos))-(pos);
	unsigned int b;
	b=atoi(s.substr(pos,len).c_str());
	return (b);
}

float extract_vertexfloat (string s, int pos)
{
	int len;
	len = (s.find(" ",pos))-(pos);
	float b;
	b=atof(s.substr(pos,len).c_str());
	return (b);
}

int extract_normalint(string s, int pos)
{
	int len;
	len = (s.find(" ",pos))-(pos);
	int b;
	b=atoi(s.substr(pos,len).c_str());
	return (b);
}

int LoadAse(string filename, Mesh &mesh)
{
	//constants
	string SCENE_FILENAME, NODE_NAME;
	int SCENE_FIRSTFRAME, SCENE_LASTFRAME, TIMEVALUE, MESH_NUMVERTEX, MESH_NUMFACES, MESH_NUMTVERTEX, MESH_NUMTVFACES;
	vector<float>	MESH_VERTEX_LIST_X, MESH_VERTEX_LIST_Y, MESH_VERTEX_LIST_Z,
								MESH_TVERTLIST_U, MESH_TVERTLIST_V;
    vector<int> MESH_VERTEX_SMOOTHING, MESH_VERTEX_TIMESLOADED;


	//variables
    filebuf fb;
    fb.open (filename.c_str(),ios::in);
    istream is(&fb);
	string s;
	int x, pos;
	vector<TVertex> vectex;
	vector<Face> faceex;
	while (getline(is, s))
	{
		if (s.find("*SCENE {") != string::npos)
		{
			SCENE_FILENAME = extract_string(is,s);
			//cout << "SCENE_FILENAME " << SCENE_FILENAME <<  endl;

			SCENE_FIRSTFRAME = extract_int(is,s);
			//cout << "SCENE_FIRSTFRAME " << SCENE_FIRSTFRAME <<  endl;

			SCENE_LASTFRAME = extract_int(is,s);
			//cout << "SCENE_LASTFRAME " << SCENE_LASTFRAME <<  endl;

			//omitting the following constants for now
			getline(is, s); //SCENE_FRAMESPEED
			getline(is, s); //SCENE_TICKSPERFRAME
			getline(is, s); //SCENE_BACKGROUND_STATIC
			getline(is, s); //SCENE_AMBIENT_STATIC
		}
		else if (s.find("*GEOMOBJECT {") != string::npos)
		{
			NODE_NAME= extract_string(is,s);
			//cout << "NODE_NAME" << NODE_NAME <<  endl;
			//omitting the following constants for now
			getline(is, s); //INHERIT_POS
			getline(is, s); //INHERIT_ROT
			getline(is, s); //INHERIT_SCL
			getline(is, s); //TM_ROW0
			getline(is, s); //TM_ROW1
			getline(is, s); //TM_ROW2
			getline(is, s); //TM_ROW3
			getline(is, s); //TM_POS
			getline(is, s); //TM_ROTAXIS
			getline(is, s); //TM_ROTANGLE
			getline(is, s); //TM_SCALE
			getline(is, s); //TM_SCALEAXIS
			getline(is, s); //TM_SCALEAXISANG
		}

		//finding MESH section
		else if (s.find("*MESH {") != string::npos)
		{
			TIMEVALUE = extract_int(is,s);
			//cout << "TIMEVALUE " << TIMEVALUE <<  endl;

			MESH_NUMVERTEX = extract_int(is,s);
			//cout << "MESH_NUMVERTEX " << MESH_NUMVERTEX  << endl;

			MESH_NUMFACES = extract_int(is,s);
			//cout << "MESH_NUMFACES " << MESH_NUMFACES  << endl;
		}
		else if (s.find("*MESH_VERTEX_LIST {") != string::npos)
		{

			bool loop = true;
			x=0;
			TVertex tempvertex;
			while(loop == true)
			{
				getline(is,s);
				if (s.find("}") != string::npos) //escape da lup
				{
					loop = false;;
				}
				else
				{
					pos=s.find("X");
					pos=s.find_first_not_of(" \t",pos+1);

					//get x pos
					pos = (s.find_first_of(" \t",pos))+1;
					//MESH_VERTEX_LIST_X.push_back (extract_vertexfloat(s,pos));
					tempvertex.x = extract_vertexfloat(s,pos);
					//cout << tempvertex.x;
					//get y pos
					pos = (s.find_first_of(" \t",pos))+1;
					//MESH_VERTEX_LIST_Y.push_back (extract_vertexfloat(s,pos));
					tempvertex.y = extract_vertexfloat(s,pos);

					//get z pos
					pos = (s.find_first_of(" \t",pos))+1;
					//MESH_VERTEX_LIST_Z.push_back (extract_vertexfloat(s,pos));
					tempvertex.z = extract_vertexfloat(s,pos);
					tempvertex.val = x;
					tempvertex.sg = 0;

					vectex.push_back(tempvertex);


					//cout << "MESH_VERTEX " << x << ": "<< (vectex.at(x)).x << "\t";
					//cout << (vectex.at(x)).y << "\t" << (vectex.at(x)).z << endl;
					//cout << "MESH_VERTEX " << x << ": "<< MESH_VERTEX_LIST_X.at(x) << "\t";
					//cout << MESH_VERTEX_LIST_Y.at(x) << "\t" << MESH_VERTEX_LIST_Z.at(x) << endl;
					x++;
				}
			}
		}
		else if (s.find("*MESH_FACE_LIST {") != string::npos)
		{
			bool loop=true;
			x=0;
			Face tempface;
			unsigned int sg;
			TVertex tempvertex;
			while (loop == true)
			{
				getline(is,s);
				if (s.find("}") != string::npos) //escape da lup
				{
					loop = false;
				}
				else
				{
					pos=s.find("C")+1;
					//find n:
					pos=s.find_first_not_of(" \t",pos+1);
					//find blank, then A:, then blank
					pos=s.find_first_of(" \t",pos);
					pos=s.find_first_not_of(" \t",pos);
					pos=s.find_first_of(" \t",pos)+1;
					tempface.point[0] = extract_singleint(s,pos);

					pos=s.find_first_of(" \t",pos);
					pos=s.find_first_not_of(" \t",pos);
					pos=s.find_first_of(" \t",pos)+1;
					tempface.point[1] = extract_singleint(s,pos);

					pos=s.find_first_of(" \t",pos);
					pos=s.find_first_not_of(" \t",pos);
					pos=s.find_first_of(" \t",pos)+1;
					tempface.point[2] = extract_singleint(s,pos);

					pos=s.find("G");
					pos=s.find_first_not_of(" \t",pos+1);
					sg = extract_singleint(s,pos);

					for(unsigned int i=0; i < vectex.size(); i++)
					{
                        if (vectex.at(i).sg != sg && vectex.at(i).val == tempface.point[0])
                        {
                            if (vectex.at(i).sg == 0)
                            {
                                vectex.at(i).sg = sg;
                            }
                            else
                            {
                                tempvertex.x = vectex.at(i).x;
                                tempvertex.y = vectex.at(i).y;
                                tempvertex.z = vectex.at(i).z;
                                tempvertex.val = vectex.at(i).val;
                                tempvertex.sg = sg;
                                vectex.push_back(tempvertex);
                                tempface.point[0] = vectex.size()-1;
                            }
                        }
                        if (vectex.at(i).sg != sg && vectex.at(i).val == tempface.point[1])
                        {
                            if (vectex.at(i).sg == 0)
                            {
                                vectex.at(i).sg = sg;
                            }
                            else
                            {
                                tempvertex.x = vectex.at(i).x;
                                tempvertex.y = vectex.at(i).y;
                                tempvertex.z = vectex.at(i).z;
                                tempvertex.val = vectex.at(i).val;
                                tempvertex.sg = sg;
                                vectex.push_back(tempvertex);
                                tempface.point[1] = vectex.size()-1;
                            }
                        }
                        if (vectex.at(i).sg != sg && vectex.at(i).val == tempface.point[2])
                        {
                            if (vectex.at(i).sg == 0)
                            {
                                vectex.at(i).sg = sg;
                            }
                            else
                            {
                                tempvertex.x = vectex.at(i).x;
                                tempvertex.y = vectex.at(i).y;
                                tempvertex.z = vectex.at(i).z;
                                tempvertex.val = vectex.at(i).val;
                                tempvertex.sg = sg;
                                vectex.push_back(tempvertex);
                                tempface.point[2] = vectex.size()-1;
                            }
                        }
					}

					faceex.push_back(tempface);

					x++;
				}
			}

			MESH_NUMTVERTEX = extract_int(is, s);
		}
		else if (s.find("*MESH_TVERTLIST {") != string::npos)
		{
			bool loop=true;
			x=0;
			while (loop == true)
			{
				getline(is,s);
				if (s.find("}") != string::npos) //escape da lup
				{
					loop = false;
				}
				else
				{
					pos=s.find("R")+1;
					pos=s.find_first_not_of(" \t",pos+1);
					pos = (s.find_first_of(" \t",pos))+1;

					//cout<<"hej "<<s<<endl;
					//cout<<"pos "<<extract_vertexfloat(s,pos)<<endl;

					MESH_TVERTLIST_U.push_back(extract_vertexfloat(s,pos));
					//cout<<"value "<<MESH_TVERTLIST_U.at(MESH_TVERTLIST_U.size()-1)<<endl;
					pos = (s.find_first_of(" \t",pos))+1;
					MESH_TVERTLIST_V.push_back(extract_vertexfloat(s,pos));
				}
				x++;
			}
			MESH_NUMTVFACES = extract_int(is, s);
		}
		else if (s.find("*MESH_TFACELIST {") != string::npos)
		{
			cout << vectex.size() << endl;
			bool loop=true;
			x=0;
			int p0, p1, p2, index;
			while (loop == true)
			{
				getline(is,s);
				if (s.find("}") != string::npos) //escape da lup
				{
					loop = false;;
				}
				else
				{
					pos=s.find("C")+1;
					pos=s.find_first_of(" \t",pos)+1;
					index = extract_singleint(s,pos);

                    cout<<"index "<<index<<endl;
					//get first point
					pos = (s.find_first_of(" \t",pos))+1;
					p0 = extract_singleint(s,pos);
					//get second point
					pos = (s.find_first_of(" \t",pos))+1;
					p1 = extract_singleint(s,pos);
					//get third point
					pos = (s.find_first_of(" \t",pos))+1;
					p2 = extract_singleint(s,pos);

					vectex.at( faceex.at(index).point[0] ).u = MESH_TVERTLIST_U.at(p0);
					vectex.at( faceex.at(index).point[0] ).v = MESH_TVERTLIST_V.at(p0);

                    vectex.at( faceex.at(index).point[1] ).u = MESH_TVERTLIST_U.at(p1);
					vectex.at( faceex.at(index).point[1] ).v = MESH_TVERTLIST_V.at(p1);

                    vectex.at( faceex.at(index).point[2] ).u = MESH_TVERTLIST_U.at(p2);
					vectex.at( faceex.at(index).point[2] ).v = MESH_TVERTLIST_V.at(p2);
                }
				x++;
			}
		}

	}

	cout<<endl<<endl<<"DONE!"<<endl;
	mesh.Init(vectex.size(),faceex.size());
    unsigned int i;

    for(i=0;i<vectex.size();i++)
    {
        mesh.vertex[i].x=vectex.at(i).x;
        mesh.vertex[i].y=vectex.at(i).y;
        mesh.vertex[i].z=vectex.at(i).z;

        //mesh.vertex[i].nx=vectex.at(i).nx;
        //mesh.vertex[i].ny=vectex.at(i).ny;
        //mesh.vertex[i].nz=vectex.at(i).nz;

        mesh.vertex[i].u=vectex.at(i).u;
        mesh.vertex[i].v=vectex.at(i).v;
    }

    for(i=0;i<faceex.size();i++)
    {
        mesh.face[i].point[0]=faceex.at(i).point[0];
        mesh.face[i].point[1]=faceex.at(i).point[1];
        mesh.face[i].point[2]=faceex.at(i).point[2];
    }

	return 0;
}
