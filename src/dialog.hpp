#pragma once

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Dialog;

typedef std::string DialogId;
typedef std::unordered_map<DialogId, Dialog> DialogMap;

struct DialogResponse
{
  std::string text;
  DialogId next_dialog_id;

  std::function<void()> func;
};

struct Dialog
{
  std::string actor_name;
  std::string text;
  std::vector<DialogResponse> responses;

  std::function<std::optional<DialogId>()> prefunc;

  static DialogMap load_dialogs(const std::string &name);
  static const std::string START_DIALOG_ID;
  static const std::string END_DIALOG_ID;

  static const Dialog END_DIALOG;

private:
};
