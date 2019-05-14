#pragma once

#include <istream>
#include <map>
#include <ostream>
#include <variant>
#include <vector>

#include "../algo/processes/types.h"
#include "tasks.h"

namespace Utils::details {
using Utils::TaskException;

/**
 *  @brief Создает объект задания "Диспетчеризация памяти" из JSON-объекта.
 *
 *  @param obj JSON-объект.
 *
 *  @return Объект задания.
 *
 *  @throws Utils::TaskException Исключение возникает в следующих случаях:
 *
 *  "UNKNOWN_STRATEGY" - неизвестный тип стратегии выбора блока памяти;
 *  "UNKNOWN_REQUEST" - неизвестный тип заявки.
 */
inline MemoryTask loadMemoryTask(const nlohmann::json &obj) {
  using namespace MemoryManagement;
  StrategyPtr strategy;

  auto strategyType = obj["strategy"];
  if (strategyType == "FIRST_APPROPRIATE") {
    strategy = FirstAppropriateStrategy::create();
  } else if (strategyType == "MOST_APPROPRIATE") {
    strategy = MostAppropriateStrategy::create();
  } else if (strategyType == "LEAST_APPROPRIATE") {
    strategy = LeastAppropriateStrategy::create();
  } else {
    throw TaskException("UNKNOWN_STRATEGY");
  }

  uint32_t completed = obj["completed"];

  std::vector<Request> requests;
  for (auto req : obj["requests"]) {
    if (req["type"] == "CREATE_PROCESS") {
      requests.push_back(
          CreateProcessReq(req["pid"], req["bytes"], req["pages"]));
    } else if (req["type"] == "TERMINATE_PROCESS") {
      requests.push_back(TerminateProcessReq(req["pid"]));
    } else if (req["type"] == "ALLOCATE_MEMORY") {
      requests.push_back(
          AllocateMemory(req["pid"], req["bytes"], req["pages"]));
    } else if (req["type"] == "FREE_MEMORY") {
      requests.push_back(FreeMemory(req["pid"], req["address"]));
    } else {
      throw TaskException("UNKNOWN_REQUEST");
    }
  }

  std::vector<MemoryBlock> blocks, freeBlocks;
  for (auto block : obj["state"]["blocks"]) {
    blocks.emplace_back(block["pid"], block["address"], block["size"]);
  }
  for (auto blockObj : obj["state"]["free_blocks"]) {
    freeBlocks.emplace_back(blockObj["pid"], blockObj["address"],
                            blockObj["size"]);
  }

  return MemoryTask::create(strategy, completed, {blocks, freeBlocks},
                            requests);
}

/**
 *  @brief Создает объект задания "Диспетчеризация процессов" из JSON-объекта.
 *
 *  @param obj JSON-объект.
 *
 *  @return Объект задания.
 *
 *  @throws Utils::TaskException Исключение возникает в следующих случаях:
 *
 *  "UNKNOWN_STRATEGY" - неизвестный тип планировщика;
 *  "UNKNOWN_REQUEST" - неизвестный тип заявки;
 *  "UNKNOWN_PROCSTATE" - неизвестное состояние процесса.
 */
inline ProcessesTask loadProcessesTask(const nlohmann::json &obj) {
  using namespace ProcessesManagement;
  StrategyPtr strategy;

  auto strategyType = obj["strategy"];
  if (strategyType == "ROUNDROBIN") {
    strategy = RoundRobinStrategy::create();
  } else if (strategyType == "FCFS") {
    strategy = FcfsStrategy::create();
  } else if (strategyType == "SJN") {
    strategy = SjnStrategy::create();
  } else if (strategyType == "SRT") {
    strategy = SrtStrategy::create();
  } else if (strategyType == "WINNT") {
    strategy = WinNtStrategy::create();
  } else if (strategyType == "UNIX") {
    strategy = UnixStrategy::create();
  } else {
    throw TaskException("UNKNOWN_STRATEGY");
  }

  uint32_t completed = obj["completed"];

  std::vector<Request> requests;
  for (auto req : obj["requests"]) {
    if (req["type"] == "CREATE_PROCESS") {
      requests.push_back(CreateProcessReq(req["pid"], req["ppid"],
                                          req["priority"], req["basePriority"],
                                          req["timer"], req["workTime"]));
    } else if (req["type"] == "TERMINATE_PROCESS") {
      requests.push_back(TerminateProcessReq(req["pid"]));
    } else if (req["type"] == "INIT_IO") {
      requests.push_back(InitIO(req["pid"]));
    } else if (req["type"] == "TERMINATE_IO") {
      requests.push_back(TerminateIO(req["pid"], req["augment"]));
    } else if (req["type"] == "TRANSFER_CONTROL") {
      requests.push_back(TransferControl(req["pid"]));
    } else if (req["type"] == "TIME_QUANTUM_EXPIRED") {
      requests.push_back(TimeQuantumExpired());
    } else {
      throw TaskException("UNKNOWN_REQUEST");
    }
  }

  std::map<std::string, ProcState> stateMap = {
      {"ACTIVE", ProcState::ACTIVE},
      {"EXECUTING", ProcState::EXECUTING},
      {"WAITING", ProcState::WAITING}};

  std::vector<Process> processes;
  for (auto process : obj["state"]["processes"]) {
    if (stateMap.find(process["state"]) == stateMap.end()) {
      throw TaskException("UNKNOWN_PROCSTATE");
    }
    processes.emplace_back(Process{}
                               .pid(process["pid"])
                               .ppid(process["ppid"])
                               .priority(process["priority"])
                               .basePriority(process["basePriority"])
                               .timer(process["timer"])
                               .workTime(process["workTime"])
                               .state(stateMap[process["state"]]));
  }
  std::array<std::deque<int32_t>, 16> queues;
  for (size_t i = 0; i < queues.size(); ++i) {
    for (int32_t pid : obj["state"]["queues"][i]) {
      queues[i].push_back(pid);
    }
  }

  return ProcessesTask::create(strategy, completed, {processes, queues},
                               requests);
}
} // namespace Utils::details

namespace Utils {
/**
 *  @brief Загружает задания из файла.
 *
 *  @param is Дескриптор файла.
 *
 *  @return Массив из объектов заданий.
 *
 *  @throws Utils::TaskException Исключение возникает в
 *  следующих случаях:
 *
 *  "UNKNOWN_TASK" - неизвестный тип задания.
 */
inline std::vector<Task> loadTasks(std::istream &is) {
  nlohmann::json obj;
  is >> obj;
  std::vector<Task> tasks;

  for (auto task : obj) {
    if (task["type"] == "MEMORY_TASK") {
      tasks.emplace_back(details::loadMemoryTask(task));
    } else if (task["type"] == "PROCESSES_TASK") {
      tasks.emplace_back(details::loadProcessesTask(task));
    } else {
      throw TaskException("UNKNOWN_TASK");
    }
  }

  return tasks;
}

/**
 * @brief Сохраняет задания в файл.
 *
 * @param tasks Массив из объектов заданий.
 *
 * @param os Дескриптор файла.
 */
inline void saveTasks(const std::vector<Task> &tasks, std::ostream &os) {
  auto obj = nlohmann::json::array();

  for (Task task : tasks) {
    std::visit([&obj](const auto &task) { obj.push_back(task.dump()); }, task);
  }

  os << obj;
}
} // namespace Utils
