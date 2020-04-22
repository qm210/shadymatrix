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
    vec2 get_last_pixel_plus_one() {return get_pixel(pixels-1);}
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
                                                                segments[selected_segment].get_last_pixel_plus_one().x, segments[selected_segment].get_last_pixel_plus_one().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x, height - segments[selected_segment].get_last_pixel_plus_one().y);
                            break;
                        case SDLK_DOWN:
                            selected_segment = (selected_segment - 1 + segments.size()) % segments.size();
                            printf("selected segment %i \t %i pixels \t\t (%.2f, %.2f)  (%.2f, %.2f) \t\t ANTI [ (%.2f, %.2f)  (%.2f, %.2f) ] \n", selected_segment, segments[selected_segment].pixels,
                                                                segments[selected_segment].origin_x, segments[selected_segment].origin_y,
                                                                segments[selected_segment].get_last_pixel_plus_one().x, segments[selected_segment].get_last_pixel_plus_one().y,
                                                                width - segments[selected_segment].origin_x, height - segments[selected_segment].origin_y,
                                                                width - segments[selected_segment].get_last_pixel_plus_one().x, height - segments[selected_segment].get_last_pixel_plus_one().y);
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
                                    iseg->origin_x, iseg->origin_y, iseg->get_last_pixel_plus_one().x, iseg->get_last_pixel_plus_one().y,
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
            float corner4x = iseg->get_last_pixel_plus_one().x + ca * L2 + sa * H2;
            float corner4y = iseg->get_last_pixel_plus_one().y - sa * L2 + ca * H2;
            float corner3x = iseg->get_last_pixel_plus_one().x + ca * L2 - sa * H2;
            float corner3y = iseg->get_last_pixel_plus_one().y - sa * L2 - ca * H2;
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
//        SDL_Delay(5);
        time++;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

float pos0 = 0;
float pos1 = 0;
float pos2 = 0;
float pos3 = 0;
float vel0 = 0;
float vel1 = 0;
float ang0 = 0;
float ang1 = 0;


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

  bool wal = true;

  float mod_time = fmod(time, 100.);
  if(mod_time < .01)
  {
    pos0 = pseudorandom(time);
    pos1 = pseudorandom(time + 1.);
    ang0 = 2.*PI*pseudorandom(2.*time);
  }
  float edge1 = .01 * mod_time;
  ang0 += 0.0033 * mod_time * 1e-4;
  float x = coord.x - pos0;
  float y = coord.y - pos1;
  float xx =  cos(ang0) * x + sin(ang0) * y;
  float yy = -sin(ang0) * x + cos(ang0) * y;
  float intensity = exp(-15. * fabs( min(edge1-fabs(xx), edge1-fabs(yy)) )) * (smoothstep(0,10,mod_time) - smoothstep(80, 100, mod_time));
  LED cube1 = wal ? LED(250, 0, .6 * intensity) : LED(266, .3, intensity);

  float mod_time2 = fmod(time + 83., 167.);
  if(mod_time2 < .02)
  {
    pos2 = pseudorandom(7.*time);
    pos3 = pseudorandom(7.*time + 1.);
    ang1 = 2.*PI*pseudorandom(3.*time);
  }
  float edge2 = .004 * (mod_time2);
  ang1 -= 0.0021 * mod_time2 * 1e-4;
  x =  cos(ang1) * coord.x + sin(ang1) * coord.y - pos2;
  y = -sin(ang1) * coord.x + cos(ang1) * coord.y - pos3;
  intensity = exp(-12.4 * fabs( min(edge2-fabs(x), edge2-fabs(y)) )) * (smoothstep(0,10,mod_time) - smoothstep(140, 167, mod_time2));
  LED cube2 = wal ? LED(166, .2, intensity) : LED(111, 0, intensity);

  cube1.mix_shitty(cube2, 1.);
  return cube1;
}
    /*
    bool wal = true;

    if(debug)
    {
        float hue = fmod(35. * segment, 360.);
        float white = 0;
        float lumi = 0.6;
        if(segment == 0)
        {
            white = 1;
            lumi = 1;
        }
        return LED(hue, white, lumi);
    }

    if(false)
    {
        float hue_cyan = 180.;
        float hue_magenta = 300.;
        float hue_avg = .5 * (hue_cyan + hue_magenta);
        float hue_spread = hue_magenta - hue_avg;

        LED base = LED(hue_cyan, 0, 1);

        float amount = sin(1.4 * coord.x + 4.7 * coord.y + .06 * time);
        LED add = LED(hue_magenta, (1-amount), amount);

        LED mix = (1-amount) * base + amount * add;

        float mod_time = fmod(time, 240.);
        float yellow_mixing = smoothstep(160., 200., mod_time) * (1. - smoothstep(200., 240., mod_time));
        mix = (1-yellow_mixing) * mix + yellow_mixing * LED(90, .6, 1);

        return mix;
    }
    else if(false)
    {
        LED led_bg = wal ? LED(250, 0, .25 + .25 * sin(0.001*time)) : LED(330, 0, .6);
        LED led_shine = wal ? LED(210, .4, 1.) : LED(90, .3, 1.);

        if(time < 0.01)
        {
            pos0 = pos1 = 0.5;
            vel0 = 3e-5; //.00010;
            vel1 = 1e-5; //.00003;
        }
        if(pos0 > 1 || pos0 < 0) vel0 *= -1;
        if(pos1 > 1 || pos1 < 0) vel1 *= -1;

        pos0 += vel0;
        pos1 += vel1;

        float distance = coord.get_distance_to(vec2(pos0, pos1));
        float shine = exp(-pow(distance*4.,2.));
        return shine * led_shine + (1-shine) * led_bg;
    }
    else if (false){
        float mod_pos = fmod(pixel + 0.6*time, 80);
        float intensity = exp(-.07 * mod_pos) * (mod_pos < 0 ? 0. : 1.);
        return LED(150, .3, intensity);
    }
    else if (true) {
        float mod_time = fmod(time, 100.);
        if(mod_time < .01)
        {
            pos0 = pseudorandom(time);
            pos1 = pseudorandom(time + 1.);
            ang0 = 2.*PI*pseudorandom(2.*time);
        }
        float edge1 = .01 * mod_time;
        ang0 += 0.0033 * mod_time * 1e-4;
        float x = coord.x - pos0;
        float y = coord.y - pos1;
        float xx =  cos(ang0) * x + sin(ang0) * y;
        float yy = -sin(ang0) * x + cos(ang0) * y;
        float intensity = exp(-15. * fabs( min(edge1-fabs(xx), edge1-fabs(yy)) )) * (1 - smoothstep(80, 100, mod_time));
        LED cube1 = wal ? LED(250, 0, .6 * intensity) : LED(266, .3, intensity);

        float mod_time2 = fmod(time + 83., 167.);
        if(mod_time2 < .02)
        {
            pos2 = pseudorandom(7.*time);
            pos3 = pseudorandom(7.*time + 1.);
            ang1 = 2.*PI*pseudorandom(3.*time);
        }
        float edge2 = .004 * (mod_time2);
        ang1 -= 0.0021 * mod_time2 * 1e-4;
        x =  cos(ang1) * coord.x + sin(ang1) * coord.y - pos2;
        y = -sin(ang1) * coord.x + cos(ang1) * coord.y - pos3;
        intensity = exp(-12.4 * fabs( min(edge2-fabs(x), edge2-fabs(y)) )) * (1 - smoothstep(140, 167, mod_time2));
        LED cube2 = wal ? LED(166, .2, intensity) : LED(111, 0, intensity);

        cube1.mix_shitty(cube2, 1.);

        return cube1;
    }
    else if (false) {
        float repeat = 200.;
        float mod_time = fmod(time, repeat);
        float div_time = floor(time/repeat)*repeat;

        LED cube;
        int n = 4;
        for(int c=0; c < n; c++)
        {
            if(c>0)
            {
                mod_time -= repeat/n;
                div_time += (float)c/n;
            }
            ang0 = pseudorandom(2.*div_time) * (2.*PI* + 0.033 * mod_time);
            float edge = .01 * (mod_time - c*50.);

            float center_x = 0.4 + 0.2 * pseudorandom(div_time);
            float center_y = 0.4 + 0.2 * pseudorandom(div_time * 7.);
            float x = coord.x - center_x;
            float y = coord.y - center_y;
            float xx =  cos(ang0) * x + sin(ang0) * y;
            float yy = -sin(ang0) * x + cos(ang0) * y;
            float intensity = exp(-15. * fabs( min(edge-fabs(xx), edge-fabs(yy)) )) * (1 - smoothstep(.8*repeat, repeat, mod_time));
            if (c==0)
            {
                LED cube = LED(266, .3, intensity);
            }
            else
            {
                cube.mix_shitty(LED(111, 0, intensity), 1.);
            }
        }

        return cube;
    }
}
*/
