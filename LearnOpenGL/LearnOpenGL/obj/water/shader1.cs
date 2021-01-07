#version 430 core
struct WaterColumn
{
    float height;                                                                  
    float flow;                                                                    
};

layout(std430, binding = 0) buffer WaterGrid
{
    WaterColumn columns[];
};

#define GRID_SIZE 18
#define ROW_SIZE (GRID_SIZE * gl_NumWorkGroups.x)

layout(local_size_x = GRID_SIZE, local_size_y = GRID_SIZE, local_size_z = 1) in;
uniform uvec2 dropCoord;                                      

const float RADIUS = 0.03;
const float PI = 3.141592653589793;
const float STRENGTH = 5.0;

void main(void)
{
    float row_size_1 = float(ROW_SIZE) - 1.0;
    vec2 pos = vec2(gl_GlobalInvocationID.xy) / row_size_1;
    vec2 center = vec2(dropCoord) / row_size_1;
    float dist = length(center - pos);
    float drop = max(0, 1.0 - dist / RADIUS);
    drop = 0.5 - cos(drop * PI) * 0.5;

    uint globalIdx = gl_GlobalInvocationID.y * ROW_SIZE + gl_GlobalInvocationID.x;
    columns[globalIdx].height += drop * STRENGTH;
}