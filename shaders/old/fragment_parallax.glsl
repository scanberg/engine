varying vec3 lightVec;
varying vec3 viewVec;
varying vec2 texCoord;
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D heightMap;
	
void main (void)
{
	vec3 lVec = normalize(lightVec);
	vec3 vVec = normalize(viewVec);
		
	// Calculate offset, scale & biais
	float height = texture2D(heightMap, texCoord).x;
	vec2 newTexCoord = texCoord + ((height * 0.04 - 0.02) * (vVec.xy));
		
	vec4 base = texture2D(diffuseMap, newTexCoord);
	vec3 bump = normalize(texture2D(normalMap, newTexCoord).xyz * 2.0 - 1.0);
	bump = normalize(bump);

	float diffuse = max( dot(lVec, bump), 0.0 );

	if(diffuse > 0.0)
	{
		
		float specular = pow(clamp(dot(reflect(-vVec, bump), lVec), 0.0, 1.0), gl_FrontMaterial.shininess );
	
		vec4 vAmbient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
		vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * diffuse;	
		vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular *specular;	
		gl_FragColor = vAmbient*base + (vDiffuse*base + vSpecular);	
	}
	else
	{
		gl_FragColor = gl_LightSource[0].ambient*gl_LightSource[0].ambient*base;
	}
}