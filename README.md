# SoftwareRender

[toc]



## Show Case

- Normal Interpolation

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145044441.png" alt="image-20231127145044441" style="zoom: 50%;" />

- **Normal mapping + Texture**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145300076.png" alt="image-20231127145300076" style="zoom: 50%;" />

**PhongShader: Specular + Ambient + Diffuse**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128141210757.png" alt="image-20231128141210757" style="zoom: 50%;" />

## Tech

### Camera

- 规定$CenterLocation - CameraLocation$ 作为向量规定为**摄像机坐标系下z轴**的**正方向**，即**离摄像机越远的z值越大。**
- **右手坐标系**
- ![image-20231121144853967](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231121144853967.png)

#### 透视矫正插值

https://zhuanlan.zhihu.com/p/403259571

#### Culling

- 如何写一个软渲染(4)-Clipping - 拳四郎的文章 - 知乎 https://zhuanlan.zhihu.com/p/43586784



### Tangent Space Normal Mapping

法线贴图、切线空间 - 夏新温的文章 - 知乎
https://zhuanlan.zhihu.com/p/489792336

存储在模型顶点的**切线空间tangent space**中，优势：

1. 切线空间存储的是相对法线信息，因此换个网格（或者网格变换deforming）应用该纹理，也能得到合理的结果。

2. 可以进行uv动画，通过移动该纹理的uv坐标实现凹凸移动的效果，这种UV动画在水或者火山熔岩这种类型的物体会会用到。

3. 可以重用法线纹理，比如,一个砖块,我们仅使用一张法线纹理就可以用到所有的6个面。

4. 可以压缩。因为切线空间的法线z方向总是正方向，因此可以仅存储xy方向，从而推导z方向（存储的法线向量是单位向量，用勾股定理由xy得出z，取z为正的一个即可）。

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

### Shadow Mapping

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



#### PCF

#### PCSS

#### VSM

#### SDF

更多技术：

PCF/PCSS/VSM/SDF阴影

### AO

#### SSAO(Screen-Space Ambient Occlusion)

- AO的概念理解 https://mentalraytips.blogspot.com/2008/11/joy-of-little-ambience.html

- https://learnopengl-cn.readthedocs.io/zh/latest/05%20Advanced%20Lighting/09%20SSAO/
- http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
- 球体均匀采样https://mathworld.wolfram.com/SpherePointPicking.html

- 游戏后期特效第四发 -- 屏幕空间环境光遮蔽(SSAO) - 音速键盘猫的文章 - 知乎
  https://zhuanlan.zhihu.com/p/25038820

实现的几个细节：

- 物体相差远的地方, 会在深度图上误认为有AO,实际上不应该有AO

  <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231205011832980.png" alt="image-20231205011832980" style="zoom: 67%;" />

- 由于深度图精度问题, 即使本身深度通过的地方, 在相机看来不通过, 产生z-fighting现象.
- 降噪: 双边滤波

Only SSAO Exist with all white model 

| With SSAO                                                    | Without SSAO                                                 |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| ![image-20231204200320537](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231204200320537.png) | <img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231204201046444.png" alt="image-20231204201046444" style="zoom:85%;" /> |

#### HBAO





### Anti-Aliasing

#### TAA



#### MSAA



### SSR



### PBR



### IBL



### Scene Management

Multi Objects Render Pipeline

![image-20231210235752725](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231210235752725.png)





My Coord Tips:

![image-20231210235613863](https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231210235613863.png)
