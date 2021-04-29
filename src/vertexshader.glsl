#version 330 core

layout( location = 0 ) in vec3 vertexPos; // Posição do vertex
layout( location = 1 ) in vec3 vertexNormal; // Normal do vertex
layout( location = 2 ) in vec2 vertexTexCoord; // Coordenada da textura
layout( location = 3 ) in vec3 vertexTangent; // Tangente do vertex
struct Light
{
    vec3 position; // Posição da Luz no espaço da câmera
};
uniform Light light;

uniform mat4 mvp; // Model View Projection
uniform mat4 mv; // Model View
uniform mat4 m; // Model
uniform mat4 mv_ti; // Inversa Transposta da MV

out vec2 mapCoord; // Coordenada da textura
out vec3 lightDir; // Direção da Luz (calculada no espaço tangente)
out vec3 viewDir; // Direção do Olho (calculada no espaço tangente)

void main()
{
    gl_Position = mvp * vec4( vertexPos, 1 );
    vec3 fragTan = (vec3( mv*vec4( vertexTangent, 1 ) ));
    vec3 fragPos = (vec3( mv*vec4( vertexPos, 1 ) ));
    vec3 fragNormal = normalize(vec3(mv*vec4( vertexNormal, 0 ) ));

    //fragTan = normalize(fragTan - dot(fragTan, fragNormal)*fragNormal);
    vec3 binormal = cross(fragNormal, fragTan);
    mat3 rotation = (mat3(fragTan, binormal, fragNormal));

    //vec3(inverse(mv)*vec4(light.position, 1))
    lightDir = rotation*(light.position - fragPos);
    viewDir = rotation*(vec3((0.0f,0.0f,5.0f))-fragPos);
    mapCoord = vertexTexCoord;
}
