# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

IAmSad is an Unreal Engine 5.5 third-person game project for Windows. It uses the Enhanced Input system and is configured for DX12 with ray tracing support.

## Build Commands

Build from command line using Unreal Build Tool (UBT):
```cmd
# Build for editor (Development)
"<UE5_PATH>\Engine\Build\BatchFiles\Build.bat" IAmSadEditor Win64 Development "<PROJECT_PATH>\IAmSad.uproject" -waitmutex

# Build standalone game
"<UE5_PATH>\Engine\Build\BatchFiles\Build.bat" IAmSad Win64 Development "<PROJECT_PATH>\IAmSad.uproject" -waitmutex

# Regenerate project files
"<UE5_PATH>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" "<PROJECT_PATH>\IAmSad.uproject"
```

Replace `<UE5_PATH>` with your UE5.5 installation path and `<PROJECT_PATH>` with the project directory.

## Architecture

### Module Structure
- **IAmSad** (Runtime module): Main game module containing gameplay code
- Dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput

### Key Classes
- `AIAmSadCharacter` (`Source/IAmSad/IAmSadCharacter.cpp`): Third-person character with spring arm camera, Enhanced Input bindings for movement/jumping/looking. Movement is currently side-scroll only (forward movement commented out).
- `AIAmSadGameMode` (`Source/IAmSad/IAmSadGameMode.cpp`): Sets default pawn to `BP_ThirdPersonCharacter` blueprint.

### Blueprint Integration
- Character C++ class is extended by `Content/ThirdPerson/Blueprints/BP_ThirdPersonCharacter`
- Input actions defined in `Content/ThirdPerson/Input/Actions/` (IA_Jump, IA_Look, IA_Move)
- Input mapping context: `Content/ThirdPerson/Input/IMC_Default`

### Maps
- Main map: `Content/ThirdPerson/Maps/ThirdPersonMap`
- Starter content maps available in `Content/StarterContent/Maps/`

## Engine Configuration
- Lumen global illumination enabled
- Virtual shadow maps enabled
- Static lighting disabled
- Ray tracing enabled
