//
// Created by why on 2024/1/8.
//

#ifndef CHILLSOFTWARERENDERER_GBUFFER_H
#define CHILLSOFTWARERENDERER_GBUFFER_H

#include "../Math.h"
#include <map>

template<typename T>
class Buffer
{
 public:
	int Width;
	int Height;
	T* Data;

 public:
	Buffer(Vec2i Size)
	{
		Width = Size.width;
		Height = Size.height;
		Data = new T[Width * Height];
	}
	~Buffer()
	{
		delete[] Data;
		Data = nullptr;
	}

	T* operator [](int k)
	{
		return &Data[k * Width];
	}

};

class GBuffer {
private:
	GBuffer() {};
	~GBuffer() {};
	GBuffer(const GBuffer&) {};
	GBuffer& operator=(const GBuffer&){};

	std::map<std::string, void*>Buffers;

public:
	static GBuffer Get()
	{
		static GBuffer Instance;
		return Instance;
	}
	template<typename BufferDataType>
	Buffer<BufferDataType>* GetBuffer(const std::string& BufferName) const
	{
		auto BufferFound = Buffers.find(BufferName);
		if(BufferFound == Buffers.end()) return nullptr;
		return BufferFound;
	}
};


#endif //CHILLSOFTWARERENDERER_GBUFFER_H
