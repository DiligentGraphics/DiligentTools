# Render State Packager
## Run
Example Run
```sh
Diligent-RenderStatePackager.exe -o Archive.bin --vulkan --dx12 -c Config.json -s . -i SamplePSO_0.drsn -i SamplePSO_1.drsn
```
### Arguments
- #### Device Flags
  - ```--dx11``` 
  - ```--dx12```
  - ```--vulkan```
  - ```--opengl```
  - ```--opengles```
  - ```--metal_macos```
  - ```--metal_ios```
- #### Another
    - ```-o``` Binary Output
    - ```-s``` Shader Directory
    - ```-c``` Config File
    - ```-i``` Input DRSN Files

## DRSN
Example DRSN file

```json
{
    "ResourceSignatures": [
        {
            "Name": "Signature0",
            "Resources": [
                {
                    "Name": "Constants",
                    "ShaderStages": [
                        "VERTEX",
                        "PIXEL"
                    ],
                    "ResourceType": "CONSTANT_BUFFER"
                }
            ],
            "ImmutableSamplers": [
                {
                    "ShaderStages": ["PIXEL"],
                    "SamplerOrTextureName": "LinearSampler",
                    "Desc": {
                        "BorderColor" : [1.0, 1.0, 1.0, 1.0]
                    }
                }
            ]
        }
    ],
    "Shaders": [
        {
            "Desc": {
                "Name": "Draw command test vertex shader",
                "ShaderType": "VERTEX"
            },
            "SourceLanguage": "HLSL",
            "FilePath": "cube.vsh",
            "UseCombinedTextureSamplers": true,
            "Macros": [
                {"Name": "DEFINE_0", "Definition": "0"}
            ]
        },
        {
            "Desc": {
                "Name": "Draw command test pixel shader",
                "ShaderType": "PIXEL"
            },
            "SourceLanguage": "HLSL",
            "FilePath": "cube.psh",
            "UseCombinedTextureSamplers": true
        }
    ],
    "Pipeleines": [
        {
            "GraphicsPipeline": {
                "DepthStencilDesc": {
                    "DepthEnable": false
                },
                "InputLayout": {
                    "LayoutElements": [
                        {
                            "NumComponents": 3
                        },
                        {
                            "InputIndex": 1,
                            "NumComponents": 4
                        }
                    ]
                },
                "NumRenderTargets": 1,
                "RTVFormats": {
                    "0": "RGBA16_SINT"
                },
                "RasterizerDesc": {
                    "CullMode": "FRONT"
                },
                "BlendDesc": {
                    "RenderTargets": {
                        "0": {
                            "BlendEnable": true
                        }
                    }
                }
            },
            "PSODesc": {
                "Name": "PSO_NAME_0",
                "PipelineType": "GRAPHICS"
            },
            "ppResourceSignatures": [
                "Signature0"
            ],
            "pVS": "Draw command test vertex shader",
            "pPS": "Draw command test pixel shader"
        }
    ]
}
```
## Config
Example config file (PS: For more information see other fields of the `SerializationDeviceCreateInfo`)
```json
{
    "Vulkan": {
        "Version": { "Major": 1, "Minor": 2 },
        "SupportedSpirv14": true,
        "DxCompilerPath": "${PATH_TO_COMPILER}"
    },
    "Metal": {
        "MslPreprocessorCmd": "${PREPROCESSOR_CMD}"
    }
}
```
