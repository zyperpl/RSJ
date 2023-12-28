#pragma once

#include <functional>
#include <string>

struct Quest
{
  std::string description{};
  std::function<size_t()> progress;
  std::function<size_t()> max_progress;

  [[nodiscard]] bool is_accepted() const noexcept { return accepted; }
  void accept() noexcept { accepted = true; }

  [[nodiscard]] bool is_completed() const noexcept { return progress() >= max_progress(); }

  [[nodiscard]] bool is_reported() const noexcept { return reported; }
  void report() noexcept { reported = true; }

  bool accepted{ false };
  bool reported{ false };
};
