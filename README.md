# SoftwareRender

## Show Case

- Normal Interpolation

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145044441.png" alt="image-20231127145044441" style="zoom: 50%;" />

- **Normal mapping + Texture**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145300076.png" alt="image-20231127145300076" style="zoom: 50%;" />

**PhongShader: Specular + Ambient + Diffuse**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128141210757.png" alt="image-20231128141210757" style="zoom: 50%;" />

## Tech

### Tangent Space Normal Mapping



### AECS ToneMapping 

- [Tone mapping进化论](https://zhuanlan.zhihu.com/p/21983679 ) 
- [ACES Filmic Tone Mapping Curve](https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)

A very elegant fitting.

```c++
float3 ACESToneMapping(float3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}
```

| With ACES ToneMapping                                        | Without ACES ToneMapping                                     |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150403640.png" alt="image-20231128150403640" style="zoom:67%;" /> | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150312255.png" alt="image-20231128150312255" style="zoom:67%;" /> |

### Omnidirectional Shadow maps

Omnidirectional Shadow maps(点光源阴影)

- Z-fighting https://en.wikipedia.org/wiki/Z-fighting

- DirectX - 改进阴影深度映射的常见技术 https://learn.microsoft.com/zh-cn/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN

Here we use  DepthShader to get Depth Buffer

| Depth Buffer（In Light View）                                |
| ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231129161027212.png" alt="image-20231129161027212" style="zoom: 25%;" /> |

​	For a point, We transform it form FrameBuffer screen coordinates to DepthBuffer screen coordinates.If a point's depth is deeper than DepthBuffer, then this point should be a shadow point. We let the `color*ShadowFactor`, and ShadowFactor is between 0 to 1 to make this point's color dark.

| With Shadow                                                  | Without Shadow                                               |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231129173100355.png" alt="image-20231129173100355" style="zoom:67%;" /> | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128150403640.png" alt="image-20231128150403640" style="zoom:67%;" /> |

当一个像素点在Shadow Map（阴影贴图）中找不到对应的深度值是很常见的问题，比如你的光源没有把整个角色照到。

以下是几种处理方法：

1. 固定阴影：将超出Shadow Map范围的像素点视为在阴影中，这样就可以给予物体一个全局的阴影效果。这种方法适用于开放世界游戏中的遥远物体或无法包含在Shadow Map中的大场景。
2. 边界处理：在将Shadow Map的深度值与对应像素点的深度值进行比较时，如果像素点处于阴影区域的边界上（即在Shadow Map范围内但未找到对应深度值），可以根据相邻像素点的深度值进行插值，以消除锯齿或产生更平滑的阴影过渡效果。
3. PSSM（Parallel Split Shadow Maps）：PSSM是一种将阴影贴图分成多个级别的技术，每个级别具有不同的观察矩阵和投影矩阵。如果一个像素点不在当前级别的Shadow Map中，可以尝试从下一个级别的Shadow Map中查找对应深度值。这种方法在遥远物体的阴影渲染中特别有效。
4. 使用近似算法：有时可以使用近似算法来估计不在Shadow Map中的像素点的阴影值。例如，可以使用屏幕空间的法线、深度等信息进行计算，以生成一个近似的阴影效果。



### SSAO

**屏幕空间环境光遮蔽(Screen-Space Ambient Occlusion, SSAO)**

- AO的概念理解 https://mentalraytips.blogspot.com/2008/11/joy-of-little-ambience.html

- https://learnopengl-cn.readthedocs.io/zh/latest/05%20Advanced%20Lighting/09%20SSAO/
- http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html

| With SSAO | Without SSAO |
| --------- | ------------ |
|           |              |

