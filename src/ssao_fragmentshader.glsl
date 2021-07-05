#version 330 core
uniform sampler2D gPosition; // Textura com posicoes
uniform sampler2D gNormal; // Textura com normais
uniform sampler2D noise;

uniform int kernelSize; // Numero de amostras
uniform vec3 samples[64];
uniform mat4 projection;

// Screen size
uniform int width;
uniform int height;

in vec2 fragUV; // Coord de textura do fragmento
out float fragColor;

// tile noise texture over screen, based on screen dimensions divided by noise size
vec2 noiseScale = vec2(width/4.0, height/4.0); // screen = 800x600

void main()
{
    vec3 fragPos = texture(gPosition, fragUV).xyz;
    vec3 normal = texture(gNormal, fragUV).rgb;
    vec3 randomVec = texture(noise, fragUV*noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusionFactor = 0.0;
    float radius = 1.0; // Raio do hemisferio de amostragem
    for(int i = 0; i < kernelSize; i++)
    {
        vec3 samplePos = TBN*samples[i]; // transforma a amostra de espaço tangente para camera
        samplePos = fragPos + samplePos * radius; // calcula a posicao da amostra em relacao ao fragmento

        vec4 offset = projection * vec4(samplePos, 1.0); // coloca em espaço da tela
        offset.xyz /= offset.w; // normalizando
        offset.xyz = offset.xyz*0.5 + 0.5; // clipa entre 0 e 1

        float sampleDepth = texture(gPosition, offset.xy).z; // pegando a profundidade do fragmento amostrado

        // Se a amostra esta ofuscada na cena entao ela contribuira para criar oclusao
        float bias = 0.025;
//        occlusionFactor += (sampleDepth >= samplePos.z+bias ? 1.0 : 0.0);

        // Codigo para checar se não estamos fazendo sampling de algo muito distante,
        // O que geraria oclusão forte em todas as bordas de um objeto caso tenha algo atrás, como uma parede.
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusionFactor += (sampleDepth >= samplePos.z+bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusionFactor = 1.0 - (occlusionFactor / kernelSize); // normalizando o fator de oclusao baseado no numero de amostras
    fragColor = pow(occlusionFactor, 8.0);
}
