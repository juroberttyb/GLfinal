#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;
uniform sampler2D screen;

uniform sampler2D noise;
uniform int mode;

uniform int xpos;
uniform int ypos;

uniform int width;
uniform int height;

uniform float time;

void main()
{ 
    if (mode == 0)
    {
        FragColor = texture(screen, TexCoords);
    }
    else if (mode == 1)
    {
        int half_size = 2;
        vec4 color_sum = vec4(0);

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
                color_sum += texelFetch(screen, coord, 0);
            }
        }

        int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur = color_sum / sample_count;

        int quan = 8;
        vec4 quantized = vec4(floor(blur.xyz * quan) / quan, 1.0);

        // const float offset = 1.0 / 300.0;
        float offset = 1.0 / (height / 2.0);

        vec2 offsets[9] = vec2[](
            vec2(-offset,  offset), // top-left
            vec2( 0.0f,    offset), // top-center
            vec2( offset,  offset), // top-right
            vec2(-offset,  0.0f),   // center-left
            vec2( 0.0f,    0.0f),   // center-center
            vec2( offset,  0.0f),   // center-right
            vec2(-offset, -offset), // bottom-left
            vec2( 0.0f,   -offset), // bottom-center
            vec2( offset, -offset)  // bottom-right
        );

        float kernel[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        );

        vec3 sampleTex[9];
        for(int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screen, TexCoords.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for(int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = (3 * quantized + vec4(col, 1.0)) / 4;
    }
    else if (mode == 2)
    {
        vec4 noise_color = texture(noise, TexCoords);
        ivec2 noise_coord = ivec2(gl_FragCoord.xy + noise_color.xy * 10);

        int half_size = 3;
        vec4 color_sum = vec4(0);

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = noise_coord + ivec2(i, j);
                color_sum += texelFetch(screen, coord, 0);
            }
        }

        int sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur = color_sum / sample_count;

        int quan = 8;
        vec4 quantized = vec4(floor(blur.xyz * quan) / quan, 1.0);

        FragColor = quantized;
    }
    else if (mode == 3)
    {
        float radius = 128.0, scale = 0.5;

        if (distance(gl_FragCoord.xy, vec2(xpos, ypos)) < radius) // (gl_FragCoord.x < width && gl_FragCoord.y < height)
        {

            float length = distance(gl_FragCoord.xy, vec2(xpos, ypos));

            vec2 unit_vec = normalize(gl_FragCoord.xy - vec2(xpos, ypos));

            FragColor = texelFetch(screen, ivec2(xpos, ypos) + ivec2(scale * length * unit_vec), 0);

            // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            FragColor = texture(screen, TexCoords);
        }
    }
    else if (mode == 4) // bloom effect
    {
        int half_size = 2;
        vec4 color_sum = vec4(0);
        float threshold = 0.5;

        for (int i = -half_size; i <= half_size; ++i)
        {
            for (int j = -half_size; j <= half_size; ++j)
            {
                ivec2 coord = ivec2(gl_FragCoord.xy) + ivec2(i, j);
                vec4 neighbor = texelFetch(screen, coord, 0);

                float bright = 0.2126 * neighbor.x + 0.7152 * neighbor.y + 0.0722 * neighbor.z;

                if (bright >= threshold)
                {
                    color_sum += neighbor;
                }
                else
                {
                    color_sum += vec4(0);
                }

                // color_sum += texelFetch(screen, coord, 0);
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
                vec4 neighbor = texelFetch(screen, coord, 0);

                float bright = 0.2126 * neighbor.x + 0.7152 * neighbor.y + 0.0722 * neighbor.z;

                if (bright >= threshold)
                {
                    color_sum += neighbor;
                }
                else
                {
                    color_sum += vec4(0);
                }

                // color_sum += texelFetch(screen, coord, 0);
            }
        }

        sample_count = (half_size * 2 + 1) * (half_size * 2 + 1);
        vec4 blur2 = color_sum / sample_count;




        FragColor = 0.25 * blur2 + 0.5 * blur + texture(screen, TexCoords);
    }
    else if (mode == 5) // pixelization
    {
        int pixel_wide = 12;

        int pixel_x = int(floor(gl_FragCoord.x / pixel_wide)) * pixel_wide;
        int pixel_y = int(floor(gl_FragCoord.y / pixel_wide)) * pixel_wide;



        vec4 color_sum = vec4(0);

        for(int i = 0; i < pixel_wide; i++)
        {
            for (int j = 0; j < pixel_wide; j++)
            {
                ivec2 coord = ivec2(pixel_x, pixel_y) + ivec2(i, j);
                color_sum += texelFetch(screen, coord, 0);
            }
        }

        int sample_count = pixel_wide * pixel_wide;
        vec4 blur = color_sum / sample_count;

        FragColor = blur;
    }
    else if (mode == 6) // sin wave
    {
        FragColor = texture(screen, vec2(TexCoords.x + 0.05 * sin(TexCoords.y * 2 * 3.1416 + time), TexCoords.y));
    }
}