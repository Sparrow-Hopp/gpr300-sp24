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

struct PointLight 
{
	vec3 position;
	float radius;
	vec3 color;
};
#define MAX_POINT_LIGHTS 1024
layout (std140, binding=0) uniform AdditionalLights
{
	PointLight _PointLights[MAX_POINT_LIGHTS];
};

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

float attenuateExponential(float distance, float radius)
{
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
}


vec3 calcPointLight(PointLight light, vec3 normal)
{
	//Sample surface properties for this screen pixel
	vec3 worldNormal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	vec3 albedo = texture(_gAlbedo,UV).xyz;

	vec3 diff = light.position - worldPos;
	//Direction toward light position
	vec3 toLight = normalize(diff);

	//TODO: Usual blinn-phong calculations for diffuse + specular
	//Make sure fragment normal is still length 1 after interpolation.
	// ambient
    vec3 ambient = _AmbientColor * light.color * _Material.Ka;
    // diffuse
    float diffuse = max(dot(toLight, normal), 0.0);
    vec3 diffuseFactor = _Material.Kd * diffuse * light.color;
    // specular
    vec3 viewDir = normalize(_EyePos - worldPos);
    vec3 halfwayDir = normalize(toLight + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);
    vec3 specularFactor = _Material.Ks * spec * light.color; 
	vec3 lightColor = albedo * (ambient + diffuseFactor + specularFactor);

	//Attenuation
	float d = length(diff); //Distance to light
	lightColor *= attenuateExponential(d,light.radius); //See below for attenuation options
	return lightColor;
}

void main()
{ 
	vec3 normal = normalize(texture(_gNormals,UV).xyz);

	vec3 totalLight = calcDirectionalLight();
	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		totalLight += calcPointLight(_PointLights[i], normal);
	}
	FragColor = vec4(totalLight, 1.0);
}
