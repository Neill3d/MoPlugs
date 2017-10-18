#version 440

struct Particle {
	vec2 position;
	vec2 acceleration;
	float mass;
};

layout(local_size_x=256) in;

layout(std430, binding=0) buffer particles {
	Particle p[];
};

const float G        = 6.67384e-11;
const float DBL_MIN  = 2.2250738585072014e-308;
const float TRESHOLD = 2e-3;

shared Particle tmp[gl_WorkGroupSize.x];

void main() {
	int N = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
	int id = int(gl_GlobalInvocationID);

	vec2 position = p[id].position;
	vec2 acceleration = vec2(0, 0);
	float mass = p[id].mass;

	for (int tile = 0; tile < N; tile += int(gl_WorkGroupSize.x)) {
		tmp[gl_LocalInvocationIndex] = p[tile + int(gl_LocalInvocationIndex)];
		groupMemoryBarrier();
		barrier();
		for(int i = 0; i < gl_WorkGroupSize.x; i++) {
			if (i == id) {
				continue;
			}
			vec2 other = tmp[i].position;
			vec2 vDist = position - other;
			float dist = length(vDist);
			if (dist < TRESHOLD) {
				continue;
			}
			vec2 uDist = normalize(vDist);
			acceleration -= G * mass * tmp[i].mass / (dist * dist) * uDist;
		}
		groupMemoryBarrier();
		barrier();
	}
	p[id].acceleration = acceleration;
}
