#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoord;

out VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;     
    vec2 texcoord;
    vec4 FragPosLightSpace;
} vs_out;     

const float PI = 3.1415926f;

uniform vec3 light_pos;

uniform vec3 wind_dir = vec3(1.0f, 0.0f, 1.0f);
uniform float wind_force = 5.0f;
uniform float wind_wave_effect_range = 5.0f;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
uniform mat4 lightSpaceMatrix;

uniform float time;

vec3 oppositePoint(vec3 origin, vec3 origin_dir, float length, vec3 dir)
{
	vec3 nd = normalize(dir);
	float angle = acos(dot(normalize(origin_dir), nd));
	if (angle < PI / 4)
	{
		return origin;
	}
	else if (angle > PI * 3 / 4)
	{
		return origin + length * origin_dir;
	}
	else
	{
		vec3 left = cross(origin_dir, vec3(0.0f, 1.0f, 0.0f));
		angle = acos(dot(normalize(left), nd));
		if (angle > PI / 2)
			return origin + length * vec3(0.0f, 0.0f, origin_dir.z);
		else
			return origin + length * vec3(origin_dir.x, 0.0f, 0.0f);
	}
}

float distanceToLine(vec2 p1, vec2 p2, vec2 test)
{
	vec2 v1 = p2 - p1;
	vec2 v2 = p1 - test;
	vec2 v3 = vec2(v1.y, -v1.x);
	return abs(dot(v2, normalize(v3)));
}

void main()
{
    int id = gl_InstanceID;
    float x = mod(id, 9), z = id / 9, scale = 16;
	float length = scale * 9;
	float period = sin(radians(time));
    vec3 offset = vec3(scale * x, 0, scale * z);
	vec3 offset_pos = aPos + offset;
	vec3 animation_offset = vec3(period) * wind_dir * wind_force * offset_pos.y * offset_pos.y;
	vec3 wave_front = oppositePoint(aPos, vec3(1.0f, 0.0f, 1.0f), length, wind_dir) + mod(time, length) * wind_dir;
	vec3 wave_vec = cross(normalize(wind_dir), vec3(0.0f, 1.0f, 0.0f));
	vec3 wave_pointB = wave_front + wave_vec;
	float dist_to_wave_front = distanceToLine(vec2(wave_front.x, wave_front.z), vec2(wave_pointB.x, wave_pointB.z), vec2(offset_pos.x, offset_pos.z));
	vec3 wave_offset = vec3(0.0f, 0.0f, 0.0f);
	if (dist_to_wave_front < wind_wave_effect_range)
		wave_offset = min(100.0f, wind_force * wind_force) * (wind_wave_effect_range - dist_to_wave_front) * wind_dir * offset_pos.y * offset_pos.y;
    vec3 pos = offset_pos + 0.001f * animation_offset + 0.0001f * wave_offset;

    vec4 P = view * model * vec4(pos, 1.0);
    vs_out.N = mat3(transpose(inverse(view * model))) * anormal;
    vs_out.L = (view * vec4(light_pos, 1.0)).xyz - P.xyz;
    vs_out.V = -P.xyz;
    gl_Position = project * P;

    vs_out.texcoord = atexcoord;

    vs_out.FragPosLightSpace = lightSpaceMatrix * model * vec4(pos, 1.0);
} 