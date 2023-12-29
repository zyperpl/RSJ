#pragma once

#include <functional>
#include <string>

struct Quest
{
  std::string description{};
  std::function<size_t()> progress;
  std::function<size_t()> max_progress;
  std::function<void()> on_report;

  [[nodiscard]] bool is_accepted() const noexcept { return accepted; }
  void accept() noexcept { accepted = true; }

  [[nodiscard]] bool is_completed() const noexcept { return reported || progress() >= max_progress(); }

  [[nodiscard]] bool is_reported() const noexcept { return reported; }
  void report() noexcept;

  bool accepted{ false };
  bool reported{ false };

  size_t score{ 12000 };
};
