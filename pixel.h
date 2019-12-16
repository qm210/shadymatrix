#ifndef PIXEL_H
#define PIXEL_H

struct vec2;

class LED;

class Pixel
{
    public:
        float x = 0;
        float y = 0;
        LED L = LED(1, 1);
        int segcount = -1;
        
    Pixel() {}
    
    Pixel(float x, float y) : x(x), y(y) {}
    
    Pixel(vec2 v) {x = v.x; y = v.y;}

    Pixel(vec2 v, int count) {x = v.x; y = v.y; segcount = count;}
    
    Pixel(float x, float y, LED L) : x(x), y(y), L(L) {}
    
    void set(LED newL)
    {
        L = newL;
    }
    
    vec2 get_coord() {return vec2(x, y);}
};

#endif
