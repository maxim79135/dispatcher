#include <algorithm>

#include <QDebug>

#include <tl/optional.hpp>

#include <algo/memory/requests.h>
#include <algo/memory/types.h>

#include <qtutils/literals.h>

#include <listitems/memoryblockitem.h>

#include "dialogs/allocatememorydialog.h"
#include "dialogs/freememorydialog.h"
#include "dialogs/pidinputdialog.h"
#include "menus/memorytaskaddrequestmenu.h"

#include "memorytaskbuilder.h"
#include "ui_memorytaskbuilder.h"

using namespace MemoryManagement;
using namespace QtUtils::Literals;

MemoryTaskBuilder::MemoryTaskBuilder(const Utils::Task &task, QWidget *parent)
    : QWidget(parent), _task(task.get<Utils::MemoryTask>()),
      ui(new Ui::MemoryTaskBuilder) {
  ui->setupUi(this);

  connect(ui->requestsList,
          &QListWidget::currentRowChanged,
          this,
          &MemoryTaskBuilder::currentRequestChanged);
  connect(ui->requestsList,
          &QListWidget::customContextMenuRequested,
          this,
          &MemoryTaskBuilder::provideContextMenu);

  loadTask(_task);
  selectCurrentRequest(0);
}

MemoryTaskBuilder::~MemoryTaskBuilder() { delete ui; }

Utils::Task MemoryTaskBuilder::task() const { return _task; }

void MemoryTaskBuilder::loadTask(const Utils::MemoryTask &task) {
  auto state = MemoryState::initial();

  states.clear();
  for (const auto &req : task.requests()) {
    states.push_back(state);
    state = task.strategy()->processRequest(req, state);
  }
  setRequestsList(task.requests());
  setStrategy(task.strategy()->type);
}

void MemoryTaskBuilder::currentRequestChanged(int index) {
  auto indexu = static_cast<std::size_t>(index);
  if (index < 0 || indexu > _task.requests().size()) {
    return;
  }

  auto state = states[index];
  auto request = _task.requests().at(indexu);
  updateTaskView(state, request);
}

void MemoryTaskBuilder::processContextMenuAction(const QString &action,
                                                 int requestIndex) {
  auto requests = _task.requests();

  if (requestIndex != -1) {
    auto requestIndexu = static_cast<std::size_t>(requestIndex);
    auto request = requests.at(requestIndexu);

    if (action == RequestItemMenu::TO_TOP) {
      requests.erase(requests.begin() + requestIndex);
      requests.insert(requests.begin(), request);
      requestIndex = 0;
      requestIndexu = 0u;
    } else if (action == RequestItemMenu::TO_BOTTOM) {
      requests.erase(requests.begin() + requestIndex);
      requests.push_back(request);
      requestIndex = static_cast<int>(requests.size() - 1);
      requestIndexu = requests.size() - 1;
    } else if (action == RequestItemMenu::MOVE_UP && requestIndex > 0) {
      std::swap(requests.at(requestIndexu - 1), requests.at(requestIndexu));
      requestIndex--;
      requestIndexu--;
    } else if (action == RequestItemMenu::MOVE_DOWN &&
               requestIndexu < requests.size() - 1) {
      std::swap(requests.at(requestIndexu + 1), requests.at(requestIndexu));
      requestIndex++;
      requestIndexu++;
    }
  }

  if (auto request = processAddRequestMenuAction(action); request.has_value()) {
    requests.push_back(*request);
  }

  _task = Utils::MemoryTask::create(
      _task.strategy(), 0, MemoryState::initial(), requests);
  loadTask(_task);

  if (requestIndex == -1) {
    selectCurrentRequest(static_cast<int>(requests.size() - 1));
  } else {
    selectCurrentRequest(requestIndex);
  }
}

tl::optional<Request>
MemoryTaskBuilder::processAddRequestMenuAction(const QString &action) {
  if (action == MemoryTaskAddRequestMenu::CREATE_PROCESS) {
    auto info = AllocateMemoryDialog::getMemoryBlockInfo(this, 256);

    if (info) {
      auto [pid, pages] = *info;
      return CreateProcessReq(pid, pages * 4096 - 1);
    }
  } else if (action == MemoryTaskAddRequestMenu::TERMINATE_PROCESS) {
    if (auto pid = PidInputDialog::getPid(this); pid.has_value()) {
      return TerminateProcessReq(pid.value());
    }
  } else if (action == MemoryTaskAddRequestMenu::ALLOCATE_MEMORY) {
    auto info = AllocateMemoryDialog::getMemoryBlockInfo(this, 256);

    if (info) {
      auto [pid, pages] = *info;
      return AllocateMemory(pid, pages * 4096 - 1);
    }
  } else if (action == MemoryTaskAddRequestMenu::FREE_MEMORY) {
    auto info = FreeMemoryDialog::getFreeMemoryInfo(this);

    if (info) {
      auto [pid, address] = *info;
      return FreeMemory(pid, address);
    }
  }

  return tl::nullopt;
}

void MemoryTaskBuilder::selectCurrentRequest(int requestIndex) {
  ui->requestsList->setCurrentRow(requestIndex);
}

void MemoryTaskBuilder::updateTaskView(const MemoryState &state,
                                       const Request &request) {
  setMemoryState(state);
  setRequest(request);
}

void MemoryTaskBuilder::provideContextMenu(const QPoint &pos) {
  qDebug() << "ContextMenu";

  auto globalPos = ui->requestsList->mapToGlobal(pos);
  auto row = ui->requestsList->indexAt(pos).row();

  qDebug() << "ContextMenu:" << row;

  MemoryTaskAddRequestMenu menu(row != -1);

  auto action = menu.exec(globalPos);
  if (!action) {
    return;
  }

  processContextMenuAction(action->text(), row);
}

void MemoryTaskBuilder::setMemoryState(const MemoryState &state) {
  QListWidget *list;

  list = ui->listMemBlocks;
  list->clear();

  for (const auto &block : state.blocks) {
    list->addItem(new MemoryBlockItem(block, true));
  }

  list = ui->listFreeBlocks;
  list->clear();

  for (const auto &block : state.freeBlocks) {
    list->addItem(new MemoryBlockItem(block, true));
  }
}

void MemoryTaskBuilder::setRequest(const Request &request) {
  auto text = request.match(
      [](const CreateProcessReq &req) {
        return "Создан новый процесс PID = %1. "
               "Для размещения процесса в памяти (включая "
               "служебную информацию) требуется выделить "
               "%2 байт (%3 параграфов)"_qs.arg(req.pid())
                   .arg(req.bytes())
                   .arg(req.pages());
      },
      [](const TerminateProcessReq &req) {
        return "Процесс PID = %1 завершен"_qs.arg(req.pid());
      },
      [](const AllocateMemory &req) {
        return "Процесс PID = %1 выдал запрос на выделение ему "
               "%2 байт (%3 параграфов) оперативной памяти"_qs.arg(req.pid())
                   .arg(req.bytes())
                   .arg(req.pages());
      },
      [](const FreeMemory &req) {
        return "Процесс PID = %1 выдал запрос на освобождение "
               "блока памяти с адресом %2"_qs.arg(req.pid())
                   .arg(req.address());
      });
  ui->labelRequestDescr->setText(text);
}

void MemoryTaskBuilder::setRequestsList(const std::vector<Request> &requests) {
  auto *list = ui->requestsList;
  list->clear();
  for (const auto &request : requests) {
    list->addItem(request.match(
        [](const CreateProcessReq &) { return "Создать процесс"; },
        [](const TerminateProcessReq &) { return "Завершить процесс"; },
        [](const AllocateMemory &) { return "Выделить память"; },
        [](const FreeMemory &) { return "Освободить память"; }));
  }
}

void MemoryTaskBuilder::setStrategy(StrategyType type) {
  auto *label = ui->strategyLabel;

  if (type == StrategyType::FIRST_APPROPRIATE) {
    label->setText("Стратегия: первый подходящий");
  } else if (type == StrategyType::MOST_APPROPRIATE) {
    label->setText("Стратегия: наиболее подходящий");
  } else if (type == StrategyType::LEAST_APPROPRIATE) {
    label->setText("Стратегия: наименее подходящий");
  }
}
