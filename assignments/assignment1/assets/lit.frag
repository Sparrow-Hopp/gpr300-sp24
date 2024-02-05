#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

const float offset = 1.0 / 300.0;  

in Surface{
	vec3 WorldPos; //Vertex position in world space
	vec3 WorldNormal; //Vertex normal in world space
	vec2 TexCoord;
}vs_in;

struct Blur{
    float intensity; 
};

uniform Blur _Blur;

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

	vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        kernel[i] /= 16;
        sampleTex[i] = vec3(texture(_ColorBuffer, vs_in.TexCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

	FragColor = vec4(col,1.0);
}
