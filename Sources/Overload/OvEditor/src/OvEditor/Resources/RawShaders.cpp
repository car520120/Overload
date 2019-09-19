/**
* @project: Overload
* @author: Overload Tech.
* @restrictions: This software may not be resold, redistributed or otherwise conveyed to a third party.
*/

#include "OvEditor/Resources/RawShaders.h"

std::pair<std::string, std::string> OvEditor::Resources::RawShaders::GetGrid()
{
	std::pair<std::string, std::string> source;

	source.first = R"(
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 1) in vec2 geo_TexCoords;
layout (location = 2) in vec3 geo_Normal;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

out VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
} vs_out;

void main()
{
    vs_out.FragPos      = vec3(ubo_Model * vec4(geo_Pos, 1.0));
    vs_out.TexCoords    = geo_TexCoords;

    gl_Position = ubo_Projection * ubo_View * vec4(vs_out.FragPos, 1.0);
}
)";

	source.second = R"(
#version 460 core

out vec4 FRAGMENT_COLOR;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

in VS_OUT
{
    vec3 FragPos;
    vec2 TexCoords;
} fs_in;

uniform vec3 u_Color;

const float divisions = 1000.0f;
const float lineWidth = 1.0f;
const float step = 100.0f;
const float subdivisions = 4.0f;

vec4 Grid(float p_divisions)
{
    // Pick a coordinate to visualize in a grid
    vec2 coord = fs_in.TexCoords.xy * p_divisions;

    // Compute anti-aliased world-space grid lines
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    float lineResult = lineWidth - min(line, lineWidth);

    // Just visualize the grid lines directly
    vec3 fakeViewPos = ubo_ViewPos;
    fakeViewPos.y = 0.0f;
    return vec4(vec3(lineResult) * u_Color, 0.05 * lineResult);
}

void main()
{
	float divs;

	divs = divisions / pow(2, round((abs(ubo_ViewPos.y) - step / subdivisions) / step));
	vec4 grid1 = Grid(divs) + Grid(divs / subdivisions);

	divs = divisions / pow(2, round((abs(ubo_ViewPos.y + 50) - step / subdivisions) / step));
	vec4 grid2 = Grid(divs) + Grid(divs / subdivisions);

	float alpha = mod(abs(ubo_ViewPos.y), step);
	alpha = 0.0;

    FRAGMENT_COLOR = mix(grid1, grid2, alpha);

    vec3 pseudoViewPos = vec3(ubo_ViewPos.x, fs_in.FragPos.y, ubo_ViewPos.z);
    float distanceToCamera = max(distance(fs_in.FragPos, pseudoViewPos) - abs(ubo_ViewPos.y), 0);
    
    float alphaDecreaseDistance = 20.0f;
    float decreaseDistance = 30.0f;
    if (distanceToCamera > alphaDecreaseDistance)
    {
        float normalizedDistanceToCamera = clamp(distanceToCamera - alphaDecreaseDistance, 0.0f, decreaseDistance) / decreaseDistance;
        FRAGMENT_COLOR.a *= clamp(1.0f - normalizedDistanceToCamera, 0.0f, 1.0f);
    }
}
)";

	return source;
}

std::pair<std::string, std::string> OvEditor::Resources::RawShaders::GetGizmo()
{
	std::pair<std::string, std::string> source;

	source.first = R"(
#version 460 core

layout (location = 0) in vec3 geo_Pos;
layout (location = 2) in vec3 geo_Normal;
layout (location = 1) in vec2 geo_TexCoords;

layout (std140) uniform EngineUBO
{
    mat4    ubo_Model;
    mat4    ubo_View;
    mat4    ubo_Projection;
    vec3    ubo_ViewPos;
    float   ubo_Time;
};

out VS_OUT
{
    flat vec3   Color;
} vs_out;

uniform bool u_IsBall;
uniform bool u_IsPickable;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4
    (
        oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
        oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
        oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
        0.0,                                0.0,                                0.0,                                1.0
    );
}

void main()
{
    mat4 instanceModel = ubo_Model;

    if (gl_InstanceID == 1) 
        instanceModel *= rotationMatrix(vec3(0, 1, 0), radians(-90)); /* X axis */
    else if (gl_InstanceID == 2) 
        instanceModel *= rotationMatrix(vec3(1, 0, 0), radians(90)); /* Y axis */

    float distanceToCamera = distance(ubo_ViewPos, instanceModel[3].xyz);

	vec3 pos = geo_Pos;

    vec3 fragPos = vec3(instanceModel * vec4(pos * distanceToCamera * 0.1f, 1.0));

	if (u_IsPickable)
	{
		int blueComponent = 0;

		if (gl_InstanceID == 1)
			blueComponent = 252;

		if (gl_InstanceID == 2)
			blueComponent = 253;

		if (gl_InstanceID == 0)
			blueComponent = 254;

		vs_out.Color = vec3(1.0f, 1.0f, blueComponent / 255.0f);
	}
	else
	{
		if (u_IsBall)
		{
			vs_out.Color = vec3(1.0f);
		}
		else
		{
			float red	= float(gl_InstanceID == 1); // X
			float green = float(gl_InstanceID == 2); // Y
			float blue	= float(gl_InstanceID == 0); // Z

			vs_out.Color = vec3(red, green, blue);
		}
	}

    gl_Position = ubo_Projection * ubo_View * vec4(fragPos, 1.0);
}
)";

	source.second = R"(
#version 460 core

out vec4 FRAGMENT_COLOR;

in VS_OUT
{
    vec3 Color;
} fs_in;

uniform bool u_IsPickable;

void main()
{
	FRAGMENT_COLOR = vec4(fs_in.Color, 1.0f);
})";

	return source;
}