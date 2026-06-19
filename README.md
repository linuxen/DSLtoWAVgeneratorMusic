# ITMO Loops WAV Generator

Небольшая C++ программа, которая читает музыкальное описание в формате ITMO Loops DSL и генерирует `.wav` файл.

## Что умеет

* читает треки из `.txt`;
* поддерживает инструменты `sampler`, `sine`, `triangle`, `square`;
* поддерживает эффекты `gain`, `echo`, `tremolo`;
* генерирует mono WAV 44.1 kHz / 16-bit;
* поддерживает паттерны, BPM, velocity, attack/release и loop-сэмплы.

## Сборка

```bash
cmake -S . -B build
cmake --build build
```

## Запуск

```bash
./build/bin/itmoloops examples/lick.txt lick.wav
```

Формат команды:

```bash
./build/bin/itmoloops <input.txt> <output.wav>
```

## Примеры

Готовые треки лежат в папке `examples/`:

```bash
./build/bin/itmoloops examples/memory.txt memory.wav
./build/bin/itmoloops examples/sweden.txt sweden.wav
```

## Структура проекта

```text
bin/        точка входа программы
lib/        парсер, рендер, WAV-запись и обработка звука
samples/    WAV-сэмплы для sampler-инструментов
examples/   примеры треков
```
