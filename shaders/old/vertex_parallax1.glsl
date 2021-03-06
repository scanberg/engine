varying vec3 lightVec; 
varying vec3 viewVec;
varying vec2 texCoord;
varying vec4 ShadowCoord;
varying vec3 normal;
varying vec3 lightDir;

attribute vec3 tangent;         
	
void main(void)
{
	
	gl_Position = ftransform();
	texCoord = gl_MultiTexCoord0.xy;
		
	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
	normal = n;
	vec3 t = normalize(gl_NormalMatrix * tangent);
	vec3 b = cross(n, t);
		
	vec3 v;
	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);
	vec3 lVec = gl_LightSource[0].position.xyz - vVertex;
	
	v.x = dot(lVec, t);
	v.y = dot(lVec, b);
	v.z = dot(lVec, n);
	lightVec = v;
	
	vec3 vVec = -vVertex;
	v.x = dot(vVec, t);
	v.y = dot(vVec, b);
	v.z = dot(vVec, n);
	viewVec = v;

	vec3 lDir = gl_LightSource[0].spotDirection.xyz;
	v.x = dot(lDir, t);
	v.y = dot(lDir, b);
	v.z = dot(lDir, n);
	lightDir = v;

	ShadowCoord= gl_TextureMatrix[3] * gl_Vertex;
}