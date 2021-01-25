**Problem inf06-1: asm-x86/fpu-sse/trig-macloren-series**
============================================================

Реализуйте на языке ассемблера x86 (IA-32) или x86-64 функцию с сигнатурой:

    extern double my_sin(double x);

которая вычисляет значение sin(x).

Запрещено использовать встроенные тригонометрические инструкции.

    Для вычислений используйте известный вам из курса Математического анализа способ разложения функции в ряд. Точность результата должна быть маскимально возможной для типа данных double.

***