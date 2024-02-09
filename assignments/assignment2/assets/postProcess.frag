#version 450
out vec4 FragColor;
in vec2 UV;
uniform sampler2D _ColorBuffer;

const float offset = 1.0 / 300.0;  

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

    //post processing effect
	vec2 texelSize = _Blur.intensity / textureSize(_ColorBuffer,0).xy;
    vec3 totalColor = vec3(0);
    for(int y = -1; y <= 1; y++){
        for(int x = -1; x <= 1; x++){
            vec2 offset = vec2(x,y) * texelSize;
            int index = (x + 1) + (y + 1) * 3;
            totalColor += texture(_ColorBuffer,UV + offset).rgb * kernel[index];
        }
    }
    totalColor/=(16);
    FragColor = vec4(totalColor,1.0);
}
