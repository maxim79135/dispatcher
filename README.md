# Модель диспетчера задач операционной системы

Программная модель диспетчера задач для лабораторной работы по дисциплине "Операционные системы"

Руководство пользователя: [HTML](https://alirzaev.github.io/dispatcher/user-manual), [PDF](https://alirzaev.github.io/dispatcher/user-manual.pdf)

Загрузки: https://github.com/alirzaev/dispatcher/releases/latest

# Структура проекта

- qtutils - Библиотека со вспомогательными функциями

- schedulers - Библиотека с алгоритмами работы диспетчера задач

- tests - Тесты

- generator - Библиотека для генерации заданий

- dispatcher - Программная модель с графическим интерфейсом

- taskbuilder - Конструктор заданий

- widgets - Библиотека с UI-компонентами

# Файл задания

Структуру файла задания можно найти [здесь](docs/TASK.md)

# Сборка

## Необходимые компоненты

### Windows

- CMake 3.10 или новее

- Qt 5.11 или новее

- Visual Studio 2017 или новее со следующими компонентами:

  - Разработка классических приложений на C++

  - Debugging Tools for Windows

### Ubuntu

- Минимальная версия Ubuntu - 18.04

- CMake 3.10 или новее

- Qt 5.11 или новее

- g++ 7 или новее

## Используемые сторонние библиотеки

- [Catch2 2.4.1](https://github.com/catchorg/Catch2)
- [nlohmann::json 3.6.1](https://github.com/nlohmann/json)
- [Mapbox Variant 1.1.6](https://github.com/mapbox/variant)
- [TartanLlama optional 1.0.0](https://github.com/TartanLlama/optional)

## Используемые сторонние шрифты

- [Microsoft Cascadia Code 1911.21](https://github.com/microsoft/cascadia-code)

## Сборка

Проект собирается штатными средствами Qt: либо открываем `CMkaeLists.txt` через QtCreator, либо
собираем с помощью CMake:

```
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="<Qt root dir>/lib/cmake" -DDISPATCHER_DEBUG=1 ..
make
```

`DISPATCHER_DEBUG=1` - включение дополнительной отладочной информации.

# В случае возникновения проблем

Задания, которые генерируются самой программой, сохраняются в файл `dispatcher.json` во временной папке пользователя.
Открыть данную папку в стандартном файловом менеджере можно через меню программной модели:
"Справка" > "Устранение неполадок" > "Открыть временную папку".
При составлении [issue](https://github.com/alirzaev/dispatcher/issues), пожалуйста, прикрепите этот файл тоже.
