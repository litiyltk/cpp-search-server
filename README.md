# cpp-search-server  
**Высокопроизводительный поисковый сервер на C++, разработанный в рамках обучения на Яндекс Практикуме и активно дорабатываемый**  

## **Описание проекта**  
`cpp-search-server` — это поисковая система с очередью запросов и постраничной выдачей результатов, которая индексирует документы и позволяет выполнять фильтрацию результов с учетом **стоп-слов**, **минус-слов** и **ранжирования по** [**TF-IDF**](https://en.wikipedia.org/wiki/Tf%E2%80%93idf).

### **Функциональность**  
- **Индексация документов** с учетом стоп-слов (исключаются при поиске).  
- **Поиск документов** с поддержкой минус-слов (исключаются документы, содержащие минус-слова). 
- **Ранжирование результатов по TF-IDF**:  
  - **TF (Term Frequency)** — частота слова в документе.  
  - **IDF (Inverse Document Frequency)** — значимость слова в коллекции.  
  - **Оценка релевантности** — сумма произведений TF и IDF.  
  - **Сортировка** по убыванию релевантности, затем по рейтингу.  
- **Фильтрация результатов**
  - **по статусу** (ACTUAL, IRRELEVANT, BANNED, REMOVED).  
  - **при помощи пользовательских предикатов** (ID, рейтинг, статус).  
- **Очередь запросов** с логированием количества поисковых запросов без результатов.  
- **Постраничная выдача** результатов (вспомогательный класс `Paginator`).  
- **Тестирование функциональности** с использованием кастомного тестового фреймворка `tests/test_framework.h`.   

---

## **Требования**  
- **Операционная система**: Windows 10/11, Ubuntu 20.04 LTS  
- **Компилятор C++**:
  - Windows: **MinGW-w64 (GCC 14.2+)**
  - Linux: **GCC 10.5+**  
- **CMake 3.11+**  
- **(Для Windows)** MinGW Make

---

## **Сборка проекта**  

### **Debug-сборка (Windows, MinGW)**  
```sh
mkdir build_debug
cd build_debug
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### **Release-сборка  (Windows, MinGW)**
```sh
mkdir build_release
cd build_release
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### **Запуск программы**

После успешной сборки выполните команду:

```sh
./search-server  # Linux
.\search-server.exe  # Windows
```

### **Запуск тестов (доступны при Debug-сборке)**

После успешной Debug-сборки выполните команду:

```sh
./tests  # Linux
.\tests.exe  # Windows
```

## **Тестирование**

Проект содержит набор автотестов, которые проверяют:

- **Корректность индексации и поиска документов**
- **Ранжирование результатов по TF-IDF**
- **Обработку минус-слов и стоп-слов**
- **Фильтрацию результатов по статусу**
- **Работу очереди запросов**
- **Корректную работу постраничного вывода**

Дополнительно реализовано логирование времени выполнения тестов для анализа производительности (log_duration.h).

## **Стек технологий**
- **C++17**
- **STL** (`vector`, `unordered_map`, `unordered_set`, `deque`, `algorithm`, `numeric`)
- **Шаблонное программирование**
- **Итераторы и диапазоны**
- **Обработка исключений**
- **CMake**

## **Планы развития проекта**
- **Добавление тестового фреймворка Catch2**
- **Добавление тестов производительности**  
- **Реализация поддержки сложных поисковых запросов с операторами "И" (`&`) и "ИЛИ" (`|`)**   
- **Реализация GUI с использованием Qt**.
- **Интеграция библиотеки Boost для обработки строк при многопоточной обработке запросов** 
- **Перевод проекта на стандарт C++23** 