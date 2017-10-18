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

void main() {
	int N = int(gl_NumWorkGroups.x*gl_WorkGroupSize.x);
	int id = int(gl_GlobalInvocationID);

	vec2 position = p[id].position;
	vec2 acceleration = vec2(0, 0);
	float mass = p[id].mass;

	// accelerate
	for (int j = 0; j < N; ++j) {
		if (j == id) {
			continue;
		}
		vec2 other = p[j].position;
		vec2 vDist = position - other;
		float dist = length(vDist);
		if (dist < TRESHOLD) {
			continue;
		}
		vec2 uDist = normalize(vDist);
		acceleration -= G * mass * p[j].mass / (dist * dist) * uDist;
	}
	p[id].acceleration = acceleration;
}
