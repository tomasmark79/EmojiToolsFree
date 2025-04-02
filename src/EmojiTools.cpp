// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include <EmojiTools/EmojiTools.hpp>
#include <Logger/Logger.hpp>
#include <Utils/Utils.hpp>

#include "UnicodeEmojiTestTxt.hpp"
#include <algorithm>
#include <assert.h>
#include <regex>

namespace dotname {

  EmojiTools::EmojiTools () {
    LOG_D << libName << " ...constructed" << std::endl;
  }
  EmojiTools::EmojiTools (const std::filesystem::path& assetsPath) : EmojiTools () {
    assetsPath_ = assetsPath;

    if (!assetsPath_.empty ()) {
      LOG_D << "Assets path: " << assetsPath_ << std::endl;
      std::mt19937 gen (rd ());

      // https://www.unicode.org/Public/draft/emoji/emoji-test.txt

      std::string emojiTestFileDefinition = std::string (assetsPath_) + "/emoji-test.txt";

      std::ifstream is (emojiTestFileDefinition);
      if (!is) {
        LOG_E << "Emoji asset test file not found. Loading hardcoded definition." << std::endl;
        std::istringstream ss (UnicodeEmojiTestTxtContent);
        constructEmojiPropertiesMap (m_emojiPropertiesMap, static_cast<std::istream&> (ss));
      } else {
        LOG_D_MSG ("Emoji asset test file found.");
        constructEmojiPropertiesMap (m_emojiPropertiesMap, is);
      }
    } else {
      LOG_D << "Assets path is empty" << std::endl;
    }
  }
  EmojiTools::~EmojiTools () {
    // !!!TODO!!!
    // LOG_D_DESTRUCTOR (EmojiTools) << libName << " ...destructed" << std::endl;
  }

  char8_t* EmojiTools::encodeUtf8 (char32_t emojiCodePoint, char8_t* buffer8_t) {
    auto byte = [] (char32_t x) {
      assert (x <= 0x100); // 256
      return static_cast<char8_t> (x);
    };

    char32_t continuation = 128;

    if (emojiCodePoint >= 65536) {
    *buffer8_t++ = byte(0b1111'0000 | (emojiCodePoint >> 18));
    *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 12) & 0b0011'1111));
    *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 6) & 0b0011'1111));
    *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else if (emojiCodePoint >= 2048) {
    *buffer8_t++ = byte(0b1110'0000 | (emojiCodePoint >> 12));
    *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 6) & 0b0011'1111));
    *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else if (emojiCodePoint >= 128) {
    *buffer8_t++ = byte(0b1100'0000 | (emojiCodePoint >> 6));
    *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else {
      *buffer8_t++ = byte (emojiCodePoint);
    }

    return buffer8_t;
  }

  char8_t* EmojiTools::encodeUtf8Sequence (const char32_t* emojiCodePoints, size_t length,
                                           char8_t* buffer8_t) {
    for (size_t i = 0; i < length; ++i) {
      buffer8_t = encodeUtf8 (emojiCodePoints[i], buffer8_t);
    }
    return buffer8_t;
  }

  void EmojiTools::constructEmojiPropertiesMap (std::map<int, EmojiPropertiesStructure>& epm,
                                                std::istream& file) {
    int mapKey = 0;
    std::vector<char32_t> emojiCodePoints;
    std::string emojiGroup;
    std::string emojiSubGroup;
    std::string emojiUnicodeVersion;
    std::string emojiTailDescription;
    std::string emojiTextDescription;
    std::string line;
    std::string token;

    // TODO Robust error handling (artefacted file, etc.)
    while (std::getline (file, line)) {
      if (line.empty ()) {
        continue;
      } else if (line[0] == '#' && (line.find ("# subgroup:") == std::string::npos)
                 && (line.find ("# group:") == std::string::npos)) {
        continue;
      } else if (line.find ("# subgroup:") != std::string::npos) {

        emojiSubGroup = line.substr (12);
        // pr("\tSubgroup: ");pr(emojiSubGroup); br();
        continue;
      } else if (line.find ("# group:") != std::string::npos) {
        emojiGroup = line.substr (9);
        // pr("Group: "); pr(emojiGroup); br();
        continue;
      } else if ((line[0] != '#') && (line.find ("#") != std::string::npos)
                 && (line.find ("# subgroup:") == std::string::npos)
                 && (line.find ("# group:") == std::string::npos)) {

        std::string unicodeString = line.substr (0, line.find (";"));
        // pr("\t\t"); pr(unicodeString);

        std::istringstream iss (unicodeString);
        emojiCodePoints.clear ();
        while (iss >> token) {
          token.erase (std::remove_if (token.begin (), token.end (),
                                       [] (char c) { return !std::isxdigit (c); }),
                       token.end ());

          uint32_t value;
          std::stringstream ss;
          ss << std::hex << token;
          ss >> value;
          emojiCodePoints.push_back (static_cast<char32_t> (value));
        }

        emojiTailDescription = line.substr (line.find ("#") + 1);
        // pr("\t"); pr(emojiTailDescription); br();

        // regular expression for extract unicode version
        std::regex unicodeRegex (R"((E\d+\.\d+))");
        std::smatch unicodeMatch;
        if (std::regex_search (emojiTailDescription, unicodeMatch, unicodeRegex)) {
          emojiUnicodeVersion = unicodeMatch[0];
          // pr("\t");pr("Unicode v.: "); pr(unicodeMatch[0]);
        }

        // extract emoji text description
        std::string::size_type unicodeVersionPos = emojiTailDescription.find (unicodeMatch[0]);
        if (unicodeVersionPos != std::string::npos) {
          emojiTextDescription = emojiTailDescription.substr (
              unicodeVersionPos + unicodeMatch[0].length () + 1, emojiTailDescription.size ());
          // pr("\t");pr("Desc.: ");pr(emojiTextDescription);
        }

        // combine emoji character from code points
        char8_t buffer[32];
        // Utf8Tools::Utf8Parser utf8tools;
        char8_t* end
            = encodeUtf8Sequence (emojiCodePoints.data (), emojiCodePoints.size (), buffer);
        *end = '\0'; // Null-terminating the string
        // pr("\t"); pr("Emoji: "); pr(reinterpret_cast<char *>(buffer));
        // br();
      }

      // create copy of structure
      EmojiPropertiesStructure eps;
      eps.m_emojiCodePoints = emojiCodePoints;
      eps.m_emojiGroup = emojiGroup;
      eps.m_emojiSubGroup = emojiSubGroup;
      eps.m_emojiUnicodeVersion = emojiUnicodeVersion;
      eps.m_emojiTextDescription = emojiTextDescription;

      // insert eps to epm
      epm.insert (std::pair<int, EmojiPropertiesStructure> (mapKey++, eps));

      // ***
      this->m_isPopulated = true;
    }
  }

  std::string& EmojiTools::getEmojiStringCharByCodePoint (char32_t* emojiCodePoints,
                                                          size_t length) {
    if (m_isPopulated) {
      std::fill (m_emojiBuffer, m_emojiBuffer + sizeof (m_emojiBuffer), 0);
      char8_t* end = encodeUtf8Sequence (emojiCodePoints, length, m_emojiBuffer);
      *end = '\0'; // Null-terminating the string
      return this->m_emojiCharacter = std::string (reinterpret_cast<char*> (m_emojiBuffer));
    } else
      // return "";
      return this->m_emojiCharacter = "";
  }

  char8_t& EmojiTools::getEmojiChar8_tCharByCodePoint (char32_t* emojiCodePoints, size_t length) {
    std::fill (m_emojiBuffer, m_emojiBuffer + sizeof (m_emojiBuffer), 0);
    char8_t* end = encodeUtf8Sequence (emojiCodePoints, length, m_emojiBuffer);
    *end = '\0'; // Null-terminating the string
    // std::cout << reinterpret_cast<char *>(buffer);
    // TODO
    return *m_emojiBuffer;
  }

  std::string& EmojiTools::getRandomEmoji () {
    gen.seed (std::random_device () ());
    std::uniform_int_distribution<> dis (0, 5);
    int a = dis (gen);

    switch (a) {
    case 0:
      return this->getRandomEmojiFromGroup ("Smileys & Emotion");
    case 1:
      return this->getRandomEmojiFromGroup ("Animals & Nature");
    case 2:
      return this->getRandomEmojiFromGroup ("Food & Drink");
    case 3:
      return this->getRandomEmojiFromGroup ("Activities");
    case 4:
      return this->getRandomEmojiFromGroup ("Travel & Places");
    default:
      return this->getRandomEmojiFromGroup ("Objects");
    }
  }

  std::string& EmojiTools::getRandomEmojiFromGroup (std::string emojiGroup) {
    if (m_isPopulated) {
      int count = 0;
      std::uniform_int_distribution<> dis (1, getSizeOfGroupItems (emojiGroup) - 1);
      int randomIndex = dis (gen);

      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiGroup == emojiGroup) {
          if (count == randomIndex) {
            return this->m_emojiCharacter = getEmojiStringCharByCodePoint (
                       epm.second.m_emojiCodePoints.data (), epm.second.m_emojiCodePoints.size ());
          }
          count++;
        }
      }
    }

    return this->m_emojiCharacter = "";
  }

  std::string& EmojiTools::getRandomEmojiFromSubGroup (std::string emojiSubGroup) {
    if (m_isPopulated) {
      int count = 0;
      std::uniform_int_distribution<> dis (1, getSizeOfSubGroupItems (emojiSubGroup) - 1);
      int randomIndex = dis (gen);

      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiSubGroup == emojiSubGroup) {
          if (count == randomIndex) {
            return this->m_emojiCharacter = getEmojiStringCharByCodePoint (
                       epm.second.m_emojiCodePoints.data (), epm.second.m_emojiCodePoints.size ());
          }
          count++;
        }
      }
    }
    return this->m_emojiCharacter = "";
  }

  std::string EmojiTools::getEmojiesFromGroup (std::string emojiGroup) {
    if (m_isPopulated) {
      std::string emojies = "";
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiGroup == emojiGroup) {
          emojies += getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                                    epm.second.m_emojiCodePoints.size ());
        }
      }
      return emojies;
    }
    return "";
  }

  std::string EmojiTools::getEmojiesFromSubGroup (std::string emojiSubGroup) {
    if (m_isPopulated) {
      std::string emojies = "";
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiSubGroup == emojiSubGroup) {
          emojies += getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                                    epm.second.m_emojiCodePoints.size ());
        }
      }
      return emojies;
    }
    return "";
  }

  std::vector<std::string> EmojiTools::getEmojiGroupsNames () {
    if (m_isPopulated) {
      std::vector<std::string> vecGroups;

      for (auto& epm : m_emojiPropertiesMap) {
        if (std::find (vecGroups.begin (), vecGroups.end (), epm.second.m_emojiGroup)
            == vecGroups.end ()) {
          vecGroups.push_back (epm.second.m_emojiGroup); // Groups map source
        }
      }
      return vecGroups;
    }
    return {};
  }

  std::vector<std::string> EmojiTools::getEmojiSubGroupsNames () {
    if (m_isPopulated) {
      std::vector<std::string> vecSubGroups;

      for (auto& epm : m_emojiPropertiesMap) {
        if (std::find (vecSubGroups.begin (), vecSubGroups.end (), epm.second.m_emojiSubGroup)
            == vecSubGroups.end ()) {
          vecSubGroups.push_back (epm.second.m_emojiSubGroup); // SubGroups map source
        }
      }
      return vecSubGroups;
    }
    return {};
  }

  int EmojiTools::getSizeOfGroupItems (std::string emojiGroup) {
    if (m_isPopulated) {
      int count = 0;
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiGroup == emojiGroup) {
          count++;
        }
      }
      return count;
    }
    return 0;
  }

  int EmojiTools::getSizeOfSubGroupItems (std::string emojiSubGroup) {
    if (m_isPopulated) {
      int count = 0;
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiSubGroup == emojiSubGroup) {
          count++;
        }
      }
      return count;
    }
    return 0;
  }

  std::string EmojiTools::getEmojiStringByIndexFromGroup (std::string emojiGroup, int index) {
    if (m_isPopulated) {
      int count = 0;
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiGroup == emojiGroup) {
          if (count == index) {
            return getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                                  epm.second.m_emojiCodePoints.size ());
          }
          count++;
        }
      }
    }
    return "";
  }

  std::string EmojiTools::getEmojiStringByIndexFromSubGroup (std::string emojiSubGroup, int index) {
    if (m_isPopulated) {
      int count = 0;
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiSubGroup == emojiSubGroup) {
          if (count == index) {
            return getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                                  epm.second.m_emojiCodePoints.size ());
          }
          count++;
        }
      }
    }
    return "";
  }

  std::string EmojiTools::getEmojiGroupDescription (std::string emojiGroup) {
    std::stringstream ss;

    if (m_isPopulated) {
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiGroup == emojiGroup) {
          ss << "Emoji: "
             << getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                               epm.second.m_emojiCodePoints.size ())
             << "\t Group: " << epm.second.m_emojiGroup
             << " | Subgroup: " << epm.second.m_emojiSubGroup
             << " | Description: " << epm.second.m_emojiTextDescription
             << " | Unicode version: " << epm.second.m_emojiUnicodeVersion << std::endl;
        }
      }
    }
    return ss.str ();
  }

  std::string EmojiTools::getEmojiSubGroupDescription (std::string emojiSubGroup) {
    std::stringstream ss;

    if (m_isPopulated) {
      for (auto& epm : m_emojiPropertiesMap) {
        if (epm.second.m_emojiSubGroup == emojiSubGroup) {
          ss << "Emoji: "
             << getEmojiStringCharByCodePoint (epm.second.m_emojiCodePoints.data (),
                                               epm.second.m_emojiCodePoints.size ())
             << "\t Group: " << epm.second.m_emojiGroup
             << " | Subgroup: " << epm.second.m_emojiSubGroup
             << " | Description: " << epm.second.m_emojiTextDescription
             << " | Unicode version: " << epm.second.m_emojiUnicodeVersion << std::endl;
        }
      }
    }
    return ss.str ();
  }

} // namespace dotname