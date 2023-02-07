# ARI-S
*ARI-S* (pronounced "ari-us") is a real-time engine developed in OpenGL that employs an entity component system to render various scenes. This project was originally intended as means for learning and showcasing advanced rendering techniques, but the scope has expanded to include an interactive editor to dynamically create and load scenes.

# Features
- Model loading
- Texture loading/mapping
- Lighting
- Environment mapping
- Deferred rendering
- Scene saving/loading
- Entity + component creation & editing

# Dependencies

All libraries are included in the `Libraries/` directory.

- EnTT
- GLFW
- GLAD
- GLM
- ImGui
- stb
- tinygltf
- tinyobjloader
- YAML-CPP

# Credits
- All relevant classes I took at DigiPen Institute of Technology regarding the techniques used in this renderer. Classes include (but are not limited to):
  - CS300/CS350
  - CS562
- [The Cherno's Game Engine series](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT) for the editor framework and EnTT library integration
- [Michael Grieco's C++ OpenGL tutorial](https://www.youtube.com/playlist?list=PLysLvOneEETPlOI_PI4mJnocqIpr2cSHS) for OpenGL file structure
