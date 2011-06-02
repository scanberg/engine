varying vec2 texCoord;
varying vec3 normalVec;
varying float depth;

uniform sampler2D diffuseMap;
	
void main (void)
{
	vec3 n = normalize(normalVec);
		
	gl_FragData[0].xyz = texture2D(diffuseMap, texCoord).xyz * gl_FrontMaterial.diffuse.xyz * 0.9;
	gl_FragData[0].w = gl_FrontMaterial.ambient.x;
	float specular = gl_FrontMaterial.specular.x*0.7;
	gl_FragData[1] = vec4(n * 0.5 + 0.5,specular);

	gl_FragDepth = depth;
}