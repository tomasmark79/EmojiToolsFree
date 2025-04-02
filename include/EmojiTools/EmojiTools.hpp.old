#ifndef __EMOJITOOLS_H__
#define __EMOJITOOLS_H__

// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <random>
#include <cstddef>
#if __cplusplus < 202002L
using char8_t = char; // C++17 and older
#endif


#define EMOJI_BUFFER_SIZE 64

namespace EmojiSpace
{
  class EmojiTools
  {

    // Public API

  public:
    EmojiTools (const std::string &assetsPath);
    ~EmojiTools ();

    // getters
    std::string &getEmojiStringCharByCodePoint (char32_t *emojiCodePoints,
                                                size_t length);
    char8_t &getEmojiChar8_tCharByCodePoint (char32_t *emojiCodePoints,
                                             size_t length);

    std::string &getRandomEmoji ();
    std::string &getRandomEmojiFromGroup (std::string emojiGroup);
    std::string &getRandomEmojiFromSubGroup (std::string emojiSubGroup);
    
    std::string getEmojiesFromGroup (std::string emojiGroup);
    std::string getEmojiesFromSubGroup (std::string emojiSubGroup);
    std::vector<std::string> getEmojiGroupsNames ();
    std::vector<std::string> getEmojiSubGroupsNames ();
    int getSizeOfGroupItems (std::string emojiGroup);
    int getSizeOfSubGroupItems (std::string emojiSubGroup);
    
    std::string getEmojiStringByIndexFromGroup (std::string emojiGroup,
                                                int index);
    std::string getEmojiStringByIndexFromSubGroup (std::string emojiSubGroup,
                                                   int index);
    std::string getEmojiGroupDescription (std::string emojiGroup);
    std::string getEmojiSubGroupDescription (std::string emojiSubGroup);

  private:
    // alternatively, you can use a getter function
    const std::string getAssetsPath () const { return m_assetsPath; }

    std::random_device rd;
    std::mt19937 gen;

    bool m_isPopulated{ false };

    struct EmojiPropertiesStructure
    {
      std::vector<char32_t> m_emojiCodePoints;
      std::string m_emojiGroup;
      std::string m_emojiSubGroup;
      std::string m_emojiUnicodeVersion;
      std::string m_emojiTextDescription;
    };

    std::string m_assetsPath;

    char8_t m_emojiBuffer[EMOJI_BUFFER_SIZE];
    std::string m_emojiCharacter;

    std::map<int, EmojiPropertiesStructure> m_emojiPropertiesMap;

    void
    constructEmojiPropertiesMap (std::map<int, EmojiPropertiesStructure> &epm,
                                 std::istream &file);
    char8_t *encodeUtf8 (char32_t emojiCodePoint, char8_t *buffer8);
    char8_t *encodeUtf8Sequence (const char32_t *emojiCodePoints,
                                 size_t length, char8_t *buffer8);
  };

} // namespace library

#endif // __EMOJITOOLS_H__
