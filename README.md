# Трассировщик лучей на C++

В этом учебном проекте я реализовал загрузку OBJ/MTL, пересечения лучей со сферами и треугольниками, расчёт освещения и теней, рекурсивное отражение и преломление. Для отладки добавил режимы визуализации расстояний и нормалей.

## Запуск

Нужны CMake 3.20+ и компилятор с поддержкой C++20. Утилита сохраняет результат в PPM и не требует графических библиотек.

```sh
cmake -S . -B build
cmake --build build
./build/raytracer examples/demo.obj build/demo.ppm
./build/raytracer examples/demo.obj build/depth.ppm depth
./build/raytracer examples/demo.obj build/normals.ppm normals
ctest --test-dir build --output-on-failure
```

В папке `raytracer-geom` находятся геометрические примитивы, в `raytracer-reader` — чтение сцены и материалов, в `raytracer` — генерация лучей и рендеринг. Пример сцены и материалы лежат в `examples`. Камеру и размер кадра можно изменить в `main.cpp`.
