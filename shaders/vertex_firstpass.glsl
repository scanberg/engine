varying vec2 texCoord;
varying vec3 normalVec;
varying vec3 tangentVec;
varying vec4 position;
varying vec3 viewVec;

attribute vec3 tangent;
	
void main(void)
{
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;

	position = gl_ModelViewMatrix * gl_Position;
		
	normalVec = gl_NormalMatrix * gl_Normal;
	tangentVec = gl_NormalMatrix * tangent;

	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	vec3 n = normalize(normalVec);
	vec3 t = normalize(tangentVec);
	vec3 b = cross(n, t);

	vec3 vVec = -vVertex;
	vec3 v;
	v.x = dot(vVec, t);
	v.y = dot(vVec, b);
	v.z = dot(vVec, n);
	viewVec = v;
}