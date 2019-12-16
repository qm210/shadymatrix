#include <stdio.h>
#include <cmath>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <vector>
#include "LED_WS.h"
#include "pixel.h"
#include "helper.h"

#define PI 3.141592
#define numpix P.size()

int count_LEDs_in_cross_matrix(int, int);
void init_pattern();
void proceed_pattern(float time);
LED shader(float time, vec2 coord, int pixel, int segment);

const float distance_LED_in_cm = 100. / 60.;
const float distance_LED_in_m = 0.01 * distance_LED_in_cm;

bool debug = false;

struct Segment
{
    float origin_x;
    float origin_y;
    float direction; // 0 to 360 degrees
    int pixels;
    Segment(float x, float y, float to_x, float to_y) {set(x, y, to_x, to_y);}
    void set(float x, float y, float to_x, float to_y)
    {
        set_origin(x, y);
        set_direction_to(to_x, to_y);
        set_length_to(to_x, to_y);
    }
    void set_origin(float x, float y) {origin_x = x; origin_y = y;}
    void set_direction(float dir) {direction = fmod(dir + 360., 360.);}
    void set_direction_to(float to_x, float to_y) {set_direction(180./PI*atan2(origin_y - to_y, to_x - origin_x));}
    void set_length(float l) {pixels = (int)round((l + .01) / distance_LED_in_cm);}
    void set_length_to(float to_x, float to_y) {set_length(sqrt(pow(to_x - origin_x, 2) + pow(to_y - origin_y, 2)));}
    float get_length() {return pixels * distance_LED_in_cm;}
    vec2 get_pixel(int i)
    {
        return vec2(
            origin_x + i * distance_LED_in_cm * cos(direction * PI/180.),
            origin_y - i * distance_LED_in_cm * sin(direction * PI/180.)
        );
    }
    vec2 get_last_pixel() {return get_pixel(pixels-1);}
    float get_rightmost_x() {return direction > 90 && direction < 270 ? origin_x : get_last_pixel().x;}
    float get_bottommost_y() {return direction < 180 ? origin_y : get_last_pixel().y;}

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
};

int main(int argc, char* argv[])
{
    SDL_Event e;

    std::vector<Segment> segments;
    std::vector<Pixel> P;

    const float D = distance_LED_in_cm;
    const float N = 80;
    const float A = N * D;
    const float B = A - D;

    float width = 32;
    float height = 32;
    float margin = 0;
    bool circleShape = true;
    bool helperLines = false;
    bool blurLights = true;
    bool drawStripRectangle = true;
    float stripWidth = 1.0;

    int selected_segment = 0;

    /// PATTERN

    segments.clear();
    segments.push_back(Segment(14.36, 12.27, 17.69, 12.18));                             // Segment 0    Length 3.33     Pixels: 2
    segments.push_back(Segment(17.50, 12.29, 15.95, 15.24));                             // Segment 1    Length 3.33     Pixels: 2
    segments.push_back(Segment(15.85, 14.79, 13.94, 12.06));                             // Segment 2    Length 3.33     Pixels: 2
    segments.push_back(Segment(16.71, 16.22, 17.55, 31.20));                             // Segment 3    Length 15.00    Pixels: 9
    segments.push_back(Segment(15.98, 31.14, 14.32, 31.03));                             // Segment 4    Length 1.67     Pixels: 1
    segments.push_back(Segment(14.43, 29.63, 15.43, 14.66));                             // Segment 5    Length 15.00    Pixels: 9
    segments.push_back(Segment(20.42, 10.37, 25.27, 9.15));                              // Segment 6    Length 5.00     Pixels: 3
    segments.push_back(Segment(25.37, 9.47, 28.55, 10.44));                              // Segment 7    Length 3.33     Pixels: 2
    segments.push_back(Segment(28.33, 10.99, 30.48, 17.30));                             // Segment 8    Length 6.67     Pixels: 4
    segments.push_back(Segment(17.57, 9.12, 19.02, 4.34));                               // Segment 9    Length 5.00     Pixels: 3
    segments.push_back(Segment(19.55, 4.52, 23.99, 2.22));                               // Segment 10   Length 5.00     Pixels: 3
    segments.push_back(Segment(14.64, 8.60, 12.91, 5.75));                               // Segment 11   Length 3.33     Pixels: 2
    segments.push_back(Segment(12.62, 5.89, 8.11, 3.72));                                // Segment 12   Length 5.00     Pixels: 3
    segments.push_back(Segment(8.00, 4.22, 3.66, 6.71));                                 // Segment 13   Length 5.00     Pixels: 3
    segments.push_back(Segment(12.03, 10.59, 7.20, 9.31));                               // Segment 14   Length 5.00     Pixels: 3
    segments.push_back(Segment(7.02, 9.39, 2.07, 13.86));                                // Segment 15   Length 6.67     Pixels: 4
    segments.push_back(Segment(2.02, 14.22, 2.17, 19.21));                               // Segment 16   Length 5.00     Pixels: 3
    segments.push_back(Segment(20.63, 13.15, 25.40, 14.64));                             // Segment 17   Length 5.00     Pixels: 3
    segments.push_back(Segment(25.14, 15.22, 26.56, 20.02));                             // Segment 18   Length 5.00     Pixels: 3
    segments.push_back(Segment(20.33, 14.04, 23.47, 15.18));                             // Segment 19   Length 3.33     Pixels: 2
    segments.push_back(Segment(23.71, 15.08, 25.04, 18.14));                             // Segment 20   Length 3.33     Pixels: 2
    segments.push_back(Segment(20.42, 11.12, 25.33, 10.22));                             // Segment 21   Length 5.00     Pixels: 3
    segments.push_back(Segment(25.00, 10.67, 28.27, 11.31));                             // Segment 22   Length 3.33     Pixels: 2
    segments.push_back(Segment(27.42, 11.83, 29.44, 16.41));                             // Segment 23   Length 5.00     Pixels: 3
    segments.push_back(Segment(18.29, 9.54, 19.28, 6.36));                               // Segment 24   Length 3.33     Pixels: 2
    segments.push_back(Segment(19.46, 6.21, 23.24, 2.94));                               // Segment 25   Length 5.00     Pixels: 3
    segments.push_back(Segment(15.04, 7.46, 11.81, 3.64));                               // Segment 26   Length 5.00     Pixels: 3
    segments.push_back(Segment(11.54, 4.00, 8.28, 3.32));                                // Segment 27   Length 3.33     Pixels: 2
    segments.push_back(Segment(8.75, 3.12, 4.24, 5.29));                                 // Segment 28   Length 5.00     Pixels: 3
    segments.push_back(Segment(11.03, 11.25, 6.13, 10.26));                              // Segment 29   Length 5.00     Pixels: 3
    segments.push_back(Segment(6.82, 11.06, 1.77, 15.41));                               // Segment 30   Length 6.67     Pixels: 4
    segments.push_back(Segment(2.92, 15.25, 2.81, 18.58));                               // Segment 31   Length 3.33     Pixels: 2

    /// END PATTERN

    float meters_required = numpix * distance_LED_in_m;
    printf("Pixels: %i\nMeters: %g\nWidth: %g\nHeight: %g\n", numpix, meters_required, width, height);

    const float SCALE = N * .3;
    const float MARGIN = margin * SCALE;
    const float WIDTH = width * SCALE;
    const float HEIGHT = height * SCALE;
    const float LEDSIZE = .125 * N;

    printf("SCALED HEIGHT %f", HEIGHT);

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

                        case SDLK_UP:
                            selected_segment = (selected_segment + 1) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x, segments[selected_segment].origin_y,
                                                                segments[selected_segment].get_last_pixel().x, segments[selected_segment].get_last_pixel().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel().x, height - segments[selected_segment].get_last_pixel().y);
                            break;
                        case SDLK_DOWN:
                            selected_segment = (selected_segment - 1 + segments.size()) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x, segments[selected_segment].origin_y,
                                                                segments[selected_segment].get_last_pixel().x, segments[selected_segment].get_last_pixel().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel().x, height - segments[selected_segment].get_last_pixel().y);
                            break;

                        case SDLK_ESCAPE:
                            quit = 1;
                            break;

                        case SDLK_RETURN:
                            printf("\n");
                            printf("    segments.clear();\n");
                            for (std::vector<Segment>::iterator iseg = segments.begin(); iseg != segments.end(); ++iseg)
                            {
                                printf("    segments.push_back(Segment(%.2f, %.2f, %.2f, %.2f)); \t\t\t\t // Segment %i \t Length %.2f \t Pixels: %g \n",
                                    iseg->origin_x, iseg->origin_y, iseg->get_last_pixel().x, iseg->get_last_pixel().y,
                                    (iseg - segments.begin()), iseg->get_length(), iseg->get_length()/distance_LED_in_cm);
                            }
                            printf("\n");
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
                pixel.L = shader(time, relative_coord, numpix, segcount - selected_segment);
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
            SDL_RenderDrawLine(renderer, SCALE * corner1x + 1, SCALE * corner1y, SCALE * corner2x + 1, SCALE * corner2y);
            SDL_RenderDrawLine(renderer, SCALE * corner2x + 1, SCALE * corner2y, SCALE * corner3x + 1, SCALE * corner3y);
            SDL_RenderDrawLine(renderer, SCALE * corner3x + 1, SCALE * corner3y, SCALE * corner4x + 1, SCALE * corner4y);
            SDL_RenderDrawLine(renderer, SCALE * corner4x + 1, SCALE * corner4y, SCALE * corner1x + 1, SCALE * corner1y);
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
    printf("STEP %f %i %i %f\n", pos[0], counter[0], counter_max[0], white[0]);

}

LED shader(float time, vec2 coord, int pixel, int segment)
{
    if (debug)
    {
        if (segment == 0)
        {
            return LED(1, 1);
        }
        return LED(200, .5, 1);
    }

    if (segment > 5) // leaves
    {
        LED led = LED(100, 0, .8);
        return led;
    }
    else // tie
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
}
