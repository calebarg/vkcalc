## vkcalc

#### Entire calculator inside a compute shader using vulkan graphics api.

#### Limitations:
- Integer arithmetic only
- No grouping operators
- No error handling
- Only supports linux

#### Usage
```vkcalc "expr"```

#### Build instructions

```./build_linux.sh && glslc vkcalc.comp -o compute.spv```
compile platform and shader code
