<img src="assets/logo.png" alt="DotNameCpp Logo" width="20%">

[![Linux](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/linux.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/linux.yml)
[![MacOS](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/macos.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/macos.yml)
[![Windows](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/windows.yml/badge.svg)](https://github.com/tomasmark79/EmojiToolsFree/actions/workflows/windows.yml)  

# EmojiToolsFree

is a simple library that handles retrieving UTF-8 emoji characters on demand.

## Usage this project by other projects like the library

### Simply use the commands provided below.

  #### CMakeLists.txt

  ```cmake
  CPMAddPackage(
    NAME ETools
    GITHUB_REPOSITORY tomasmark79/EmojiToolsFree
    GIT_TAG main
  )
  file(COPY ${ETools_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})

  target_link_libraries(
    ${LIBRARY_NAME}
    PUBLIC dsdotname::ETools
  )
  ```

  #### C++

  ```cpp
    std::shared_ptr<EmojiSpace::EmojiToolsFree> /*💋*/ emojiTools
      = std::make_shared<EmojiSpace::EmojiToolsFree> (m_assetsPath);
  ```

## License

MIT License  
Copyright (c) 2024-2025 Tomáš Mark

[👆🏻](#EmojiToolsFree)