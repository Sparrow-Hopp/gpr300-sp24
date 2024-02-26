#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Vertex position in model space
layout(location = 1) in vec3 vNormal; //Vertex position in model space
layout(location = 2) in vec2 vTexCoord; //Vertex texture coordinate (UV)
uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

out vec2 UV;

void main(){
	float u = (((uint(gl_VertexID)+2u) / 3u) % 2u);
	float v = (((uint(gl_VertexID)+1u) / 3u) % 2u);
	UV = vec2(u,v);
	gl_Position = vec4(-1.0+u*2.0,-1.0+v*2.0,0.0,1.0);
}

