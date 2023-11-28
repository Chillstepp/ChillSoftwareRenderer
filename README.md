# SoftwareRender

## Show Case

- Normal Interpolation

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145044441.png" alt="image-20231127145044441" style="zoom: 50%;" />

- **Normal mapping + Texture**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231127145300076.png" alt="image-20231127145300076" style="zoom: 50%;" />

**PhongShader: Specular + Ambient + Diffuse**

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20231128141210757.png" alt="image-20231128141210757" style="zoom: 50%;" />

## Tech

### AECS ToneMapping 

- [Tone mapping进化论](https://zhuanlan.zhihu.com/p/21983679 ) 
- [ACES Filmic Tone Mapping Curve](https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/)

very elegant fitting

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

