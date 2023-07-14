#pragma once

class Color {
public:
    unsigned char r;  // Canal rojo
    unsigned char g;  // Canal verde
    unsigned char b;  // Canal azul

    // Constructor por defecto
    Color() : r(255), g(255), b(255) {}

    // Constructor con parámetros
    Color(unsigned char red, unsigned char green, unsigned char blue)
            : r(red), g(green), b(blue) {}
};
