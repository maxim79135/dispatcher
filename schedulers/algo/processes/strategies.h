#pragma once

#include "operations.h"
#include "requests.h"
#include "types.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace ProcessesManagement {

enum class StrategyType { ROUNDROBIN };

class AbstractStrategy {
public:
  virtual ~AbstractStrategy() {}

  AbstractStrategy(StrategyType type) : type(type) {}

  const StrategyType type;

  virtual std::string toString() const = 0;

  ProcessesState processRequest(const Request &request,
                                const ProcessesState &state) const {
    return std::visit(
        [this, state](const auto &req) {
          return this->processRequest(req, state);
        },
        request);
  }

  virtual ProcessesState processRequest(const CreateProcessReq &request,
                                        const ProcessesState &state) const = 0;

  virtual ProcessesState processRequest(const TerminateProcessReq &request,
                                        const ProcessesState &state) const = 0;

  virtual ProcessesState processRequest(const InitIO &request,
                                        const ProcessesState &state) const = 0;

  virtual ProcessesState processRequest(const TerminateIO &request,
                                        const ProcessesState &state) const = 0;

  virtual ProcessesState processRequest(const TransferControl &request,
                                        const ProcessesState &state) const = 0;

  virtual ProcessesState processRequest(const TimeQuantumExpired &request,
                                        const ProcessesState &state) const = 0;

protected:
  virtual std::optional<std::pair<int32_t, int32_t>>
  schedule(const ProcessesState &state) const = 0;

  std::optional<Process> getCurrent(const ProcessesState &state) const {
    auto current =
        std::find_if(state.processes.begin(), state.processes.end(),
                     [](const auto &process) {
                       return process.state() == ProcState::EXECUTING;
                     });
    if (current == state.processes.end()) {
      return std::nullopt;
    } else {
      return {*current};
    };
  }
};

using StrategyPtr = std::shared_ptr<AbstractStrategy>;

class RoundRobinStrategy final : public AbstractStrategy {
public:
  std::string toString() const override;

  static std::shared_ptr<RoundRobinStrategy> create();

protected:
  // <pid, queue>
  std::optional<std::pair<int32_t, int32_t>>
  schedule(const ProcessesState &state) const override;

private:
  RoundRobinStrategy();

public:
  ProcessesState processRequest(const CreateProcessReq &request,
                                const ProcessesState &state) const override;
  ProcessesState processRequest(const TerminateProcessReq &request,
                                const ProcessesState &state) const override;
  ProcessesState processRequest(const InitIO &request,
                                const ProcessesState &state) const override;
  ProcessesState processRequest(const TerminateIO &request,
                                const ProcessesState &state) const override;
  ProcessesState processRequest(const TransferControl &request,
                                const ProcessesState &state) const override;
  ProcessesState processRequest(const TimeQuantumExpired &request,
                                const ProcessesState &state) const override;
};

} // namespace ProcessesManagement