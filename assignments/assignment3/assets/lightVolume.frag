//lightVolume.frag
#version 450
out vec4 FragColor; //The color of this fragment

uniform vec3 _EyePos;
//layout(binding = i) can be used as an alternative to shader.setInt()
//Each sampler will always be bound to a specific texture unit
uniform layout(binding = 0) sampler2D _gPositions;
uniform layout(binding = 1) sampler2D _gNormals;
uniform layout(binding = 2) sampler2D _gAlbedo;

//Point light UBO
struct PointLight{
	vec3 position;
	float radius;
	vec3 color;
};

#define MAX_POINT_LIGHTS 1024
layout (std140, binding=0) uniform AdditionalLights{
	PointLight _PointLights[MAX_POINT_LIGHTS];
};

uniform int _LightIndex; //set per light volume

//Material uniforms
struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

float attenuateExponential(float distance, float radius)
{
	float i = clamp(1.0 - pow(distance/radius,4.0),0.0,1.0);
	return i * i;
}
//calcPointLight function
vec3 calcPointLight(PointLight light, vec3 worldPos, vec3 worldNormal)
{
	vec3 normal = normalize(worldNormal);
	vec3 diff = light.position - worldPos;
	//Direction toward light position
	vec3 toLight = normalize(diff);

	//TODO: Usual blinn-phong calculations for diffuse + specular
	//Make sure fragment normal is still length 1 after interpolation.
    // diffuse
    float diffuse = max(dot(toLight, normal), 0.0);
    vec3 diffuseFactor = _Material.Kd * diffuse * light.color;
    // specular
    vec3 viewDir = normalize(_EyePos - worldPos);
    vec3 halfwayDir = normalize(toLight + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), _Material.Shininess);
    vec3 specularFactor = _Material.Ks * spec * light.color; 
	vec3 lightColor = (diffuseFactor + specularFactor);

	//Attenuation
	float d = length(diff); //Distance to light
	lightColor *= attenuateExponential(d,light.radius); //See below for attenuation options
	return lightColor;
}

void main(){
	//0-1 UV for sampling gBuffers
	//gl_FragCoord is pixel position of the fragment
    vec2 UV = gl_FragCoord.xy / textureSize(_gNormals,0); 
	//Calculate lighting for single point light
	vec3 normal = texture(_gNormals,UV).xyz;
	vec3 worldPos = texture(_gPositions,UV).xyz;
	//Access this light's data
	PointLight light = _PointLights[_LightIndex];
	vec3 lightColor = calcPointLight(light,worldPos,normal);
	FragColor = vec4(lightColor * texture(_gAlbedo,UV).rgb, 1);
}
