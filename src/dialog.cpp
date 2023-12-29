#include "dialog.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.hpp"

const std::string Dialog::START_DIALOG_ID = "_start";
const std::string Dialog::END_DIALOG_ID   = "_end";

const Dialog Dialog::END_DIALOG{ Dialog::END_DIALOG_ID, "End of the conversation", {} };

static std::unordered_map<std::string, bool> introduced;

[[nodiscard]] bool is_introduced(const std::string &name)
{
  if (!introduced.contains(name))
    return false;
  return introduced[name];
}

std::unordered_map<DialogId, Dialog> Dialog::load_dialogs(const std::string &name)
{
  static std::unordered_map<std::string, DialogMap> dialogs;
  if (dialogs.empty())
  {
    DialogMap captain_dialogs;
    captain_dialogs.emplace(
      START_DIALOG_ID,
      Dialog{ "Captain",
              "Hello, my name is Captain. What is your name?",
              {
                { "My name is James", "name", []() { introduced.insert_or_assign("Captain", true); } },
                { "I'm busy, goodbye", "_end" },
              },
              []() -> std::optional<DialogId>
              {
                if (is_introduced("Captain"))
                  return "name";
                return std::nullopt;
              } });

    captain_dialogs.emplace("name",
                            Dialog{ "Captain",
                                    "Nice to meet you, James. I'm glad to see you.\n"
                                    "We have a problem. Our ship is damaged and we\n"
                                    "need to repair it. Can you help us?",
                                    {
                                      { "Yes", "help", []() { QUEST("captain1").accept(); } },
                                      { "No", "no_help" },
                                    },
                                    []() -> std::optional<DialogId>
                                    {
                                      if (QUEST("captain1").is_accepted())
                                        return "help2";
                                      return std::nullopt;
                                    } });

    captain_dialogs.emplace("help",
                            Dialog{ "Captain",
                                    "Thank you, James. I'm glad to hear that.\n"
                                    "We need to $1collect 10 crystals$0 to repair the station.\n"
                                    "You can find them in the asteroids.\n"
                                    "Be careful, there are many dangers in space.",
                                    {
                                      { "Okay", "help_ok" },
                                    } });

    captain_dialogs.emplace("help2",
                            Dialog{ "Captain",
                                    "We need to $1collect 10 crystals$0 to repair the station.\n"
                                    "You can find them in the asteroids.\n"
                                    "Be careful, there are many dangers in space.",
                                    {
                                      { "Okay", "help_ok" },
                                    },
                                    [&]() -> std::optional<DialogId>
                                    {
                                      if (QUEST("captain1").is_completed())
                                        return "completed1";
                                      return std::nullopt;
                                    } });

    captain_dialogs.emplace("help_ok",
                            Dialog{ "Captain",
                                    "Good luck, James. I'll be waiting for you on the station.",
                                    {
                                      { "Goodbye!", "_end" },
                                    } });

    captain_dialogs.emplace("no_help",
                            Dialog{ "Captain",
                                    "I'm sorry to hear that. Goodbye.",
                                    {
                                      { "Goodbye", "_end" },
                                    } });

    captain_dialogs.emplace("completed1",
                            Dialog{ "Captain",
                                    "Thank you for collecting $1the crystals$0!\n"
                                    "Now we can repair the station.",
                                    {
                                      { "Goodbye!", "_end", []() { QUEST("captain1").report(); } },
                                    },
                                    []() -> std::optional<DialogId>
                                    {
                                      if (QUEST("captain1").is_reported())
                                        return "hello";
                                      return std::nullopt;
                                    } });

    captain_dialogs.emplace("hello", Dialog{ "Captain", "Hello, James.", { { "See you later!", "_end" } } });

    dialogs.emplace("Captain", captain_dialogs);
  }

  return dialogs[name];
}
