#version 450
uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix
layout(location = 0) in vec3 vPos; //Vertex position in model space

void main()
{
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}

