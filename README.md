# Модель диспетчера задач операционной системы

[![Linux & macOS Build Status](https://img.shields.io/travis/alirzaev/dispatcher?label=Linux%20%26%20macOS%20Build%20Status)](https://travis-ci.org/alirzaev/dispatcher)
[![Windows Build Status](https://img.shields.io/appveyor/ci/alirzaev/dispatcher?label=Windows%20Build%20Status)](https://ci.appveyor.com/project/alirzaev/dispatcher)

Программная модель диспетчера задач для лабораторной работы по дисциплине "Операционные системы"

Руководство пользователя: https://alirzaev1996.gitbook.io/model-dispetchera-zadach-os/

# Структура проекта

- dispatcher.pro - Файл основного проекта

  - schedulers/schedulers.pro - Библиотека с алгоритмами работы диспетчера задач

  - tests/tests.pro - Тесты

  - generator/generator.pro - Библиотека для генерации заданий

  - gui/gui.pro - Программная модель с графическим интерфейсом

# Файл задания

Структуру файла задания можно найти [здесь](docs/TASK.md)

# Сборка

## Необходимые компоненты

- Qt 5.11 или новее

- Visual Studio 2017 или новее со следующими компонентами:
  
  - Разработка классических приложений на C++

  - Debugging Tools for Windows

## Используемые сторонние библиотеки

- [Catch2 2.4.1](https://github.com/catchorg/Catch2)
- [nlohmann::json 3.6.1](https://github.com/nlohmann/json)

## Сборка

Проект собирается штатными средствами Qt: либо открываем `dispatcher.pro` через QtCreator, либо 
собираем с помощью qmake:

```
mkdir build
cd build
qmake ../dispatcher.pro
make
```

# В случае возникновения проблем

Задания, которые генерируются самой программой, сохраняются в файл во временной папке пользователя: 
`C:\Users\%USERNAME%\AppData\Local\Temp\dispatcher.json`. При составлении 
[issue](https://github.com/alirzaev/dispatcher/issues), пожалуйста, прикрепите этот файл тоже.