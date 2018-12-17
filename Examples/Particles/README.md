# Particles

N-body simulation with 16 000 particles. Each particle gets a speed outwards of in the beginning and after that they move according to gravitational-like forces.

Simulation is done in two compute steps:
1. Calculate new speed for each particle
2. Update the position of each particle according to speed

Finally the particles are rendered with `VK_PRIMITIVE_TOPOLOGY_POINT_LIST` so that the newly calculated positions serve as the vertex input. The color of a particle changes according to speed: fast moving particles are blue and slow moving particles are turquoise.

![particles](particles.png?raw=true "particles")

![particles](particles.gif?raw=true "particles")
