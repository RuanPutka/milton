// This file is part of Milton.
//
// Milton is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// Milton is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Milton.  If not, see <http://www.gnu.org/licenses/>.


#include "system_includes.h"

#include "common.h"
#include "utils.h"


// total RAM in bytes
size_t get_system_RAM()
{
   return (size_t)SDL_GetSystemRAM() * 1024 * 1024;
}

v2i v2f_to_v2i(v2f p)
{
    return {(i32)p.x, (i32)p.y};
}

v2f v2i_to_v2f(v2i p)
{
    return {(f32)p.x, (f32)p.y};
}

// ---------------
// Math functions.
// ---------------

f32 magnitude(v2f a)
{
    return sqrtf(DOT(a, a));
}

f32 distance(v2f a, v2f b)
{
    v2f diff = a - b;

    f32 dist = sqrtf(DOT(diff, diff));

    return dist;
}

f32 deegrees_to_radians(int d)
{
    assert (0 <= d && d < 360);
    return kPi * ((f32)(d) / 180.0f);
}

f32 radians_to_degrees(f32 r)
{
    return (180 * r) / kPi;
}



// Could be called a signed area. `orientation(a, b, c) / 2` is the area of the
// triangle.
// If positive, c is to the left of ab. Negative: right of ab. 0 if
// colinear.
f32 orientation(v2f a, v2f b, v2f c)
{
    return (b.x - a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y);
}

b32 is_inside_triangle(v2f point, v2f a, v2f b, v2f c)
{
   b32 is_inside =
           (orientation(a, b, point) <= 0) &&
           (orientation(b, c, point) <= 0) &&
           (orientation(c, a, point) <= 0);
    return is_inside;
}

v2f polar_to_cartesian(f32 angle, f32 radius)
{
    v2f result = {
        radius * cosf(angle),
        radius * sinf(angle)
    };
    return result;
}

v2i rotate_v2i(v2i p, f32 angle)
{
    v2i r = {
        (i32)((p.x * cosf(angle)) - (p.y * sinf(angle))),
        (i32)((p.x * sinf(angle)) + (p.y * cosf(angle))),
    };
    return r;
}

v2f closest_point_in_segment_f(i32 ax, i32 ay,
                               i32 bx, i32 by,
                               v2f ab, f32 ab_magnitude_squared,
                               v2i point, f32* out_t)
{
    v2f result;
    f32 mag_ab = sqrtf(ab_magnitude_squared);
    f32 d_x = ab.x / mag_ab;
    f32 d_y = ab.y / mag_ab;
    f32 ax_x = (f32)(point.x - ax);
    f32 ax_y = (f32)(point.y - ay);
    f32 disc = d_x * ax_x + d_y * ax_y;
    if (disc < 0) disc = 0;
    if (disc > mag_ab) disc = mag_ab;
    if (out_t) {
        *out_t = disc / mag_ab;
    }
    result = {(ax + disc * d_x), (ay + disc * d_y)};
    return result;
}

v2i closest_point_in_segment(v2i a, v2i b,
                             v2f ab, f32 ab_magnitude_squared,
                             v2i point, f32* out_t)
{
    v2i result;
    f32 mag_ab = sqrtf(ab_magnitude_squared);
    f32 d_x = ab.x / mag_ab;
    f32 d_y = ab.y / mag_ab;
    f32 ax_x = (f32)(point.x - a.x);
    f32 ax_y = (f32)(point.y - a.y);
    f32 disc = d_x * ax_x + d_y * ax_y;
    if (disc < 0) disc = 0;
    if (disc > mag_ab) disc = mag_ab;
    if (out_t) {
        *out_t = disc / mag_ab;
    }
    result = {(i32)(a.x + disc * d_x), (i32)(a.y + disc * d_y)};
    return result;
}

b32 intersect_line_segments(v2i a, v2i b,
                            v2i u, v2i v,
                            v2f* out_intersection)
{
    b32 hit = false;
    v2i perp = perpendicular(v - u);
    i32 det = DOT(b - a, perp);
    if (det != 0) {
        f32 t = (f32)DOT(u - a, perp) / (f32)det;
        if (t > 1 && t < 1.001) t = 1;
        if (t < 0 && t > -0.001) t = 1;

        if (t >= 0 && t <= 1) {
            hit = true;
            out_intersection->x = a.x + t * (b.x - a.x);
            out_intersection->y = a.y + t * (b.y - a.y);
        }
    }
    return hit;
}

// Returns the number of rectangles into which src_rect was split.
i32 rect_split(StretchArray<Rect>& out_rects, Rect src_rect, i32 width, i32 height)
{
    int n_width = (src_rect.right - src_rect.left) / width;
    int n_height = (src_rect.bottom - src_rect.top) / height;

    if (!n_width || !n_height) {
        return 0;
    }

    i32 max_num_rects = (n_width + 1) * (n_height + 1);
    StretchArray<Rect> rects((size_t)max(max_num_rects / 2, 1));

    for ( int h = src_rect.top; h < src_rect.bottom; h += height ) {
        for ( int w = src_rect.left; w < src_rect.right; w += width ) {
            Rect rect;
            {
                rect.left = w;
                rect.right = min(src_rect.right, w + width);
                rect.top = h;
                rect.bottom = min(src_rect.bottom, h + height);
            }
            push(rects, rect);
        }
    }

    assert((i32)count(rects) <= max_num_rects);
    i32 num_rects = (i32)count(rects);
    out_rects = rects;
    return num_rects;
}

Rect rect_union(Rect a, Rect b)
{
    Rect result;
    result.left = min(a.left, b.left);
    result.right = max(a.right, b.right);

    if (result.left > result.right) {
        result.left = result.right;
    }
    result.top = min(a.top, b.top);
    result.bottom = max(a.bottom, b.bottom);
    if (result.bottom < result.top) {
        result.bottom = result.top;
    }
    return result;
}

Rect rect_intersect(Rect a, Rect b)
{
    Rect result;
    result.left = max(a.left, b.left);
    result.right = min(a.right, b.right);

    if (result.left >= result.right) {
        result.left = result.right;
    }
    result.top = max(a.top, b.top);
    result.bottom = min(a.bottom, b.bottom);
    if (result.bottom <= result.top) {
        result.bottom = result.top;
    }
    return result;
}
Rect rect_stretch(Rect rect, i32 width)
{
   Rect stretched = rect;
   // Make the raster limits at least as wide as a block
   if (stretched.bottom - stretched.top < width) {
      stretched.top -= width / 2;
      stretched.bottom += width / 2;
   }
   if (stretched.right - stretched.left < width) {
      stretched.left -= width / 2;
      stretched.right += width / 2;
   }
   return stretched;
}

Rect rect_clip_to_screen(Rect limits, v2i screen_size)
{
    if (limits.left < 0) limits.left = 0;
    if (limits.right > screen_size.w) limits.right = screen_size.w;
    if (limits.top < 0) limits.top = 0;
    if (limits.bottom > screen_size.h) limits.bottom = screen_size.h;
    return limits;
}

const Rect rect_enlarge(Rect src, i32 offset)
{
    Rect result;
    result.left = src.left - offset;
    result.top = src.top - offset;
    result.right = src.right + offset;
    result.bottom = src.bottom + offset;
    return result;
}

Rect bounding_rect_for_points(v2i points[], i32 num_points)
{
    assert (num_points > 0);

    v2i top_left =  points[0];
    v2i bot_right = points[0];

    for (i32 i = 1; i < num_points; ++i) {
        v2i point = points[i];
        if (point.x < top_left.x)   top_left.x = point.x;
        if (point.x > bot_right.x)  bot_right.x = point.x;

        if (point.y < top_left.y)   top_left.y = point.y;
        if (point.y > bot_right.y)  bot_right.y = point.y;
    }
    Rect rect = { top_left, bot_right };
    return rect;
}

b32 is_inside_rect(Rect bounds, v2i point)
{
    return
        point.x >= bounds.left &&
        point.x <  bounds.right &&
        point.y >= bounds.top &&
        point.y <  bounds.bottom;
}

b32 rect_is_valid(Rect rect)
{
    b32 valid = rect.left <= rect.right && rect.top <= rect.bottom;
    return valid;
}

Rect bounding_rect_for_points_scalar(i32 points_x[], i32 points_y[], i32 num_points)
{
    assert (num_points > 0);

    i32 top_left_x =  points_x[0];
    i32 bot_right_x = points_x[0];

    i32 top_left_y =  points_y[0];
    i32 bot_right_y = points_y[0];

    for (i32 i = 1; i < num_points; ++i) {
        if (points_x[i] < top_left_x)   top_left_x = points_x[i];
        if (points_x[i] > bot_right_x)  bot_right_x = points_x[i];

        if (points_y[i] < top_left_y)   top_left_y = points_y[i];
        if (points_y[i] > bot_right_y)  bot_right_y = points_y[i];
    }
    Rect rect;
        rect.left = top_left_x;
        rect.right = bot_right_x;
        rect.top = top_left_y;
        rect.bottom = bot_right_y;
    return rect;
}

i32 rect_area(Rect rect)
{
    return (rect.right - rect.left) * (rect.bottom - rect.top);
}

b32 is_inside_rect_scalar(Rect bounds, i32 point_x, i32 point_y)
{
    return
        point_x >= bounds.left &&
        point_x <  bounds.right &&
        point_y >= bounds.top &&
        point_y <  bounds.bottom;
}

b32 is_rect_within_rect(Rect a, Rect b)
{
    if ( (a.left   < b.left)    ||
         (a.right  > b.right)   ||
         (a.top    < b.top)     ||
         (a.bottom > b.bottom) ) {
        return false;
      }
   return true;
}

Rect rect_from_xywh(i32 x, i32 y, i32 w, i32 h)
{
    Rect rect;
    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;

    return rect;
}

