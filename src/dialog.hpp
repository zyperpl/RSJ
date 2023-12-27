#pragma once

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

typedef std::string DialogId;

struct DialogResponse
{
  std::string text;
  DialogId next_dialog_id;
};

struct Dialog
{
  std::string actor_name;
  std::string text;
  std::vector<DialogResponse> responses;
};

class DialogManager
{
public:
  DialogManager() { load_dialogs(); }

  const Dialog &get_dialog(const DialogId &dialog_id) const
  {
    assert(dialogs.contains(dialog_id));
    return dialogs.at(dialog_id);
  }

private:
  void load_dialogs()
  {
    dialogs.clear();

    dialogs.emplace("start",
                    Dialog{ "Captain",
                            "Hello, my name is Captain. What is your name?",
                            {
                              { "My name is James", "name" },
                              { "Goodbye", "_end" },
                            } });

    dialogs.emplace(
      "name",
      Dialog{ "Captain",
              "Nice to meet you, James. I'm glad to see you.\n"
              "We have a problem. Our ship is damaged and we\n"
              "need to repair it. Can you help us?",
              {
                { "Yes", "help" },
                { "No", "no_help" },
              } });

    dialogs.emplace("help",
                    Dialog{ "Captain",
                            "Thank you, James. I'm glad to hear that.\n"
                            "We need to collect 10 crystals to repair the ship.\n"
                            "You can find them in the asteroids.\n"
                            "Be careful, there are many dangers in space.",
                            {
                              { "Ok", "help_ok" },
                            } });

    dialogs.emplace("help_ok",
                    Dialog{ "Captain",
                            "Good luck, James. I'll be waiting for you on the ship.",
                            {
                              { "Goodbye", "_end" },
                            } });

    dialogs.emplace("no_help",
                    Dialog{ "Captain",
                            "I'm sorry to hear that. Goodbye.",
                            {
                              { "Goodbye", "_end" },
                            } });
  }
  std::unordered_map<DialogId, Dialog> dialogs;
};
