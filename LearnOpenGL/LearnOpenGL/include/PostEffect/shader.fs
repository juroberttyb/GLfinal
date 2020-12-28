#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D scene;
uniform sampler2D ssao;

uniform int PostEffect;
uniform int hdr;
uniform float exposure;

uniform float weight[25] = float[] (1.0 / 273.0, 4.0 / 273.0, 7.0 / 273.0, 4.0 / 273.0, 1.0 / 273.0,
                                    4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
                                    7.0 / 273.0, 26.0 / 273.0, 41.0 / 273.0, 26.0 / 273.0, 7.0 / 273.0,
                                    4.0 / 273.0, 16.0 / 273.0, 26.0 / 273.0, 16.0 / 273.0, 4.0 / 273.0,
                                    1.0 / 273.0, 4.0 / 273.0, 7.0 / 273.0, 4.0 / 273.0, 1.0 / 273.0);

float sigmoid(vec3 v)
{
    float x = 0.2126 * v.r + 0.7152 * v.g + 0.0722 * v.b;

    float y = -4 * x;

    y = 1 / (1 + exp(y));

    return x;
}

void main()
{ 
    vec4 temp = texture(scene, TexCoords);

    if (PostEffect == 0) // bloom effect
    {
        vec2 tex_offset = 1.0 / textureSize(scene, 0); // gets size of single texel
        vec3 result = texture(scene, TexCoords).rgb; // texture(scene, TexCoords).rgb * weight[0]; // current fragment's contribution
        float threshold = 1.0;

        for(int i = -2; i <= 2; ++i)
        {
            for(int j = -2; j <= 2; j++)
            {
                vec3 neib = texture(scene, TexCoords + vec2(tex_offset.x * i, tex_offset.y * j)).rgb;
                float bright = 0.2126 * neib.r + 0.7152 * neib.g + 0.0722 * neib.b;

                if (bright > threshold)
                {
                    result += neib * weight[(i + 2) * 5 + j + 2];
                }
            }
        }

        temp = vec4(result, 1.0);
    }
    else if (PostEffect == 1) // original
    {

    }
    else if (PostEffect == 2) // guassian blur
    {
        int half_size = 2;
        vec4 color_sum = vec4(0);

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
                vec4 neighbor = texelFetch(scene, coord, 0);

                color_sum += neighbor;
            }
        }

        int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur = color_sum / sample_count;

        temp = blur;
    }
    else if (PostEffect == 3) // sigmoid transform
    {
        vec3 value = texture(scene, TexCoords).rgb;
  
        float s =  sigmoid(value);

        value = s * value;
    
        temp = vec4(value, 1.0);
    }

    // hdr
    if (hdr == 1)
    {
        const float gamma = 2.2;
        vec3 hdrColor = temp.rgb;
  
        // exposure tone mapping
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        // gamma correction 
        mapped = pow(mapped, vec3(1.0 / gamma));
  
        temp = vec4(mapped, 1.0);
    }

    FragColor = temp * texture(ssao, TexCoords).r;
}