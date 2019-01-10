# Clustered

Clustered forward rendering (also known as clustered forward+) is a rendering technique where the frustum is divided into cells. Each cell has information which lights affect it. When the mesh is rendered the respective cell is located and the lighting is calculated for each light. The benefit of this compared to deferred rendering is that there are less texture accesses which makes it less bandwidth heavy. The light grid, i.e. which light belongs to which cell, is calculated first in a separate compute pass.

More information about clustered or tiled rendering

Practical Clustered Shading by Emil Persson
http://www.humus.name/Articles/PracticalClusteredShading.pdf

Forward+ (EUROGRAPHICS 2012) by Takahiro Harada
https://www.slideshare.net/takahiroharada/forward-34779335

Forward vs Deferred vs Forward+ Rendering with DirectX 11 by Jeremiah van Oosten
https://www.3dgep.com/forward-plus/

![clustered](clustered.png?raw=true "clustered")
