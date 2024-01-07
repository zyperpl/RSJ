#include "dialog.hpp"

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.hpp"

ShopItem::AvailabilityReason ShopItem::is_available() const
{
  if (on_has_item && on_has_item(*this))
    return AvailabilityReason::AlreadyOwned;

  if (GAME.crystals < price)
    return AvailabilityReason::NotEnoughMoney;

  if (on_is_available && !on_is_available(*this))
    return AvailabilityReason::NotAvailable;

  return AvailabilityReason::Available;
}

const std::string Dialog::START_DIALOG_ID = "_start";
const std::string Dialog::END_DIALOG_ID   = "_end";

const Dialog Dialog::END_DIALOG{ Dialog::END_DIALOG_ID, "End of the conversation", {} };

std::unordered_map<std::string, bool> Dialog::introduced;

bool Dialog::is_introduced(const std::string &name)
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
    {
      DialogMap captain_dialogs;
      captain_dialogs.emplace(
        START_DIALOG_ID,
        Dialog{ "Captain",
                "Greetings, I am the Captain. I've been expecting you.\n"
                "Do you have a moment to discuss?",
                {
                  { "Yes, let's talk", "name", []() { introduced.insert_or_assign("Captain", true); } },
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
                                      "We are facing a critical issue. Our station is damaged \n"
                                      "and we need to repair it. \nAre you able to assist us in this matter?",
                                      {
                                        { "Yes, I'm ready to help", "help", []() { QUEST("captain1").accept(); } },
                                        { "Nope, bye", "no_help" },
                                      },
                                      []() -> std::optional<DialogId>
                                      {
                                        if (!QUEST("meet_captain").is_reported())
                                          QUEST("meet_captain").report();

                                        if (QUEST("captain1").is_accepted())
                                        {
                                          return "help2";
                                        }
                                        return std::nullopt;
                                      } });

      captain_dialogs.emplace("help",
                              Dialog{ "Captain",
                                      "Thank you. I'm glad to hear that.\n"
                                      "Our immediate objective is to $1collect 10 crystals$0.\n"
                                      "We need them to repair the station.\n"
                                      "These crystals can be found within the asteroids.\n"
                                      "Be careful, there are many dangers in space.",
                                      {
                                        { "Understood", "help_ok" },
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
                                      "Good luck, Pilot. I'll be waiting for you here on the station.",
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
    {
      DialogMap mechanic_dialogs;
      mechanic_dialogs.emplace(START_DIALOG_ID,
                               Dialog{ "Mechanic",
                                       "Hello, my name is Mechanic. Wanna buy something?",
                                       {
                                         { "Open shop", "_shop" },
                                         { "Open ship modification", "_ship" },
                                         { "I'm busy, goodbye", "_end" },
                                       },
                                       []() -> std::optional<DialogId>
                                       {
                                         introduced.insert_or_assign("Mechanic", true);

                                         if (!is_introduced("Captain"))
                                           return "about_captain";

                                         if (QUEST("meet_captain").is_reported())
                                           return "about_shop";

                                         if (is_introduced("Mechanic"))
                                           return "about_shop";

                                         return std::nullopt;
                                       } });

      mechanic_dialogs.emplace(
        "about_captain",
        Dialog{ "Mechanic",
                "Thanks for saving us. You need to talk to $2The Captain$0.\n"
                "He's expecting you on the station.",
                { { "Where can I find him?", "where_captain", []() { QUEST("meet_captain").accept(); } },
                  { "Understood", "_end", []() { QUEST("meet_captain").accept(); } } },
                []() -> std::optional<DialogId>
                {
                  if (QUEST("meet_captain").is_accepted())
                    return "where_captain";
                  return std::nullopt;
                } });

      mechanic_dialogs.emplace("where_captain",
                               Dialog{ "Mechanic",
                                       "$2The Captain$0 is currently on the bridge.\n"
                                       "You can find it on the $1northern part of the station$0.\n"
                                       "He awaits you there.",
                                       { { "Understood", "_end", []() { QUEST("meet_captain").accept(); } } },
                                       []() -> std::optional<DialogId>
                                       {
                                         if (QUEST("meet_captain").is_completed())
                                           return "about_shop";
                                         return std::nullopt;
                                       } });

      mechanic_dialogs.emplace("about_shop",
                               Dialog{ "Mechanic",
                                       "I have some items for sale.\nYou can buy them with crystals.",
                                       {
                                         { "Open shop", "_shop" },
                                         { "Open ship modification", "_ship" },
                                         { "I'm busy, goodbye", "_end" },
                                       } });

      dialogs.emplace("Mechanic", mechanic_dialogs);
    }

    {
      DialogMap navigator_dialogs;
      navigator_dialogs.emplace(START_DIALOG_ID,
                                Dialog{ "Navigator",
                                        "Greetings Pilot!\n"
                                        "We've detected a cluster of asteroids that are a danger to us.\n"
                                        "Would you require a detailed briefing on how to destroy them?",
                                        {
                                          { "Yes please!", "tutorial" },
                                          { "No, I know this", "_end" },
                                        },
                                        []() -> std::optional<DialogId>
                                        {
                                          if (QUEST("tutorial").is_reported())
                                            return "normal_talk";

                                          return std::nullopt;
                                        } });

      navigator_dialogs.emplace("tutorial",
                                Dialog{ "Navigator",
                                        "Use the $1arrow$0 keys to accelerate and rotate the ship.\n"
                                        "Navigate carefully to avoid colliding with asteroids.",
                                        {
                                          { "Okay", "tutorial2" },
                                        } });

      navigator_dialogs.emplace("tutorial2",
                                Dialog{ "Navigator",
                                        "Use the $1space$0 key to fire at asteroids.\n"
                                        "Some asteroids will drop crystals. Collect them by flying close.\n"
                                        "Be cautious, as asteroids will fragment into smaller pieces!",
                                        {
                                          { "Next", "tutorial3" },
                                        } });

      navigator_dialogs.emplace("tutorial3",
                                Dialog{ "Navigator",
                                        "Your mission ends upon the elimination of all asteroids.\n"
                                        "Good luck, Pilot!",
                                        {
                                          { "Thank you", "_end" },
                                        } });

      navigator_dialogs.emplace("normal_talk",
                                Dialog{ "Navigator",
                                        "Hello Pilot! How are you?",
                                        {
                                          { "I'm fine, thanks", "_end" },
                                          { "I'm busy, goodbye", "_end" },
                                        } });

      dialogs.emplace("Navigator", navigator_dialogs);
    }
  }

  return dialogs[name];
}
