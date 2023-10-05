# Render State Packager

Render state packager is an off-line render state processing, optimization and packaging tool. It uses JSON-based render state description called
Diligent Render State Notation (DRSN). The states defined in a DRSN file are processed off-line and packaged into an archive in a format
optimized for run-time loading performance.

## Command Line Arguments

|       Argument            |         Description                                                |   Default value     |
|---------------------------|--------------------------------------------------------------------|---------------------|
| `-o` (`output`)           | archive output file                                                |  `Archive.bin`      |
| `-s` (`shader_dir`)       | shader search drectory                                             |  `.`                |
| `-r` (`render_state_dir`) | render states search directory                                     |  `.`                |
| `-c` (`config`)           | config file                                                        |                     |
| `-t` (`thread`)           | thread Count                                                       |  System CPU count   |
| `-i` (`input`)            | input DRSN file (Required)                                         |                     |
| `-d` (`dump_dir`)         | bytecode dump directory                                            |                     |
| `strip_reflection`        | strip reflection information when packing shaders into the archive |  No                 |

Device Flags (at least one flag is required):
  - `--dx11`
  - `--dx12`
  - `--vulkan`
  - `--opengl`
  - `--opengles`
  - `--metal_macos`
  - `--metal_ios`

Flags not supported on the platform (for example, `--metal_macos` on Windows or Linux) are ignored.


Example:

```sh
Diligent-RenderStatePackager.exe -o Archive.bin --vulkan --dx12 -c Config.json -s . -i SamplePSO_0.drsn -i SamplePSO_1.drsn
```

## Render State Notation

DRSN is a JSON-based description that mirrors core structures. The JSON file consists of three main sections: 

* *Shaders* sections defines shaders
* *ResourceSignatures* section contains the descriptions of pipeline resource signatures
* *Pipeleines* section describes pipeline states

**Important**: all shader, signature and pipeline resource names in an archive must be unique.

Example of a DRSN file:

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
                    "VarType": "DYNAMIC",
                    "ResourceType": "CONSTANT_BUFFER",
                    "Flags": ["NO_DYNAMIC_BUFFERS"]
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
                "Name": "My vertex shader",
                "ShaderType": "VERTEX"
            },
            "SourceLanguage": "HLSL",
            "FilePath": "VertexShader.vsh",
            "EntryPoint": "VSmain",
            "UseCombinedTextureSamplers": true,
            "Macros": [
                {"Name": "MY_MACRO_0", "Definition": "0"}
            ]
        },
        {
            "Desc": {
                "Name": "My pixel shader",
                "ShaderType": "PIXEL"
            },
            "SourceLanguage": "HLSL",
            "FilePath": "MyPixelShader.psh",
            "EntryPoint": "PSmain",
            "UseCombinedTextureSamplers": true
        }
    ],
    "Pipeleines": [
        {
            "PSODesc": {
                "Name": "My PSO 0",
                "PipelineType": "GRAPHICS"
            },
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
                    "0": "RGBA8_UNORM_SRGB"
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
            "ppResourceSignatures": [
                "Signature0"
            ],
            "pVS": "My vertex shader",
            "pPS": "My pixel shader"
        }
    ]
}
```

If shaders are used by a single pipeline state, they can be defined inline in the PSO description, for instance:


```json
{
    "Pipeleines": [
        {
            "PSODesc": {
                "Name": "My PSO 1",
            },
            "GraphicsPipeline": {
                "RTVFormats": {
                    "0": "RGBA8_UNORM_SRGB"
                },
            },
            "pVS": {
                "Desc": {
                    "Name": "My vertex shader"
                },
                "FilePath": "VertexShader.vsh",
                "EntryPoint": "VSmain"
            },
            "pPS": {
                "Desc": {
                    "Name": "My pixel shader"
                },
                "FilePath": "PixelShader.psh",
                "EntryPoint": "PSmain"
            }

        }
    ]
}
```

## Import Feature

DRSN files may be imported into another files allowing sharing deifintions of common states, for example:

```
Resources
|   Diligent-RenderStatePackager.exe
| ------ Shaders
   |   Shader0.hlsl
   |   Shader1.hlsl
   |    ...
| ------ RenderStates
   |   RenderState0.json
   |   RenderState1.json
   |    ...
   |   RenderStateN.json
   |   RenderStatesLib.json
``` 

```json
// RenderStatesLib.json
{
   
    "Imports": [
        "RenderState0.json",
        "RenderState1.json",
        ....
        "RenderStateN.json"
    ]
    ...
}
```

Use `-r` command line option to define the render state notation search path(s):

```sh
Diligent-RenderStatePackager.exe -s ./Shaders -r ./RenderStates -i RenderStatesLib.drsn --vulkan
```

## Defaults

Render state notation file may contain `Defaults` section that defines the default values for shaders, pipeline states and
signatures, for example:

```json
{
    "Defaults": {
        "Shader": {
            "SourceLanguage": "HLSL",
            "UseCombinedTextureSamplers": true
        },
        "Pipeline": {
            "PSODesc": {
                "ResourceLayout": {
                    "DefaultVariableType": "MUTABLE"
                }
            }
        }
    },
    "Pipelines": [
    ],
}
```

## Packager Configuration

Render state packager configuration mirrors the fields of the `SerializationDeviceCreateInfo` struct.

Example config file:

```json
{
    "Vulkan": {
        "Version": { "Major": 1, "Minor": 2 },
        "SupportsSpirv14": true,
        "DxCompilerPath": "PATH_TO_COMPILER"
    },
    "Metal": {
        "MslPreprocessorCmd": "PREPROCESSOR_CMD"
    }
}
```

Note: to enable shader debugging in XCode, use `-gline-tables-only -frecord-sources` command line
options for Metal (`-frecord-sources` replaces `-MO` that was used in earlier versions of XCode):

```json
{
    "Metal": {
        "CompileOptionsMacOS": "-sdk macosx metal -gline-tables-only -frecord-sources",
        "CompileOptionsiOS": "-sdk iphoneos metal -gline-tables-only -frecord-sources"
    }
}
```
