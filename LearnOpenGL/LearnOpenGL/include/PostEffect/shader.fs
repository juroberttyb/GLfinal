#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D scene;
uniform sampler2D ssao;

uniform int PostEffect;

float sigmoid(vec3 v)
{
    float x = 0.2126 * v.r + 0.7152 * v.g + 0.0722 * v.b;

    float y = -4 * x;

    y = 1 / (1 + exp(y));

    return x;
}

void main()
{ 
    if (PostEffect == 0)
    {
        FragColor = texture(scene, TexCoords) * texture(ssao, TexCoords).r;
    }
    else if (PostEffect == 2) // sigmoid transform
    {
        vec3 value = texture(scene, TexCoords).rgb;
  
        float s =  sigmoid(value);

        value = s * value;
    
        FragColor = vec4(value, 1.0);
    }
    else if (PostEffect == 1) // guassian blur
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

        FragColor = blur;
    }
    else if (PostEffect == 3) // bloom effect
    {
        int half_size = 2;
        vec4 color_sum = vec4(0);
        float threshold = 0.5;

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
                vec4 neighbor = texelFetch(scene, coord, 0);

                float bright = 0.2126 * neighbor.x + 0.7152 * neighbor.y + 0.0722 * neighbor.z;

                if (bright >= threshold)
                {
                    color_sum += neighbor;
                }
                else
                {
                    color_sum += vec4(0);
                }

                // color_sum += texelFetch(scene, coord, 0);
            }
        }

        int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur = color_sum / sample_count;



        half_size = 4;
        color_sum = vec4(0);

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
                vec4 neighbor = texelFetch(scene, coord, 0);

                float bright = 0.2126 * neighbor.x + 0.7152 * neighbor.y + 0.0722 * neighbor.z;

                if (bright >= threshold)
                {
                    color_sum += neighbor;
                }
                else
                {
                    color_sum += vec4(0);
                }

                // color_sum += texelFetch(scene, coord, 0);
            }
        }

        sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur2 = color_sum / sample_count;




        FragColor = 0.25 * blur2 + 0.5 * blur + texture(scene, TexCoords);
    }
}