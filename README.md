<img src="assets/logo.png" alt="DotNameCpp Logo" width="20%">

[![Linux](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/linux.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/linux.yml)
[![MacOS](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/macos.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/macos.yml)
[![Windows](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/windows.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/windows.yml)  

# EmojiToolsFree

is a simple library that handles retrieving UTF-8 emoji characters on demand.

## Usage

```cmake

CPMAddPackage(
    NAME EmojiToolsFree
    GITHUB_REPOSITORY tomasmark79/EmojiToolsFree
    GIT_TAG Freemain)

target_link_libraries(
    ${LIBRARY_NAME}
    PUBLIC dsdotname::EmojiToolsFree)
```

```cpp
    std::shared_ptr<EmojiSpace::EmojiToolsFree> emojiTools
      = std::make_shared<EmojiSpace::EmojiToolsFree>(assetsPath);
```
## License

MIT License  
Copyright (c) 2024-2025 Tomáš Mark

[👆🏻](#EmojiToolsFree)