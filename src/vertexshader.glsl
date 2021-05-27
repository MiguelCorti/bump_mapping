#version 330 core

layout( location = 0 ) in vec3 vertexPos;
layout( location = 1 ) in vec3 vertexNormal;
layout( location = 2 ) in vec2 vertexTexCoord; // Coordenadas de textura

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 mv;
uniform mat4 mv_ti; // Inversa transposta da MV
uniform mat4 mvp;
uniform mat4 lightSpaceMatrix;

out vec3 fragPos; // Posição no espaço da câmera
out vec4 fragPosLightSpace; // Posição no espaço da luz
out vec3 fragNormal; // Normal no espaço da câmera
out vec2 fragUV; // Coordenadas de textura do fragmento

void main()
{
    fragPos = vec3( model * vec4(vertexPos, 1.0) );
    fragNormal = vec3( mv_ti * vec4( vertexNormal, 0 ) );
    fragUV = vertexTexCoord;
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    gl_Position = mvp * vec4(vertexPos, 1.0);
}
