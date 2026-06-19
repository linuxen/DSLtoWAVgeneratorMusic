# 🎵 WAV Generator

**WAV Generator** — это небольшой C++ генератор музыки, который превращает текстовое описание трека в настоящий `.wav` файл.

Пишешь музыку в простом DSL-формате — получаешь готовый звук.

## ✨ Возможности

* 🎼 чтение треков из `.txt` файлов;
* 🥁 поддержка сэмплов через `sampler`;
* 🎹 генерация звука через `sine`, `triangle`, `square`;
* 🔊 эффекты `gain`, `echo`, `tremolo`;
* ⏱️ поддержка BPM, паттернов и resolution;
* 🎚️ velocity, attack/release и loop-сэмплы;
* 💿 экспорт в mono WAV 44.1 kHz / 16-bit.

## 🛠️ Сборка

```bash
cmake -S . -B build
cmake --build build
```

## 🚀 Запуск

```bash
./build/bin/decoder examples/lick.txt lick.wav
```

Общий формат:

```bash
./build/bin/decoder <input.txt> <output.wav>
```

## 🎧 Примеры

В папке `examples/` уже есть готовые треки:

```bash
./build/bin/decoder examples/memory.txt memory.wav
./build/bin/decoder examples/sweden.txt sweden.wav
```

## 📁 Структура проекта

```text
bin/        запуск программы
lib/        парсер, рендеринг, WAV и обработка звука
samples/    WAV-сэмплы для инструментов
examples/   примеры треков
```

## 🌟 Идея проекта

Проект показывает, как из обычного текста можно собрать музыку: распарсить ноты, применить инструменты и эффекты, а затем сохранить результат в WAV.
