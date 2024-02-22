#version 450
out vec4 FragColor; //The color of this fragment

in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
	vec4 LightSpacePos;
}fs_in;

uniform sampler2D _MainTex;
uniform sampler2D _ShadowMap;
uniform vec3 _EyePos;
uniform vec3 _LightPos = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos){
	//Homogeneous Clip space to NDC [-w,w] to [-1,1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    //Convert from [-1,1] to [0,1]
    sampleCoord = sampleCoord * 0.5 + 0.5;
	float myDepth = sampleCoord.z; 
	float shadowMapDepth = texture(shadowMap, sampleCoord.xy).r;
	//step(a,b) returns 1.0 if a >= b, 0.0 otherwise
	return step(shadowMapDepth,myDepth);
}

void main()
{
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(fs_in.WorldNormal);
	//Light pointing straight down
	vec3 toLight = -_LightPos;
	{
		float diffuseFactor = max(dot(normal,toLight),0.0);
		//Calculate specularly reflected light
		vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
		//Blinn-phong uses half angle
		vec3 h = normalize(toLight + toEye);
		float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
		//Combination of specular and diffuse reflection
		vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
		lightColor += _AmbientColor * _Material.Ka;
		FragColor = vec4(objectColor * lightColor,1.0);
	}
	// ambient
    vec3 ambient = _AmbientColor * _Material.Ka;
    // diffuse
    vec3 lightDir = normalize(toLight - fs_in.WorldPos);
    float diff = max(dot(toLight, normal), 0.0);
    vec3 diffuse = _Material.Kd * diff * _LightColor;
    // specular
    vec3 viewDir = normalize(_EyePos - fs_in.WorldPos);
    vec3 halfwayDir = normalize(toLight + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);
    vec3 specular = _Material.Ks * spec * _LightColor;    
    // calculate shadow
	//Usual Blinn-phong calculation
	//1: in shadow, 0: out of shadow
	float shadow = calcShadow(_ShadowMap, fs_in.LightSpacePos); 
	vec3 light = objectColor * (ambient + (diffuse + specular) * (1.0 - shadow));
	FragColor = vec4(light,1.0);
}
