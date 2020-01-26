#pragma once

#include <cstdint>
#include <utility>

#include <QDialog>

#include <tl/optional.hpp>

namespace Ui {
class TerminateIoDialog;
}

class TerminateIoDialog : public QDialog {
  Q_OBJECT

public:
  ~TerminateIoDialog();

  static tl::optional<std::pair<int32_t, std::size_t>>
  getTerminateIoInfo(QWidget *parent, bool withAugment = false);

private:
  explicit TerminateIoDialog(QWidget *parent = nullptr,
                             bool withAugment = false);

  std::pair<int32_t, std::size_t> info;

  Ui::TerminateIoDialog *ui;

  void tryAccept();
};
