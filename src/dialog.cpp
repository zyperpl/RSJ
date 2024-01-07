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
                                        { "Yes, I'm ready to help",
                                          "help",
                                          []()
                                          {
                                            QUEST("captain1").accept();
                                            MISSION(1).unlock();
                                            MISSION(2).unlock();
                                          } },
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
                                      "We need to $1obtain 10 crystals$0 to repair the station.\n"
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
                                      "Thank you for obtaining $1the crystals$0!\n"
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

      captain_dialogs.emplace("hello",
                              Dialog{ "Captain",
                                      "Go talk to the crew if you need anything.\n"
                                      "There might be many things to do around here.\n",
                                      { { "Ok, bye!", "_end" } },
                                      []() -> std::optional<DialogId>
                                      {
                                        if (QUEST("meet_captain2").is_accepted())
                                          return "endgame1";

                                        return std::nullopt;
                                      } });

      captain_dialogs.emplace("endgame1",
                              Dialog{ "Captain",
                                      "I'm glad you're here.\n"
                                      "Our scans detected many unidentified objects in the area.\n"
                                      "You need to investigate them.\n",
                                      { { "What can I do?",
                                          "endgame2",
                                          []()
                                          {
                                            MISSION(5).unlock();
                                            MISSION(6).unlock();
                                            MISSION(7).unlock();
                                            QUEST("meet_captain2").report();
                                          } } },
                                      []() -> std::optional<DialogId>
                                      {
                                        if (QUEST("meet_captain2").is_reported())
                                          return "endgame2";
                                        return std::nullopt;
                                      } });

      captain_dialogs.emplace("endgame2",
                              Dialog{ "Captain",
                                      "You can use your mission computer to select the mission.\n"
                                      "But be careful, you might need the best equipment for this.\n"
                                      "You can upgrade your ship in the mechanic's shop.\n",
                                      { { "Understood", "_end", []() { QUEST("all_missions").accept(); } } },
                                      []() -> std::optional<DialogId>
                                      {
                                        if (QUEST("all_missions").is_completed())
                                        {
                                          if (!QUEST("all_missions").is_reported())
                                            QUEST("all_missions").report();
                                          return "endgame3";
                                        }
                                        return std::nullopt;
                                      } });

      captain_dialogs.emplace("endgame3",
                              Dialog{ "Captain",
                                      "Congratulations, you've completed all missions!\n"
                                      "You can continue hanging around here.\n"
                                      "There are many unanswered questions.\n"
                                      "But there is no time to talk about that now.\n",
                                      { { "Wow, thank you!", "_end" }, { "Ok, bye.", "_end" } } });

      dialogs.emplace("Captain", captain_dialogs);
    }
    {
      DialogMap mechanic_dialogs;
      mechanic_dialogs.emplace(START_DIALOG_ID,
                               Dialog{ "Mechanic",
                                       "Hello, my name is Mechanic. Wanna buy something?",
                                       {
                                         { "Yes", "_shop" },
                                         { "I want to change my ship's gun", "_ship" },
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
                                       "You can find it on the $1northern part&U of the station$0.\n"
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
                                         { "I want to buy", "_shop" },
                                         { "I want to change my ship's weapons", "_ship" },
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
    {
      DialogMap scientist_dialogs;
      scientist_dialogs.emplace(START_DIALOG_ID,
                                Dialog{ "Scientist",
                                        "Hello!\n"
                                        "Can I help you sir?",
                                        {
                                          { "What are you doing?", "blocked_lab" },
                                          { "Where is the Captain?", "where_captain" },
                                          { "I'm busy, goodbye!", "_end" },
                                        },
                                        []() -> std::optional<DialogId>
                                        {
                                          if (QUEST("captain1").is_reported())
                                            return "normal_talk";

                                          return std::nullopt;
                                        } });

      scientist_dialogs.emplace("blocked_lab",
                                Dialog{ "Scientist",
                                        "I'm trying to get into the lab, but this part of the station\n"
                                        "is not available due to the damage.\n",
                                        {
                                          { "I see", "blocked_lab2" },
                                        } });

      scientist_dialogs.emplace("blocked_lab2",
                                Dialog{ "Scientist",
                                        "I guess I'll have to wait until the station is repaired.\n"
                                        "I hope the Captain will be able to repair the station soon.",
                                        {
                                          { "Okay", "blocked_lab3" },
                                        } });

      scientist_dialogs.emplace("blocked_lab3",
                                Dialog{ "Scientist",
                                        "Is there anything else I can help you with?",
                                        {
                                          { "Where is the Captain?", "where_captain" },
                                          { "No, goodbye!", "_end" },
                                        } });

      scientist_dialogs.emplace("where_captain",
                                Dialog{ "Scientist",
                                        "The Captain is currently on the bridge.\n"
                                        "Just go to the $1northern part&U of the station$0.\n",
                                        {
                                          { "Understood, thank you", "_end" },
                                        } });

      scientist_dialogs.emplace(
        "normal_talk",
        Dialog{ "Scientist",
                "Hello! How are you?",
                {
                  { "I'm fine, thanks", "quest1_intro", []() { introduced.insert_or_assign("Scientist", true); } },
                },
                []() -> std::optional<DialogId>
                {
                  if (QUEST("scientist1").is_reported())
                    return "quest1_completed";

                  if (QUEST("scientist1").is_accepted())
                    return "quest1_accept";

                  return std::nullopt;
                } });
      scientist_dialogs.emplace("quest1_intro",
                                Dialog{ "Scientist",
                                        "I'm glad to hear that.\n"
                                        "I'm currently working on a new project.\n"
                                        "I'm investigating the $1asteroids$0.\n",
                                        {
                                          { "Go on", "quest1_intro2" },
                                        } });

      scientist_dialogs.emplace("quest1_intro2",
                                Dialog{ "Scientist",
                                        "I'm trying to find out why there are so many asteroids in this area.\n"
                                        "We've found artifacts in some of them.\n"
                                        "I'm trying to find out what they are.\n",
                                        {
                                          { "I see", "quest1_intro3" },
                                        } });

      scientist_dialogs.emplace(
        "quest1_intro3",
        Dialog{ "Scientist",
                "I'll need to collect some more alien artifacts \n"
                "to continue my research.\n"
                "Can you help me with that?",
                {
                  { "Yes, I'll help you", "quest1_accept", []() { QUEST("scientist1").accept(); } },
                } });

      scientist_dialogs.emplace("quest1_accept",
                                Dialog{ "Scientist",
                                        "Artifacts where previously found in the asteroids.\n"
                                        "Use your mission computer to track them down.\n"
                                        "And go to the specific area to collect them.\n",
                                        {
                                          { "Understood", "_end", []() { MISSION(4).unlock(); } },
                                        },
                                        []() -> std::optional<DialogId>
                                        {
                                          if (QUEST("scientist1").is_completed())
                                            return "quest1_completed";
                                          return std::nullopt;
                                        } });

      scientist_dialogs.emplace("quest1_completed",
                                Dialog{ "Scientist",
                                        "Thank you for your help.\n"
                                        "I'll continue my research now.",
                                        {
                                          { "Goodbye",
                                            "_end",
                                            []()
                                            {
                                              if (!QUEST("scientist1").is_reported())
                                                QUEST("scientist1").report();
                                              if (!QUEST("meet_captain2").is_accepted())
                                                QUEST("meet_captain2").accept();
                                            } },
                                        },
                                        []() -> std::optional<DialogId>
                                        {
                                          if (QUEST("scientist1").is_reported())
                                            return "all_done1";
                                          return std::nullopt;
                                        } });

      scientist_dialogs.emplace("all_done1",
                                Dialog{ "Scientist",
                                        "The more we investigate the asteroids, the more questions we have.\n"
                                        "Also there is one weird thing...\n",
                                        {
                                          { "What is it?", "all_done2" },
                                          { "I'm busy, goodbye!", "_end" },
                                        } });

      scientist_dialogs.emplace("all_done2",
                                Dialog{ "Scientist",
                                        "We measured space anomalies in the area.\n"
                                        "And we concluded that space around us is a donut.",
                                        {
                                          { "What?", "all_done3" },
                                          { "Nonsense, goodbye!", "_end" },
                                        } });

      scientist_dialogs.emplace("all_done3",
                                Dialog{ "Scientist",
                                        "Yes, you heard me right.\n"
                                        "Space is a donut.\n",
                                        {
                                          { "I don't believe you", "all_done4" },
                                          { "I'm busy, goodbye!", "_end" },
                                        } });

      scientist_dialogs.emplace("all_done4",
                                Dialog{ "Scientist",
                                        "I know it sounds crazy, but it's true.\n"
                                        "We've measured it. After you go to the edge of the universe,\n"
                                        "you'll end up where you started.\n"
                                        "Space is warped like a donut.\n",
                                        {
                                          { "You are right!", "all_done5" },
                                          { "Ok, bye!", "_end" },
                                        } });

      scientist_dialogs.emplace("all_done5",
                                Dialog{ "Scientist",
                                        "Yep, I know.\n"
                                        "I'm glad you understand.\n",
                                        { { "Goodbye!", "_end" } } });

      dialogs.emplace("Scientist", scientist_dialogs);
    }
  }

  return dialogs[name];
}
