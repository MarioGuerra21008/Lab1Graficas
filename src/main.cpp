#include <iostream>
#include <fstream>
#include <vector>

// Color struct to represent RGB values
struct Color
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;

    Color(unsigned char r, unsigned char g, unsigned char b)
            : red(r), green(g), blue(b)
    {
    }
};

struct Vertex
{
    int x;
    int y;

    Vertex(int px, int py) : x(px), y(py)
    {
    }
};


// Function to write BMP image file
void renderBuffer(const std::string& filename, int width, int height, unsigned char* framebuffer)
{
    std::ofstream file(filename, std::ios::binary);

    // BMP file header
    unsigned char fileHeader[] = {
            'B', 'M',  // Signature
            0, 0, 0, 0, // File size (will be filled later)
            0, 0,      // Reserved
            0, 0,      // Reserved
            54, 0, 0, 0 // Pixel data offset
    };

    // BMP info header
    unsigned char infoHeader[] = {
            40, 0, 0, 0,  // Info header size
            0, 0, 0, 0,   // Image width (will be filled later)
            0, 0, 0, 0,   // Image height (will be filled later)
            1, 0,         // Number of color planes
            24, 0,        // Bits per pixel (3 bytes)
            0, 0, 0, 0,   // Compression method
            0, 0, 0, 0,   // Image size (will be filled later)
            0, 0, 0, 0,   // Horizontal resolution (pixels per meter, not used)
            0, 0, 0, 0,   // Vertical resolution (pixels per meter, not used)
            0, 0, 0, 0,   // Number of colors in the palette (not used)
            0, 0, 0, 0    // Number of important colors (not used)
    };

    // Calculate some values
    int imageSize = width * height * 3;  // 3 bytes per pixel (BGR)
    int fileSize = imageSize + sizeof(fileHeader) + sizeof(infoHeader);

    // Fill in the file header
    *(int*)&fileHeader[2] = fileSize;          // File size
    *(int*)&fileHeader[10] = sizeof(fileHeader) + sizeof(infoHeader);  // Pixel data offset

    // Fill in the info header
    *(int*)&infoHeader[4] = width;             // Image width
    *(int*)&infoHeader[8] = height;            // Image height
    *(int*)&infoHeader[20] = imageSize;        // Image size

    // Write the headers to the file
    file.write(reinterpret_cast<char*>(fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<char*>(infoHeader), sizeof(infoHeader));

    // Write the pixel data
    file.write(reinterpret_cast<char*>(framebuffer), imageSize);

    // Close the file
    file.close();
}

void clear(unsigned char* framebuffer, int width, int height, const Color& color)
{
    for (int i = 0; i < width * height * 3; i += 3)
    {
        framebuffer[i] = color.blue;
        framebuffer[i + 1] = color.green;
        framebuffer[i + 2] = color.red;
    }
}

void point(unsigned char* framebuffer, int width, int height, int x, int y, const Color& color)
{
    int pixelIndex = (y * width + x) * 3;
    framebuffer[pixelIndex] = color.blue;
    framebuffer[pixelIndex + 1] = color.green;
    framebuffer[pixelIndex + 2] = color.red;
}

void line(unsigned char* framebuffer, int width, int height, const Vertex& start, const Vertex& end, const Color& color)
{
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        point(framebuffer, width, height, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int err2 = 2 * err;

        if (err2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (err2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void drawPolygon(unsigned char* framebuffer, int width, int height, const std::vector<Vertex>& points, const Color& color)
{
    // Verifica si el número de puntos es válido para dibujar un polígono
    if (points.size() < 2)
    {
        std::cout << "Error: Se requieren al menos dos puntos para dibujar un polígono." << std::endl;
        return;
    }

    // Dibuja líneas entre los puntos consecutivos del polígono
    for (size_t i = 0; i < points.size() - 1; ++i)
    {
        line(framebuffer, width, height, points[i], points[i + 1], color);
    }

    // Dibuja una línea desde el último punto hasta el primer punto
    line(framebuffer, width, height, points[points.size() - 1], points[0], color);
}

bool isInsidePolygon(int x, int y, const std::vector<Vertex>& vertices)
{
    int intersectCount = 0;
    size_t vertexCount = vertices.size();

    for (size_t i = 0; i < vertexCount; ++i)
    {
        const Vertex& v1 = vertices[i];
        const Vertex& v2 = vertices[(i + 1) % vertexCount];

        if (((v1.y > y) != (v2.y > y)) && (x < (v2.x - v1.x) * (y - v1.y) / (v2.y - v1.y) + v1.x))
        {
            intersectCount++;
        }
    }

    return (intersectCount % 2 == 1);
}

void fillPolygon(unsigned char* framebuffer, int width, int height, const std::vector<Vertex>& vertices, const Color& color)
{
    // Verifica si el número de vértices es válido para rellenar un polígono
    if (vertices.size() < 3)
    {
        std::cout << "Error: Se requieren al menos tres vértices para rellenar un polígono." << std::endl;
        return;
    }

    // Encuentra los valores mínimo y máximo en X y Y
    int minX = width;
    int minY = height;
    int maxX = 0;
    int maxY = 0;

    for (const Vertex& vertex : vertices)
    {
        if (vertex.x < minX)
            minX = vertex.x;
        if (vertex.x > maxX)
            maxX = vertex.x;
        if (vertex.y < minY)
            minY = vertex.y;
        if (vertex.y > maxY)
            maxY = vertex.y;
    }

    // Itera sobre cada píxel en el rectángulo delimitado por los valores mínimo y máximo
    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            // Verifica si el punto (x, y) está dentro del polígono
            if (isInsidePolygon(x, y, vertices))
            {
                point(framebuffer, width, height, x, y, color);
            }
        }
    }
}

void render(unsigned char* framebuffer, int width, int height)
{
    Vertex polygonPoints[] = { {165, 380}, {185, 360}, {180, 330}, {207, 345}, {233, 330}, {230, 360}, {250, 380}, {220, 385}, {205, 410}, {193, 383} };
    std::vector<Vertex> polygonVertices(polygonPoints, polygonPoints + sizeof(polygonPoints) / sizeof(polygonPoints[0]));
    drawPolygon(framebuffer, width, height, polygonVertices, Color(0, 0, 0));
    fillPolygon(framebuffer, width, height, polygonVertices, Color(255, 255, 0));
}

int main()
{
    const int width = 800;
    const int height = 600;

    // Create an array to hold the pixel data (framebuffer)
    unsigned char* framebuffer = new unsigned char[width * height * 3];

    // Clear the framebuffer to white (255, 255, 255)
    clear(framebuffer, width, height, Color(0, 0, 0));

    // Calculate the center coordinates
    int centerX = width / 2;
    int centerY = height / 2;

    Vertex start(100, 100);
    Vertex end(700, 400);

    //line(framebuffer, width, height, start, end, Color(0, 0, 255));

    render(framebuffer, width, height);

    // Write the image to a BMP file
    renderBuffer("polygon1.bmp", width, height, framebuffer);

    // Clean up the memory
    delete[] framebuffer;

    std::cout << "Image generated successfully!" << std::endl;

    return 0;
}
