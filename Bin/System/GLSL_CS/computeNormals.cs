
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: computeNormals.cs
//
//	calculate model normal and tangent with a given list of blendshapes
//
//	Author Sergey Solokhin (Neill3d)
//
//	GitHub page - https://github.com/Neill3d/MoPlugs
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#version 430

layout (local_size_x = 1024, local_size_y = 1) in;

uniform int		numberOfBlendshapes;
uniform int		numberOfVertices;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
// INPUT from mobu mesh data
layout (std430, binding = 0) readonly buffer NormalBuffer
{
	vec4	normals[];
} normalBuffer;
*/
// stores x-blendshape index offset, w-blendshape weight
layout (std430, binding = 0) buffer WeightsBuffer
{
	vec4	weights[];
} weightsBuffer;

// stores all blendshape normals and tangents (static shapes)
layout (std430, binding = 1) buffer BlendShapeBuffer
{
	vec4	normals[];
} blendshapeBuffer;

// output
layout (std140, binding = 2) buffer OutputBuffer
{
	vec4	normals[];
} outputBuffer;

#ifdef BLEND_POSITIONS
layout (std430, binding = 3) buffer BlendShapePositions
{
	vec4	positions[];
} blendshapePositions;

layout (std140, binding = 4) buffer OutputPositions
{
	vec4	positions[];
} outputPositions;
#endif

uint get_invocation()
{
   //uint work_group = gl_WorkGroupID.x * gl_NumWorkGroups.y * gl_NumWorkGroups.z + gl_WorkGroupID.y * gl_NumWorkGroups.z + gl_WorkGroupID.z;
   //return work_group * gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z + gl_LocalInvocationIndex;

   // uint work_group = gl_WorkGroupID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + gl_WorkGroupID.x * gl_WorkGroupSize.x + gl_LocalInvocationIndex;
   uint work_group = gl_GlobalInvocationID.y * (gl_NumWorkGroups.x * gl_WorkGroupSize.x) + gl_GlobalInvocationID.x;
   return work_group;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void main()
{
	// particle id in the array
	uint flattened_id = get_invocation();
	
	// ?! skip unused part of the array
	if (flattened_id >= numberOfVertices)
		return;
	
#ifdef BLEND_POSITIONS
	// Read source normal
	vec4 nor = blendshapeBuffer.normals[flattened_id];
	vec4 pos = blendshapePositions.positions[flattened_id];
	//vec4 tan = tangentBuffer.tangents[flattened_id];
	
	// blend with active blendshapes
	
	for (int i=0; i<numberOfBlendshapes; ++i)
	{
		float weight = weightsBuffer.weights[i].x;
		int index = (i+1) * numberOfVertices + int(flattened_id);
		
		vec4 blendshapeNormal = blendshapeBuffer.normals[index];
		vec4 blendshapePos = blendshapePositions.positions[index];
		//vec4 blendshapeTangent = blendshapeBuffer.tangents[index];
		
		nor = mix(nor, blendshapeNormal, weight);
		pos = mix(pos, blendshapePos, weight);
		
		//nor = blendshapeNormal;
		//tan = mix(tan, blendshapeTangent, weights.w);
	}
	
	outputBuffer.normals[flattened_id] = nor;
	outputPositions.positions[flattened_id] = pos;
	
#else
	// Read source normal
	vec4 nor = blendshapeBuffer.normals[flattened_id];
	//vec4 tan = tangentBuffer.tangents[flattened_id];
	
	// blend with active blendshapes
	
	for (int i=0; i<numberOfBlendshapes; ++i)
	{
		float weight = weightsBuffer.weights[i].x;
		int index = (i+1) * numberOfVertices + int(flattened_id);
		
		vec4 blendshapeNormal = blendshapeBuffer.normals[index];
		//vec4 blendshapeTangent = blendshapeBuffer.tangents[index];
		
		nor = mix(nor, blendshapeNormal, weight);
		
		//nor = blendshapeNormal;
		//tan = mix(tan, blendshapeTangent, weights.w);
	}
	
	outputBuffer.normals[flattened_id] = nor;
	//outputBuffer.tangents[flattened_id] = tan;
#endif
}