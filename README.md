# opengl_core_learning
Each branch has a project developed for college. Every project was built on top of the main branch using QT libraries (5.15), OpenGL core profile with GLSL 330.<br>
- **main:** implements a base project with a scene using Blinn-Phong reflection model, texture mapping and ArcBall.<br>
- **normal_mapping:** implements two scenes using normal maps to simulate bumps (theres a golfball and a stone floor scene).<br>
- **shadow_mapping:** implements a Shadow Mapping algorithm using a simple PCF filter to make shadows smoother.<br>
- **deferred_shading:** implements the same scene from shadow_mapping but uses two steps to create the scene, first it renders the geometry and saves the data to a G-buffer, using the G-buffer it calculates the lighting on a second (deferred) step.<br>
- **ambient_occlusion:** TBA
