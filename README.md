# GLoom
This is a minimal, blank OpenGL project that sets up GLM, GLAD and SDL and draws a square to the screen, which serves as a starting point for OpenGL projects.

![OpenGL Square](square.png)

## Libraries
External libraries which are included in this project:
- **SDL2**: Handles input, windows and OpenGL context 
- **GLAD**: Loading OpenGL functions
- **GLM**: For various mathematical operations

## Compile on Linux with
```bash
g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl
```
