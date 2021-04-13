# Построение векторного поля на неструктурированной сетке при помощи линий тока

## Идея алгоритма
Для расчета значения векторного поля в каждой точке линии тока используется трилинейная интерполяция.
Для определения положения следующей точки линии тока используется алгоритм Рунге-Кутты 4-го порядка.

## Инструменты
* CMake - для сборки проекта
* VTK@9 - для визуализации сеток и полученных линий тока

## Входные данные
Построение происходит на неструктурированных сетках в формате .vtu с определенными векторными значениями Velocity в каждом узле сетки. 

`P.s. Сетки достаточно тяжелые, поэтому если вам нужны исходные данные, вы всегда можете написать мне на почту guvictory@yandex.ru`

## Результат работы программы

### Просто визуализация сетки
![1](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/1.png)
### Визуализация сетки с построенными линиями тока
![2](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/2.png)
### Визуализация сетки и сетки с построенными линиями тока
![4](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/4.png)
### Визуализация одиной сетки в разные моменты времени
![3](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/3.png)
![5](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/5.png)
### Визуализация объединенной сетки
![6](https://github.com/GuVictory/VectorFlowVisualization3D_streamlines/blob/main/img/6.png)