#include <stdio.h>
#include <cmath>
#include <cstring>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <vector>
#include "LED_WS.h"
#include "pixel.h"
#include "helper.h"

#define PI 3.141592
#define numpix P.size()

const float distance_LED_in_cm = 100. / 60.;
const float distance_LED_in_m = 0.01 * distance_LED_in_cm;

const float originOffsetX = 16;
const float originOffsetY = 16;

struct Segment
{
    float origin_x;
    float origin_y;
    float direction; // 0 to 360 degrees
    int pixels;
    int type = 0;
    Segment(float x, float y, float to_x, float to_y) {set(x, y, to_x, to_y);}
    Segment(float x, float y, float to_x, float to_y, int _type) {set(x, y, to_x, to_y); type = _type;}
    void set(float x, float y, float to_x, float to_y)
    {
        x += originOffsetX;
        y += originOffsetY;
        to_x += originOffsetX;
        to_y += originOffsetY;
        set_origin(x, y);
        set_direction_to(to_x, to_y);
        set_length_to(to_x, to_y);
    }
    void set_origin(float x, float y) {origin_x = x; origin_y = y;}
    void set_direction(float dir) {direction = fmod(dir + 360., 360.);}
    void set_direction_to(float to_x, float to_y) {set_direction(180./PI*atan2(origin_y - to_y, to_x - origin_x));}
    void set_length(float l) {pixels = (int)(round((l + 1) / distance_LED_in_cm));}
    void set_length_to(float to_x, float to_y) {set_length(sqrt(pow(to_x - origin_x, 2) + pow(to_y - origin_y, 2)));}
    float get_length() {return pixels * distance_LED_in_cm;}
    vec2 get_pixel(int i)
    {
        return vec2(
            origin_x + i * distance_LED_in_cm * cos(direction * PI/180.),
            origin_y - i * distance_LED_in_cm * sin(direction * PI/180.)
        );
    }
    vec2 get_last_pixel_plus_one() {return get_pixel(pixels);}
    vec2 get_last_pixel() {return get_pixel(pixels - 1);}
    float get_rightmost_x() {return direction > 90 && direction < 270 ? origin_x : get_last_pixel_plus_one().x;}
    float get_bottommost_y() {return direction < 180 ? origin_y : get_last_pixel_plus_one().y;}

    void move_horizontally(float inc)
    {
        origin_x += inc;
    }
    void move_vertically(float inc)
    {
        origin_y += inc;
    }
    void turn(float inc)
    {
        set_direction(direction + inc);
    }
    void change_length(int inc)
    {
        pixels = max(pixels + inc, 1);
    }
    void flip()
    {
        set_origin(get_last_pixel_plus_one().x, get_last_pixel_plus_one().y);
        set_direction(direction + 180.);
    }
};

int count_LEDs_in_cross_matrix(int, int);
void init_pattern();
void proceed_pattern(float time);
LED shader(float time, vec2 coord, int pixel, int segment, int type);
void print_all_segments(std::vector<Segment> segments);

bool debug = false;

int main(int argc, char* argv[])
{
    SDL_Event e;

    std::vector<std::vector<Segment>> figures;
    std::vector<Segment> segments;
    std::vector<Pixel> P;

    const float D = distance_LED_in_cm;
    const float N = 80;
    const float A = N * D;
    const float B = A - D;

    float width = 32;
    float height = 32;
    float margin = 0;
    bool circleShape = false;
    bool helperLines = false;
    bool blurLights = true;
    bool drawStripRectangle = false;
    float stripWidth = 1.0;

    int selected_segment = 0;
    int selected_figure = 0;

    if (argc > 1)
    {
        selected_figure = (int)argv[1][0];
    }

    /// PATTERN

    segments.clear();
    for (float y = 0; y < height; y += distance_LED_in_cm)
    {
        segments.push_back(Segment(-originOffsetX, y - originOffsetY, width - originOffsetX, y - originOffsetY));
    }

    figures.clear();
    figures.push_back(segments);

    segments = figures[selected_figure];

    /// END PATTERN

    float meters_required = numpix * distance_LED_in_m;
    printf("Pixels: %i\nMeters: %g\nWidth: %g\nHeight: %g\n", numpix, meters_required, width, height);

    const float SCALE = N * .3;
    const float MARGIN = margin * SCALE;
    const float WIDTH = width * SCALE;
    const float HEIGHT = height * SCALE;
    const float LEDSIZE = .125 * N;

    if (SDL_Init(SDL_INIT_VIDEO))
    {
        printf ("SDL_Init Error: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SDL2_gfx test", 800, 100, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
    if (window == NULL)
    {
        printf ("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf ("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 3;
    }

    SDL_Rect margin_t, margin_b, margin_l, margin_r;
    margin_t.x = 0;
    margin_t.y = 0;
    margin_b.x = 0;
    margin_b.y = HEIGHT - MARGIN;
    margin_l.x = 0;
    margin_l.y = 0;
    margin_r.x = WIDTH - MARGIN;
    margin_r.y = 0;
    margin_t.w = margin_b.w = WIDTH;
    margin_t.h = margin_b.h = MARGIN;
    margin_l.w = margin_r.w = MARGIN;
    margin_l.h = margin_r.h = HEIGHT;

    float new_origin_x, new_origin_y, new_to_x, new_to_y;

    int quit = 0;
    long time = 0;
    bool firstClick = true;

    init_pattern();

    while (!quit)
    {
        if (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_MOUSEBUTTONDOWN:
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (firstClick)
                    {
                        new_origin_x = x / SCALE;
                        new_origin_y = y / SCALE;
                        printf("(%g, %g, ...\n", new_origin_x, new_origin_y);
                        firstClick = false;
                    }
                    else
                    {
                        new_to_x = x / SCALE;
                        new_to_y = y / SCALE;
                        Segment new_seg = Segment(new_origin_x, new_origin_y, new_to_x, new_to_y);
                        segments.push_back(new_seg);
                        for (int p = 0; p < new_seg.pixels; p++)
                        {
                            P.push_back(Pixel(new_seg.get_pixel(p), segments.size() - 1));
                        }
                        meters_required = numpix * distance_LED_in_m;
                        printf("(%g, %g, %g, %g) Pixels: %i, Meters: %g\n", new_origin_x, new_origin_y, new_to_x, new_to_y, numpix, meters_required);
                        firstClick = true;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_BACKSPACE:
                        {
                            Segment last_seg = segments.back();
                            segments.pop_back();
                            for (int p = 0; p < last_seg.pixels; p++)
                            {
                                P.pop_back();
                            }
                            break;
                        }
                        case SDLK_SPACE:
                            segments.clear();
                            P.clear();
                            printf("erased everything.\n");
                            break;
                        /*
                        case SDLK_LEFT:
                            P.pop_back();
                            break;
                        */

                        case SDLK_w:
                            segments[selected_segment].move_vertically(-0.1);
                            break;
                        case SDLK_s:
                            segments[selected_segment].move_vertically(+0.1);
                            break;
                        case SDLK_d:
                            segments[selected_segment].move_horizontally(+0.1);
                            break;
                        case SDLK_a:
                            segments[selected_segment].move_horizontally(-0.1);
                            break;
                        case SDLK_e:
                            segments[selected_segment].turn(-1);
                            break;
                        case SDLK_q:
                            segments[selected_segment].turn(+1);
                            break;
                        case SDLK_r:
                            segments[selected_segment].change_length(1);
                            break;
                        case SDLK_f:
                            segments[selected_segment].change_length(-1);
                            break;
                        case SDLK_t:
                            segments[selected_segment].flip();
                            break;
                        case SDLK_x:
                            if (selected_segment == segments.size() - 1)
                            {
                                std::swap(segments.front(), segments.back());
                                selected_segment = 0;
                            }
                            else
                            {
                                std::swap(segments[selected_segment], segments[selected_segment + 1]);
                                selected_segment++;
                            }
                            print_all_segments(segments);
                            break;
                        case SDLK_y:
                            if (selected_segment == 0)
                            {
                                std::swap(segments.front(), segments.back());
                                selected_segment = segments.size() - 1;
                            }
                            else
                            {
                                std::swap(segments[selected_segment], segments[selected_segment - 1]);
                                selected_segment--;
                            }
                            print_all_segments(segments);
                            break;

                        case SDLK_UP:
                            selected_segment = (selected_segment + 1) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x - originOffsetX, segments[selected_segment].origin_y - originOffsetY,
                                                                segments[selected_segment].get_last_pixel_plus_one().x - originOffsetX, segments[selected_segment].get_last_pixel_plus_one().y - originOffsetY,
                                                                width - segments[selected_segment].origin_x - originOffsetX, height - segments[selected_segment].origin_y - originOffsetY,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x - originOffsetX, height - segments[selected_segment].get_last_pixel_plus_one().y - originOffsetY);
                            break;
                        case SDLK_DOWN:
                            selected_segment = (selected_segment - 1 + segments.size()) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x - originOffsetX, segments[selected_segment].origin_y - originOffsetY,
                                                                segments[selected_segment].get_last_pixel_plus_one().x - originOffsetX, segments[selected_segment].get_last_pixel_plus_one().y - originOffsetY,
                                                                width - segments[selected_segment].origin_x - originOffsetX, height - segments[selected_segment].origin_y - originOffsetY,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x - originOffsetX, height - segments[selected_segment].get_last_pixel_plus_one().y - originOffsetY);
                            break;

                        case SDLK_ESCAPE:
                            quit = 1;
                            break;

                        case SDLK_RETURN:
                            print_all_segments(segments);
                            break;
                    }
                    break;

                case SDL_QUIT:
                    quit = 1;
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        //////////// MARGIN /////////////

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderFillRect(renderer, &margin_t);
        SDL_RenderFillRect(renderer, &margin_b);
        SDL_RenderFillRect(renderer, &margin_l);
        SDL_RenderFillRect(renderer, &margin_r);

        ///////////// OUTLINE ///////////

        if (circleShape)
        {
            filledCircleColor(renderer, .5 * WIDTH, .5 * HEIGHT, .5 * WIDTH, RGB(255, 0, 0) );
            filledCircleColor(renderer, .5 * WIDTH, .5 * HEIGHT, .5 * WIDTH - 2, RGB(0, 0, 0) );
        }
        if (helperLines)
        {
            filledCircleColor(renderer, .5 * WIDTH, .5 * HEIGHT, .25 * WIDTH, RGB(255, 0, 0) );
            filledCircleColor(renderer, .5 * WIDTH, .5 * HEIGHT, .25 * WIDTH - 2, RGB(0, 0, 0) );
            SDL_SetRenderDrawColor(renderer, 250, 0, 0, 255);
            SDL_RenderDrawLine(renderer, .25 * WIDTH, 0, .25 * WIDTH, HEIGHT);
            SDL_RenderDrawLine(renderer, .50 * WIDTH, 0, .50 * WIDTH, HEIGHT);
            SDL_RenderDrawLine(renderer, .75 * WIDTH, 0, .75 * WIDTH, HEIGHT);
            SDL_RenderDrawLine(renderer, 0, .25 * HEIGHT, WIDTH, .25 * HEIGHT);
            SDL_RenderDrawLine(renderer, 0, .50 * HEIGHT, WIDTH, .50 * HEIGHT);
            SDL_RenderDrawLine(renderer, 0, .75 * HEIGHT, WIDTH, .75 * HEIGHT);
        }

        //////////// PATTERN ////////////
        P.clear();
        for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
        {
            int segcount = iseg - segments.begin();
            for (int p = 0; p < iseg->pixels; p++)
            {
                Pixel pixel = Pixel(iseg->get_pixel(p), segcount);
                vec2 relative_coord = vec2(pixel.x/width, pixel.y/height);
                pixel.L = shader(time, relative_coord, numpix, segcount - selected_segment, iseg->type);
                P.push_back(pixel);
            }
        }

        //////////// LIGHTS ////////////
        for(int p = 0; p < numpix; p++)
        {
            if(numpix < 1000 && blurLights)
            {
                for (int i=0; i<4; i++)
                {
                    float step = 1 + .25 * i;
                    float intensity = pow(.5, i);
                    filledCircleColor(renderer, SCALE * P[p].x, SCALE * P[p].y, LEDSIZE * step, LEDColor(P[p].L, intensity));
                }
            }
        }

        //////// STRIP RECTANGLE //////////
        if (drawStripRectangle)
        {
            float L2 = .5 * distance_LED_in_cm;
            float H2 = .5 * stripWidth;
            for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
            {
                if (iseg - segments.begin() == selected_segment)
                {
                    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 200, 128);
                }

                float ca = cos(iseg->direction * PI / 180.);
                float sa = sin(iseg->direction * PI / 180.);
                float corner1x = iseg->origin_x - ca * L2 + sa * H2;
                float corner1y = iseg->origin_y + sa * L2 + ca * H2;
                float corner2x = iseg->origin_x - ca * L2 - sa * H2;
                float corner2y = iseg->origin_y + sa * L2 - ca * H2;
                float corner4x = iseg->get_last_pixel().x + ca * L2 + sa * H2;
                float corner4y = iseg->get_last_pixel().y - sa * L2 + ca * H2;
                float corner3x = iseg->get_last_pixel().x + ca * L2 - sa * H2;
                float corner3y = iseg->get_last_pixel().y - sa * L2 - ca * H2;
                SDL_RenderDrawLine(renderer, SCALE * corner1x, SCALE * corner1y - 1, SCALE * corner2x, SCALE * corner2y - 1);
                SDL_RenderDrawLine(renderer, SCALE * corner2x, SCALE * corner2y - 1, SCALE * corner3x, SCALE * corner3y - 1);
                SDL_RenderDrawLine(renderer, SCALE * corner3x, SCALE * corner3y - 1, SCALE * corner4x, SCALE * corner4y - 1);
                SDL_RenderDrawLine(renderer, SCALE * corner4x, SCALE * corner4y - 1, SCALE * corner1x, SCALE * corner1y - 1);
                //SDL_RenderDrawLine(renderer, SCALE * corner1x + 1, SCALE * corner1y, SCALE * corner2x + 1, SCALE * corner2y);
                SDL_RenderDrawLine(renderer, SCALE * corner2x + 1, SCALE * corner2y, SCALE * corner3x + 1, SCALE * corner3y);
                SDL_RenderDrawLine(renderer, SCALE * corner3x + 1, SCALE * corner3y, SCALE * corner4x + 1, SCALE * corner4y);
                SDL_RenderDrawLine(renderer, SCALE * corner4x + 1, SCALE * corner4y, SCALE * corner1x + 1, SCALE * corner1y);
                SDL_RenderDrawLine(renderer, SCALE * corner3x - 1, SCALE * corner3y + 1, SCALE * corner4x - 1, SCALE * corner4y);
            }
        }

        SDL_RenderPresent(renderer);
        if (debug)
        {
            SDL_Delay(5);
        }

        time++;
        proceed_pattern(time);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

float pos[4];
float vel[4];
float ang[4];
float lumi[4];
float white[4];
int counter[3];
int counter_max[3];

#define globalHue 224
#define WATER_HUE globalHue
#define WATER_SCALE .5
#define WATER_SPEED .008
#define WATER_SPEED_RND 0.42
#define WATER_PERIOD_MIN 64
#define WATER_PERIOD_RND 16
#define WATER_BG 0.7
#define WATER_WHITE_MAX 1
#define WATER_WHITE_EXPONENT 2.6
#define WATER_Y_OFFSET 0.47

void init_pattern()
{
    for (int s=0; s<4; s++)
    {
        pos[s] = 0.;
        vel[s] = 0.;
        ang[s] = 0.;
        lumi[s] = 0.;
        white[s] = 0.;
    }

    counter_max[0] = WATER_PERIOD_MIN + random(WATER_PERIOD_RND);
    counter_max[1] = WATER_PERIOD_MIN + random(WATER_PERIOD_RND);
    counter_max[2] = WATER_PERIOD_MIN + random(WATER_PERIOD_RND);

    counter[0] = 0;
    counter[1] = -WATER_PERIOD_MIN / 3;
    counter[2] = -WATER_PERIOD_MIN * 2 / 3;
}

void proceed_pattern(float time)
{
    for(int p=0; p<3; p++)
    {
        if (counter[p] < counter_max[p])
        {
            pos[p] = WATER_SPEED * counter[p];
            counter[p]++;
        }
        if (counter[p] == counter_max[p])
        {
            lumi[p] = WATER_BG + (1 - WATER_BG) * 0.01 * random(100);
            white[p] = WATER_WHITE_MAX * pow(0.01 * random(100), WATER_WHITE_EXPONENT);
            counter_max[p] = WATER_PERIOD_MIN + random(WATER_PERIOD_RND);
            counter[p] = 0;
        }
    }
    // printf("STEP %f %i %i %f\n", pos[0], counter[0], counter_max[0], white[0]);
}

// Reminder: coord is scaled as [0,1] in each dimension.
LED shader(float time, vec2 coord, int pixel, int segment, int type)
{
    if (debug)
    {
        if (segment == 0)
        {
            return LED(1, 1);
        }
        return LED(200, .5, 1);
    }

    if (type == 1) // tie
    {
        LED led = LED(WATER_HUE, 0, WATER_BG);

        for(int p=0; p<3; p++)
        {
            float ypos = (float)(pos[p] - coord.y + WATER_Y_OFFSET);
            if (ypos >= 0)
            {
                led.mix(LED(WATER_HUE, white[p], lumi[p] * max(0., (1 - ypos / WATER_SCALE))), 1);
            }
        }

        return led;
    }
    else // leaves
    {
        LED led = LED(100, 0, .8);

        float r = sqrt(pow(coord.x - .5, 2) + pow(coord.y - .5, 2));
        float phi = 180./PI * atan2(coord.y - .5, coord.x - .5);
        float spiralHue = 100. + 30. * sin(10. * r + 0.01 * phi - 0.2 * time);

        LED led_spiral = LED(spiralHue, 0, 1);

//        printf("PIXEL %i %f %f %f %f\n", pixel, coord.x, coord.y, r, phi);

        return led_spiral;
    }
}

void print_all_segments(std::vector<Segment> segments)
{
    printf("\n");
    printf("    segments.clear();\n");
    for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
    {
        printf("    segments.push_back(Segment(%.2f, %.2f, %.2f, %.2f)); \t\t\t\t // Segment %i \t Length %.2f \t Pixels: %g \n",
            iseg->origin_x, iseg->origin_y, iseg->get_last_pixel_plus_one().x, iseg->get_last_pixel_plus_one().y,
            (iseg - segments.begin()), iseg->get_length(), iseg->get_length()/distance_LED_in_cm);
    }
    printf("\n");
}