# Screen space reflection

Screen-space reflections (SSR) require a G-buffer that has at least position, normals and color in view space. With the G-buffer it's possible to use ray marching to find the correct sample for a reflective surface. [Ray marching](https://computergraphics.stackexchange.com/questions/161/what-is-ray-marching-is-sphere-tracing-the-same-thing) means taking one step at a time and checking if the depth is greater than the respective value in the G-buffer. If the depth is greater than the value in the G-buffer, then the point where the ray reflects is found. The ray for ray marching is the reflection vector of the view position and view normal. Then at each step the ray needs to be projected (i.e. multiplied with projection matrix) and transformed to UV-space so the sample can be taken.

There are several draw backs in screen-space reflections, mainly because there's no more information about the scene than what is currently visible in the screen. This means that rays that go out of screen, behind an object, towards the viewer and so on will give incorrect results.

For more information about SSR can be found at 
- http://roar11.com/2015/07/screen-space-glossy-reflections/
- http://www.cse.chalmers.se/edu/year/2017/course/TDA361/Advanced%20Computer%20Graphics/Screen-space%20reflections.pdf
- http://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
- https://virtexedgedesign.com/2018/06/24/shader-series-basic-screen-space-reflections/

![reflection](reflection.png?raw=true "reflection")