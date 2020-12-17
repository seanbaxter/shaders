#include <mgpu/kernel_mergesort.hxx>
#include <mgpu/app.hxx>

using mgpu::gl_buffer_t;

// TODO: Only put simulation params here.

// simulation parameters
struct SimParams {
  vec3  colliderPos;
  float colliderRadius;

  vec3  gravity;
  float globalDamping;
  float particleRadius;

  ivec3 gridSize;
  int   numCells;
  vec3  worldOrigin;
  vec3  cellSize;

  int   numBodies;

  float deltaTime;
  float spring;
  float damping;
  float shear;
  float attraction;
  float boundaryDamping;
};

inline vec3 collide_spheres(vec3 posA, vec3 posB, vec3 velA, vec3 velB,
  float radiusA, float radiusB, const SimParams& params) {

  vec3 relPos = posB - posA;
  float dist = length(relPos);
  float collideDist = radiusA + radiusB;

  vec3 force { };
  if(dist < collideDist) {
    vec3 norm = relPos / dist;

    // relative velocity.
    vec3 relVel = velB - velA;

    // relative tangential velocity.
    vec3 tanVel = relVel - dot(relVel, relVel) * norm;

    // spring force.
    force = -params.spring * (collideDist - dist);
    
    // dashpot (damping) fgorce
    force += params.damping * relVel;

    // tangential shear force
    force += params.shear * tanVel;

    // attraction
    force += params.attraction * relPos;
  }

  return force;
}

inline ivec3 calcGridPos(vec3 p, const SimParams& params) {
  return (ivec3)floor((p - params.worldOrigin) / params.cellSize);
}
inline int hashGridPos(ivec3 p, const SimParams& params) {
  p &= params.gridSize - 1;
  return p.x + params.gridSize.x * (p.y + params.gridSize.y * p.z);
}

struct system_t {
  gl_buffer_t<vec4[]> positions;
  gl_buffer_t<vec4[]> velocities;

  gl_buffer_t<vec4[]> positions_out;
  gl_buffer_t<vec4[]> velocities_out;

  // Hash each particle to a cell ID.
  gl_buffer_t<int[]> cell_hash;

  // When sorting by cell hash, generate these particle indices to help
  // reorder the buffers.
  gl_buffer_t<int[]> gather_indices;

  // Keep the min and max particle index for each cell.
  gl_buffer_t<ivec2[]> cell_ranges;

  mgpu::mergesort_pipeline_t<int, int> sort_pipeline;

  SimParams params;
  int num_particles;

  void advance_frame();
  void collide();
  void integrate();
  void render();
  void sort_particles();
};

// Park the simulation parameters at ubo 1 and keep it there throughout the
// frame. UBO 0 is reserved for gl_transform.
[[using spirv: uniform, binding(1)]]
SimParams sim_params_ubo;

void system_t::advance_frame() {
  int num_particles = params.numBodies;

  cell_hash.resize(num_particles);
  gather_indices.resize(num_particles);
  cell_ranges.resize(num_particles);

  // First perform collision to accumulate forces on each particles.
  // This is the physics part.
  collide();

  // Advance the velocities and positions.
  integrate();

  // Draw the pretty picture.
  render();

  // Reorder the particles for the next frame.
  sort_particles();
}

void system_t::collide() {
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto cell_ranges_in = cell_ranges.bind_ssbo<2>();

  auto vel_out = velocities.bind_ssbo<3>();

  mgpu::gl_transform([=](int index) {
    vec3 f { };
    float r = sim_params_ubo.particleRadius;

    // Read particle data.
    vec3 pos = pos_in[index].xyz;
    vec3 vel = vel_in[index].xyz;

    // Hash to the grid.
    ivec3 gridPos = calcGridPos(pos, sim_params_ubo);

    // Examine neighbouring cells.
    for(int z = -1; z <= 1; ++z) {
      for(int y = -1; y <= 1; ++y) {
        for(int x = -1; x <= 1; ++x) {
          int hash = hashGridPos(gridPos + ivec3(x, y, z), sim_params_ubo);

          // Get the range of particles for this cell.
          ivec2 range = cell_ranges_in[hash];

          for(int i = range.x; i < range.y; ++i) {
            // Visit each particle in the cell.
            vec3 pos2 = pos_in[i].xyz;
            vec3 vel2 = vel_in[i].xyz;

            // Compute the force on the left particle.
            f += collide_spheres(pos, pos2, vel, vel2, r, r, sim_params_ubo);
          }
        }
      }
    }

    // Collide with the cursor sphere.
    f += collide_spheres(pos, sim_params_ubo.colliderPos, vel, vec3(), r, 
      sim_params_ubo.colliderRadius, sim_params_ubo);

    // Integrate the velocity by the new acceleration and write out.
    vel += f * sim_params_ubo.deltaTime;
    vel_out[index] = vec4(vel, 0);

  }, num_particles);
}

void system_t::integrate() {
  auto pos_data = positions.bind_ssbo<0>();
  auto vel_data = velocities.bind_ssbo<1>();

  mgpu::gl_transform([=](int index) {
    SimParams params = sim_params_ubo;

    // Load the particle.
    vec4 pos4 = pos_data[index];
    vec4 vel4 = vel_data[index];

    vec3 pos = pos4.xyz;
    vec3 vel = vel4.xyz;

    // Apply gravity and damping.
    vel += params.gravity;
    vel *= params.globalDamping;

    // Integrate the position.
    pos += vel * params.deltaTime;

    // Collide with the cube sides.
    bvec3 clip_max = pos > 1 - params.particleRadius;
    pos = clip_max ? 1 - params.particleRadius : pos;
    vel *= clip_max ? params.boundaryDamping : 1;

    bvec3 clip_min = pos < -1 + params.particleRadius;
    pos = clip_min ? -1 + params.particleRadius : pos;
    vel *= clip_max ? params.boundaryDamping : 1;

    // Store updated terms.
    pos_data[index] = vec4(pos, pos4.w);
    vel_data[index] = vec4(vel, vel4.w);

  }, num_particles);
}

[[spirv::vert]]
void vertex_shader() {
//  vec3 
}

[[spirv::frag]]
void fragment_shader() {
  constexpr vec3 light_dir(.577, .577, .577);

  // Calculate normal from texture coordinates.

}


void system_t::render() {

}


      // vertex shader
      const char *vertexShader = STRINGIFY(
                                     uniform float pointRadius;  // point size in world space
                                     uniform float pointScale;   // scale to calculate size in pixels
                                     uniform float densityScale;
                                     uniform float densityOffset;
                                     void main()
      {
          // calculate window-space point size
          vec3 posEye = vec3(gl_ModelViewMatrix * vec4(gl_Vertex.xyz, 1.0));
          float dist = length(posEye);
          gl_PointSize = pointRadius * (pointScale / dist);

          gl_TexCoord[0] = gl_MultiTexCoord0;
          gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz, 1.0);

          gl_FrontColor = gl_Color;
      }
                                 );

      // pixel shader for rendering points as shaded spheres
      const char *spherePixelShader = STRINGIFY(
                                          void main()
      {
          const vec3 lightDir = vec3(0.577, 0.577, 0.577);

          // calculate normal from texture coordinates
          vec3 N;
          N.xy = gl_TexCoord[0].xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
          float mag = dot(N.xy, N.xy);

          if (mag > 1.0) discard;   // kill pixels outside circle

          N.z = sqrt(1.0-mag);

          // calculate lighting
          float diffuse = max(0.0, dot(lightDir, N));

          gl_FragColor = gl_Color * diffuse;
      }
                                      );




void system_t::sort_particles() {
  int num_particles = params.numBodies;

  // Hash particles into cells.
  auto pos_data = positions.bind_ssbo<0>();
  auto hash_data = cell_hash.bind_ssbo<1>();

  // 1. Quantize the particles into cells. Hash the cell coordinates
  //    into an integer.
  mgpu::gl_transform([=](int index) {
    vec3 pos = pos_data[index].xyz;
    ivec3 gridPos = calcGridPos(pos, sim_params_ubo);
    int hash = hashGridPos(gridPos, sim_params_ubo);

    hash_data[index] = hash;

  }, num_particles);

  // 2. Sort the particles by their hash. The value of the sort is the index
  //    of the particle.
  sort_pipeline.sort_keys_indices(cell_hash, gather_indices, num_particles);

  // 3. Reorder the particles according to their gather indices.

  // Clear the ranges array because we'll never visit cells with no
  // particles.  
  ivec2 zero { };
  glClearNamedBufferSubData(cell_ranges, GL_RG32I, 0, 
    num_particles * sizeof(ivec2), GL_RG, GL_INT, &zero);

  // Reorder the particles and fill the ranges.
  auto pos_in = positions.bind_ssbo<0>();
  auto vel_in = velocities.bind_ssbo<1>();
  auto hash_in = cell_hash.bind_ssbo<2>();
  auto gather_in = gather_indices.bind_ssbo<3>();
  auto pos_out = positions_out.bind_ssbo<4>();
  auto vel_out = positions_out.bind_ssbo<5>();
  auto cell_ranges_out = cell_ranges.bind_ssbo<6>();

  mgpu::gl_transform([=](int index) {
    // Load the gather and hash values.
    int gather = gather_in[index];
    int hash = hash_in[index];
    int hash_prev = index ? hash_in[index - 1] : -1;

    // Load the particle data.
    vec4 pos = pos_in[gather];
    vec4 vel = vel_in[gather];

    // Write the cell ranges.
    if(hash_prev < hash) {
      if(index) cell_ranges_out[hash_prev].y = index;
      cell_ranges_out[hash].x = index;
    }

    if(index == sim_params_ubo.numBodies - 1)
      cell_ranges_out[hash].y = sim_params_ubo.numBodies;

    // Write the particles to memory.
    pos_out[index] = pos;
    vel_out[index] = vel;

  }, num_particles);
}

int main() {
  mgpu::app_t app("particles");
}