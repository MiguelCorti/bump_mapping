#version 330 core

layout( location = 0 ) in vec3 vertexPos;
layout( location = 1 ) in vec2 vertexTexCoord; // Coordenadas de textura

out vec2 fragUV; // Coordenadas de textura do fragmento

void main()
{
    fragUV = vertexTexCoord;
    gl_Position = vec4(vertexPos, 1.0);
}
