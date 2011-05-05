varying vec3 lightVec;
varying vec3 viewVec;
varying vec2 texCoord;

varying vec4 ShadowCoord;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D ShadowMap;


	
void main (void)
{
	vec4 shadowCoordinateWdivide = ShadowCoord / ShadowCoord.w;


	
//Used to lower moiré pattern and self-shadowing

	//shadowCoordinateWdivide.z += 0.0005;

	float distanceFromLight = texture2D(ShadowMap,shadowCoordinateWdivide.st).z;


	
float shadow = 1.0;
 
	if (ShadowCoord.w > 0.0)
 
		shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.5 : 1.0 ;


	float totLight = 20000.0/dot(lightVec,lightVec);
	totLight = clamp(totLight,0.0,1.0);
	totLight = 1.0;

	//Normalize the incomming vectors
	vec3 lVec = normalize(lightVec);
	vec3 vVec = normalize(viewVec);
		
	// Calculate offset, scale & biais
	//float height = texture2D(normalMap, texCoord).w; // Get the forth component in normalMap
	//vec2 newTexCoord;// = texCoord + ((height * 0.04 - 0.02) * (vVec.xy));
		
	vec4 base = texture2D(diffuseMap, texCoord);
	vec3 bump = normalize(texture2D(normalMap, texCoord).xyz * 2.0 - 1.0);

	float diffuse = max( dot(lVec, bump), 0.0 );

	float specular = pow(clamp(dot(reflect(-vVec, bump), lVec), 0.0, 1.0), gl_FrontMaterial.shininess );
	
	vec4 vAmbient = gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
	vec4 vDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse * diffuse;	
	vec4 vSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular *specular;	
	gl_FragColor = vAmbient*base + shadow*totLight*(vDiffuse*base + vSpecular);

}