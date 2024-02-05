#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

const float offset = 1.0 / 300.0;  

in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
}fs_in;

struct Blur{
    float intensity; 
};

uniform sampler2D _MainTex; 
uniform vec3 _EyePos;
uniform vec3 _LightDirection = vec3(0.0,-1.0,0.0);
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};

uniform Blur _Blur;
uniform Material _Material;

void main(){
	vec3 color = texture(_ColorBuffer,UV).rgb;

    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

	float kernel[9] = float[](
        1, 2, 1,
        2,  4, 2,
        1, 2, 1
    );

    //Make sure fragment normal is still length 1 after interpolation.
	vec3 normal = normalize(fs_in.WorldNormal);
	//Light pointing straight down
	vec3 toLight = -_LightDirection;
	float diffuseFactor = max(dot(normal,toLight),0.0);
	//Calculate specularly reflected light
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	//Blinn-phong uses half angle
	vec3 h = normalize(toLight + toEye);
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	//Combination of specular and diffuse reflection
	vec3 lightColor = (_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor;
	lightColor+=_AmbientColor * _Material.Ka;
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;

    //post processing effect
	vec2 texelSize = _Blur.intensity / textureSize(_ColorBuffer,0).xy;
    vec3 totalColor = vec3(0);
    for(int y = -2; y <= 2; y++){
        for(int x = -2; x <= 2; x++){
            vec2 offset = vec2(x,y) * texelSize;
            totalColor += texture(_ColorBuffer,fs_in.TexCoord + offset).rgb;
        }
    }
    totalColor/=(5 * 5);
    FragColor = vec4(totalColor * lightColor,1.0);
}
