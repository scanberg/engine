varying vec3 lightVec;
varying vec3 viewVec;
varying vec2 texCoord;
varying vec3 normal;
varying vec3 lightDir;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D lightMap;
uniform int screenWidth;
uniform int screenHeight;
	
void main (void)
{
	float totLight = 100000.0/dot(lightVec,lightVec);
	totLight = clamp(totLight,0.0,1.0);

	vec2 lightCoord = gl_FragCoord.xy/vec2(screenWidth,screenHeight);
	float shadow=texture2D(lightMap, lightCoord).r;

	vec3 lVec = normalize(lightVec);
	vec3 vVec = normalize(viewVec);
		
	vec4 base = texture2D(diffuseMap, texCoord);
	vec3 bump = normalize(texture2D(normalMap, texCoord).xyz * 2.0 - 1.0);

	float diffuse = max( dot(lVec, bump), 0.0 );

	if(diffuse > 0.0)
	{
		
		float specular = pow(clamp(dot(reflect(-vVec, bump), lVec), 0.0, 1.0), gl_FrontMaterial.shininess );
	
		vec4 vAmbient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
		vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * diffuse;	
		vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular *specular;	
		gl_FragColor = vAmbient*base + totLight*shadow*(vDiffuse*base + vSpecular);	
	}
	else
	{
		gl_FragColor = gl_LightSource[0].ambient*gl_LightSource[0].ambient*base;
	}
	
}