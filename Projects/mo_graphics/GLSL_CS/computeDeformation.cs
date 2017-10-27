
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: computeDeformations.cs
//
//	alternative solution or GPU skinning and blendshapes
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

uniform int		numberOfClusters;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TYPES AND DATA BUFFERS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BlendShapeVertex
{
	vec4	pos;
	vec4	nor;
};

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
	BlendShapeVertex	vertices[];
} blendshapeBuffer;

// output
layout (std140, binding = 2) buffer OutputNormals
{
	vec4	normals[];
} outputNormals;

layout (std140, binding = 3) buffer OutputPositions
{
	vec4	positions[];
} outputPositions;

// for each vertex data, limit 4 bones per vertex 
//	almost constant data !

struct ClusterVertex
{
	vec4	links;
	vec4	weights;
};

layout (std140, binding = 4) buffer ClusterInfo
{
	ClusterVertex	vertices[];
} clusterInfo;

// update each frame for the binded skeleton
layout (std430, binding = 5) buffer JointsInfo
{
	mat4	tm[];		// computed transform of each link (cluster)
} jointsInfo;

////////////////////////////////////////////////////////
//

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
	
	// Read source normal
	vec4 nor = blendshapeBuffer.vertices[flattened_id].nor;
	vec4 pos = blendshapeBuffer.vertices[flattened_id].pos;
	//vec4 tan = tangentBuffer.tangents[flattened_id];
	
	// default deform order: blendshapes, then clusters
	
	// blend with active blendshapes
	
	for (int i=0; i<numberOfBlendshapes; ++i)
	{
		float weight = weightsBuffer.weights[i].x;
		int index = (i+1) * numberOfVertices + int(flattened_id);
		
		vec4 blendshapeNormal = blendshapeBuffer.vertices[index].nor;
		vec4 blendshapePos = blendshapeBuffer.vertices[index].pos;
		//vec4 blendshapeTangent = blendshapeBuffer.tangents[index];
		
		nor = mix(nor, blendshapeNormal, weight);
		pos = mix(pos, blendshapePos, weight);
		
		//nor = blendshapeNormal;
		//tan = mix(tan, blendshapeTangent, weights.w);
	}
	
	// deform with clusters
	
	if (numberOfClusters > 0)
	{
		vec4 weights = clusterInfo.vertices[flattened_id].weights;
		vec4 links = clusterInfo.vertices[flattened_id].links;
		
		
		pos.w = 1.0;
		/*
		float weight1 = weights.x;
		//mat4 tm = mat4(1.0);
		pos.w = 1.0;
		outPos = vec4(0.0, 0.0, 0.0, 0.0);
		
		float weight2 = weights.y;
		mat4 influence = jointsInfo.tm[int(links.x)];
		pos1 = influence * pos;
		
		//outPos += pos1;
		*/
		
		int id1 = int(links.x);
		//if (clamp(links.x, 0.0, 1.0) > 0.0) id1 = 1;
		int id2 = int(links.y);
		//if (clamp(links.y, 0.0, 1.0) > 0.0) id2 = 1;
		
		float w1 =  clamp(weights.x, 0.0, 1.0);
		float w2 =  clamp(weights.y, 0.0, 1.0);
		
		//w2 = 1.0 - w1;
		
		float sum = w1 + w2;
		
		if (sum == 1.0)
		{
		
		mat4 inf1 = jointsInfo.tm[id1];
		vec4 pos1 = inf1 * pos;
		
		mat4 inf2 = jointsInfo.tm[id2];
		vec4 pos2 = inf2 * pos;
		
		pos = pos1 * w1 + pos2 * w2;
		pos.w = 1.0;
		/*
		vec4 nor1 = transpose(inverse(inf1)) * nor;
		vec4 nor2 = transpose(inverse(inf2)) * nor;
		
		nor1 = normalize(nor1);
		nor1.w = 0.0;
		nor2 = normalize(nor2);
		nor2.w = 0.0;
		
		nor = nor1 * w1 + nor2 * w2;
		*/
		}
		/*
		//outPos += weights.y * pos2;
		if (weight1 > weight2)
			pos = pos1;
		else
			pos = pos2;*/
		//pos = pos2;
		//pos = pos1 + pos2;
		/*
		if (weights.x >= 0.0)
		{
			
			int id = 0;
			if (links.x > 0.0) id = 1;
			
			//int id = int(links.x);
			mat4 influence = jointsInfo.tm[id];
			pos1 = influence * pos;
			
			//tm = influence * weights.x;
			//pos = tm * vec4(pos.x, pos.y, pos.z, 1.0);
			outPos = pos1;
		}
		if (weights.y >= 0.0)
		{
			
			int id = 0;
			if (links.y > 0.0) id = 1;
			
			//int id = int(links.y);
			mat4 influence = jointsInfo.tm[id];
			pos2 = influence * pos;
			
			//tm += influence * weights.y;
			//pos = pos2;
			
			outPos = pos2;
		}
		
		
		pos = outPos;
		*/
		//pos = mix(pos1, pos2, weights.x); // pos1 * weights.x + pos2 * weights.y;
		//pos = tm * vec4(pos.x, pos.y, pos.z, 1.0);
		//nor = tm * vec4(nor.x, nor.y, nor.z, 1.0);
		//nor = normalize(nor);
	}
	
	
	outputNormals.normals[flattened_id] = nor;
	outputPositions.positions[flattened_id] = pos;
}