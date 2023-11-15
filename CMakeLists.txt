cmake_minimum_required(VERSION 3.22)
project(TinyRenderLessonCode)

set(CMAKE_CXX_STANDARD 20)

add_executable(TinyRenderLessonCode main.cpp TGAImage.cpp TGAImage.h Model.cpp Model.h Math.cpp Math.h Render.cpp Render.h)
