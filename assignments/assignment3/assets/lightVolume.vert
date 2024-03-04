#version 450
uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

void main()
{
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}

