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
uniform vec3 _LightPos;
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

struct Shadow{
	float camDistance;
	float camSize;
	float minBias;
	float maxBias;
};
uniform Shadow _Shadow;

float calcShadow(sampler2D shadowMap, vec4 lightSpacePos, float bias)
{
	//Homogeneous Clip space to NDC [-w,w] to [-1,1]
    vec3 sampleCoord = lightSpacePos.xyz / lightSpacePos.w;
    //Convert from [-1,1] to [0,1]
    sampleCoord = sampleCoord * 0.5 + 0.5;

	float myDepth = sampleCoord.z - bias; 

	float totalShadow = 0;
	vec2 texelOffset = 1.0 /  textureSize(shadowMap,0);
	for(int y = -1; y <=1; y++){
		for(int x = -1; x <=1; x++)
		{
			vec2 uv = sampleCoord.xy + vec2(x * texelOffset.x, y * texelOffset.y);
			totalShadow+=step(texture(shadowMap,uv).r,myDepth);
		}
	}
	totalShadow/=9.0;

	return totalShadow;
}

void main()
{
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(fs_in.WorldNormal);
	// ambient
    vec3 ambient = _AmbientColor * _LightColor * _Material.Ka;
    // diffuse
    vec3 lightDir = normalize(_LightPos - fs_in.WorldPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = _Material.Kd * diff * _LightColor;
    // specular
    vec3 viewDir = normalize(_EyePos - fs_in.WorldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);
    vec3 specular = _Material.Ks * spec * _LightColor;    
    // calculate shadow
	//Usual Blinn-phong calculation
	//1: in shadow, 0: out of shadow
	float minBias = 0.005; //Example values! 
	float maxBias = 0.015;
	float bias = max(_Shadow.maxBias * (1.0 - dot(normal,_LightPos)),_Shadow.minBias);
	float shadow = calcShadow(_ShadowMap, fs_in.LightSpacePos, bias); 
	vec3 light = objectColor * (ambient + (diffuse + specular) * (1.0 - shadow));
	FragColor = vec4(light,1.0);
}
