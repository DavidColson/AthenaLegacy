# Athena Toolset

This first version of Athen was a small game toolkit I was working on to learn about game engines, I made asteroids in it, which quite frankly is overcomplicated now and some other small demos. I have since abandoned the project because I wanted to work on game projects that were more focused and less about just game engine technology. Some of the source code of the project has been brough forward into newer projects of mine such as the new Athena, but I no longer work on this. The new Athena, while being a custom made engine, is being made as part of a specific game, which it turns out is a far better way of doing things.

Athena was a small, personal game development toolset built as simply as possible to serve little game experiments I feel like making. The idea is that I make little games and experiments and this drives me to develop various different features that get added to the toolset. I'm making games here, not unecessary game engine features. The first game is an asteroids remake, which looks like this:

![gameplay](GitHubImages/AsteroidsImage1.png)

![3D editor](https://user-images.githubusercontent.com/7236152/133889136-aefc77b1-ccd5-4255-a15d-688b2c5d9d1f.jpg)

![2D editor](https://user-images.githubusercontent.com/7236152/133889143-838c9eb1-efc8-4c8d-85f4-a498fcfdaea4.jpg)

A small sampling of the features it had

- 2D vector graphics
- Basic 3D graphics
- A type reflection system
- A simple editor with debug tools
- Post processing with bloom, chromatic abberation, screen warping and a few other things
- Font drawing
- Simple audio
- An Entity system
- Input

# Supported platforms

Currently only Windows 64 bit, as it uses DirectX11. It shouldn't be that hard to port to other platforms though, as the DX11 code all lives in a single file which can be replaced by a OpenGL or Vulkan version. One day I'll get around to this, promise.

# Design principles

 - Simpler is generally better, simpler code is easier to change
 - Never add complexity to solve a problem you aren't sure exists
 - Do not attempt to solve all problems
 - Be willing to rewrite systems that aren't solving the problem well enough (this is easier if things are kept simple)
 - Accept that you do not know what the end result will be like
 - When designing systems, it must be considered how they're used before writing them
 - Be explicit. Hiding things is asking for unseen complexity.
 - Design with performance in mind (DOD etc), but allow for fast iterations
 
 We essentially use the same practices and design principles as Godot. It has an excellent document on how to program using these ideas which you [can read here](https://docs.godotengine.org/en/stable/community/contributing/best_practices_for_engine_contributors.html)

 # Compiling

 Before you get to compiling, know that the only external dependency is SDL2. Before you try compiling, make sure SDL development libraries are installed somewhere on your PC.

 Once that's dealt with, it's the usual cmake affair. I like using the cmake gui, but you can use the command line too. Run cmake on the base directory, specifying the "Build" folder as the place to build binaries. It'll ask you for some directories of SDL, and after that generate and build. Currently only tested with VS 2017 Win64. But it should work fine with other generators, so long as it's 64 bit.
