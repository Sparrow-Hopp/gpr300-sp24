#version 450
out vec4 FragColor; //The color of this fragment
in vec2 UV;

uniform sampler2D _ShadowMap;

uniform vec3 _EyePos;
uniform vec3 _LightPos;
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

uniform mat4 _LightViewProj; //view + projection of light source camera

//layout(binding = i) can be used as an alternative to shader.setInt()
//Each sampler will always be bound to a specific texture unit
uniform layout(binding = 0) sampler2D _gPositions;
uniform layout(binding = 1) sampler2D _gNormals;
uniform layout(binding = 2) sampler2D _gAlbedo;

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

vec3 calcDirectionalLight()
{
	//Sample surface properties for this screen pixel
	vec3 worldNormal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	vec3 albedo = texture(_gAlbedo,UV).xyz;
	vec4 lightSpacePos = _LightViewProj * vec4(worldPos, 1);
	
	//Usual Blinn-phong calculation
	//Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(worldNormal);
	// ambient
    vec3 ambient = _AmbientColor * _LightColor * _Material.Ka;
    // diffuse
    vec3 lightDir = normalize(_LightPos - worldPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = _Material.Kd * diff * _LightColor;
    // specular
    vec3 viewDir = normalize(_EyePos - worldPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);
    vec3 specular = _Material.Ks * spec * _LightColor;  
	
    // calculate shadow
	//1: in shadow, 0: out of shadow
	float bias = max(_Shadow.maxBias * (1.0 - dot(normal,_LightPos)),_Shadow.minBias);
	float shadow = calcShadow(_ShadowMap, lightSpacePos, bias); 
	vec3 light = albedo * (ambient + (diffuse + specular) * (1.0 - shadow));
	return light;
}

void main()
{ 
	vec3 normal = normalize(texture(_gNormals,UV).xyz);

	vec3 totalLight = calcDirectionalLight();
	FragColor = vec4(totalLight, 1.0);
}
