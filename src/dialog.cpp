#include "dialog.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

const std::string Dialog::START_DIALOG_ID = "_start";

static std::unordered_map<std::string, bool> introduced;
static std::unordered_map<std::string, bool> quest_accepted;

[[nodiscard]] bool is_introduced(const std::string &name)
{
  if (!introduced.contains(name))
    return false;
  return introduced[name];
}

[[nodiscard]] bool is_quest_accepted(const std::string &name)
{
  if (!quest_accepted.contains(name))
    return false;
  return quest_accepted[name];
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
                { "My name is James", "name", [name]() { introduced.insert_or_assign(name, true); } },
                { "Goodbye", "_end" },
              },
              [name]() -> std::optional<DialogId>
              {
                if (is_introduced(name))
                  return "name";
                return std::nullopt;
              } });

    captain_dialogs.emplace(
      "name",
      Dialog{ "Captain",
              "Nice to meet you, James. I'm glad to see you.\n"
              "We have a problem. Our ship is damaged and we\n"
              "need to repair it. Can you help us?",
              {
                { "Yes", "help", [name]() { quest_accepted.insert_or_assign("captain1", true); } },
                { "No", "no_help" },
              },
              [name]() -> std::optional<DialogId>
              {
                if (is_quest_accepted("captain1"))
                  return "help2";
                return std::nullopt;
              } });

    captain_dialogs.emplace("help",
                            Dialog{ "Captain",
                                    "Thank you, James. I'm glad to hear that.\n"
                                    "We need to collect 10 crystals to repair the ship.\n"
                                    "You can find them in the asteroids.\n"
                                    "Be careful, there are many dangers in space.",
                                    {
                                      { "Ok", "help_ok" },
                                    } });

    captain_dialogs.emplace("help2",
                            Dialog{ "Captain",
                                    "We need to collect 10 crystals to repair the ship.\n"
                                    "You can find them in the asteroids.\n"
                                    "Be careful, there are many dangers in space.",
                                    {
                                      { "Ok", "help_ok" },
                                    } });

    captain_dialogs.emplace("help_ok",
                            Dialog{ "Captain",
                                    "Good luck, James. I'll be waiting for you on the ship.",
                                    {
                                      { "Goodbye", "_end" },
                                    } });

    captain_dialogs.emplace("no_help",
                            Dialog{ "Captain",
                                    "I'm sorry to hear that. Goodbye.",
                                    {
                                      { "Goodbye", "_end" },
                                    } });

    dialogs.emplace("captain", captain_dialogs);
  }

  return dialogs[name];
}
