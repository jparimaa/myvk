# Specialization constant

Specialization constant can be used to change a constant value in a shader. This makes possible to use a single shader for different purposes (e.g. enable / disable effect) so there is no need to copy the whole shader. This is sometimes called as uber shader; a single shader that does many things. Two pipelines are created and they are the same except they have different specialization values.

![specialization_constant.png](specialization_constant.png.png?raw=true "specialization_constant.png")
