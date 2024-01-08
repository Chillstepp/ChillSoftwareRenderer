cmake_minimum_required(VERSION 3.22)
project(TinyRenderLessonCode)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyRenderLessonCode main.cpp TGAImage.cpp TGAImage.h Model.cpp Model.h Math.cpp Math.h Render.cpp Render.h Shader/IShader.cpp Shader/IShader.h Window.cpp Window.h Scene.cpp Scene.h Object.cpp Object.h Camera.cpp Camera.h Shader/PCSSShader.cpp Shader/PCSSShader.h Shader/PhongShader.cpp Shader/PhongShader.h Shader/GBuffer.cpp Shader/GBuffer.h)
