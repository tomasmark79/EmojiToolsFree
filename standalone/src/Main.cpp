// MIT License
// Copyright (c) 2024-2025 Tomáš Mark

#include "EmojiTools/EmojiTools.hpp"
#include "Logger/Logger.hpp"
#include "Utils/Utils.hpp"

#include <cxxopts.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Config {
  constexpr char standaloneName[] = "EmojiToolsApp";
  const std::filesystem::path executablePath = Utils::FSManager::getExecutePath ();
  constexpr std::string_view utilsAssetPath = UTILS_ASSET_PATH;
  constexpr std::string_view utilsFirstAssetFile = UTILS_FIRST_ASSET_FILE;
  const std::filesystem::path assetsPath = executablePath / utilsAssetPath;
  const std::filesystem::path assetsPathFirstFile = assetsPath / utilsFirstAssetFile;
}

std::unique_ptr<dotname::EmojiTools> uniqueLib;

int processArguments (int argc, const char* argv[]) {
  try {
    auto options = std::make_unique<cxxopts::Options> (argv[0], Config::standaloneName);
    options->positional_help ("[optional args]").show_positional_help ();
    options->set_width (80);
    options->set_tab_expansion ();
    options->add_options () ("h,help", "Show help");
    options->add_options () ("1,omit", "Omit library loading",
                             cxxopts::value<bool> ()->default_value ("false"));
    options->add_options () ("2,log2file", "Log to file",
                             cxxopts::value<bool> ()->default_value ("false"));
    const auto result = options->parse (argc, argv);

    if (result.count ("help")) {
      LOG_I_STREAM << options->help ({ "", "Group" }) << std::endl;
      return 0;
    }

    if (result["log2file"].as<bool> ()) {
      LOG.enableFileLogging (std::string (Config::standaloneName) + ".log");
      LOG_D_STREAM << "Logging to file enabled [-2]" << std::endl;
    }

    if (!result.count ("omit")) {
      // uniqueLib = std::make_unique<dotname::EmojiTools> ();
      // LOG_D << Config::assetsPath << std::endl;
      uniqueLib = std::make_unique<dotname::EmojiTools> (Config::assetsPath);

      LOG_I_STREAM << uniqueLib->getRandomEmoji () << std::endl;
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Smileys & Emotion");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("People & Body");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Component");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Animals & Nature");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Food & Drink");
      LOG_I_STREAM<< uniqueLib->getEmojiesFromGroup ("Travel & Places");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Activities");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Objects");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Symbols");
      LOG_I_STREAM << uniqueLib->getEmojiesFromGroup ("Flags") << std::endl;

    } else {
      LOG_D_STREAM << "Loading library omitted [-1]" << std::endl;
    }

    if (!result.unmatched ().empty ()) {
      for (const auto& arg : result.unmatched ()) {
        LOG_E_STREAM << "Unrecognized option: " << arg << std::endl;
      }
      LOG_I_STREAM << options->help () << std::endl;
      return 1;
    }

  } catch (const cxxopts::exceptions::exception& e) {
    LOG_E_STREAM << "error parsing options: " << e.what () << std::endl;
    return 1;
  }
  return 0;
}

int main (int argc, const char* argv[]) {
  LOG.noHeader (true);
  LOG_I_STREAM << "Starting " << Config::standaloneName << " ..." << std::endl;
  if (processArguments (argc, argv) != 0) {
    return 1;
  }
  return 0;
}