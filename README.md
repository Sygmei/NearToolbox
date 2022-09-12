# NearToolbox

NEAR Protocol C++ library meant to be embedded

## Introduction

NearToolbox is the first and only (that I know of) NEAR Protocol C++ SDK !

Since this library is made with C++ (with optional C interface available), it can easily be embedded in all major games engines.

Integrations are already available for [Unity](https://unity.com/), [Godot](https://godot.org) and [ÖbEngine](https://obengine.io) (Unreal coming soon).

## Unity Integration

Enable the `BUILD_UNITY_INTEGRATION` CMake flag when building the project, this will build a `neartoolbox_unity` library.

Once the library is built, copy it in the `Assets/Plugins` directory of your Unity project.

Finally, use the [`NearClient.cs`](https://github.com/Sygmei/NearToolbox/blob/main/integrations/unity/scripts/NearClient.cs) script available on this repository to get started with using NEAR on Unity !

## Godot Integration

Enable the `BUILD_GODOT_INTEGRATION` CMake flag when building the project, this will build a `neartoolbox_godot` library.

> ⚠️ The build will take some time due to the `godot-cpp` compilation

Once the library is built, wrap it in a `.gdextension` module (see https://godotengine.org/article/introducing-gd-extensions for more details).

Finally, you can instantiate a `NodeClientWrapper` Node in your game and check this [example](https://github.com/Sygmei/NearToolbox/blob/main/integrations/godot/examples/simple_tx.gd) for usage.

## Unreal Integration (Coming soon)

Soon...

## ÖbEngine Integration (Coming soon)

Soon...