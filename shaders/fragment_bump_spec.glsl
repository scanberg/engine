varying vec2 texCoord;
varying vec3 normalVec;
varying vec3 tangentVec;
varying float depth;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
	
void main (void)
{
	vec3 n = normalize(normalVec);
	vec3 t = normalize(tangentVec);
	vec3 b = cross(n,t);
		
	gl_FragData[0].xyz = texture2D(diffuseMap, texCoord).xyz * gl_FrontMaterial.diffuse.xyz;
	gl_FragData[0].w = gl_FrontMaterial.ambient.x;

	vec3 bumpNormal = texture2D(normalMap, texCoord).xyz * 2.0 - 1.0;

	bumpNormal = bumpNormal.x * t + bumpNormal.y * b + bumpNormal.z * n;

	float specular = texture2D(specularMap, texCoord).x * gl_FrontMaterial.specular.x;
	gl_FragData[1] = vec4(bumpNormal * 0.5 + 0.5,specular);

	gl_FragDepth = depth;
}