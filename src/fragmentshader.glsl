#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColorSpec;

uniform sampler2D sampler; // Textura de Cor

in vec3 fragPos; // Posicao do fragmento na camera
in vec3 fragNormal; // Normal do fragmento
in vec2 fragUV; // Coord de textura do fragmento

void main()
{
    gPosition = fragPos; // guarda a pos do fragmento no primeiro buffer
    gNormal = normalize(fragNormal); // guarda a normal do fragmento
    gColorSpec.rgb = texture(sampler, fragUV).rgb; // guarda o valor da componente difusa
    gColorSpec.a = 50.0; // intensidade especular hard coded
}
