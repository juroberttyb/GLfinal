#version 330 core
out float FragColor;
  
in vec2 TexCoords;
  
uniform sampler2D scene;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(scene, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(scene, TexCoords + offset).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
} 