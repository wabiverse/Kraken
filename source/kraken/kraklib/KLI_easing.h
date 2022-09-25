/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/** \file
 * \ingroup kli
 */

#ifdef __cplusplus
extern "C" {
#endif

float KLI_easing_back_ease_in(
    float time, float begin, float change, float duration, float overshoot);
float KLI_easing_back_ease_out(
    float time, float begin, float change, float duration, float overshoot);
float KLI_easing_back_ease_in_out(
    float time, float begin, float change, float duration, float overshoot);
float KLI_easing_bounce_ease_out(float time, float begin, float change, float duration);
float KLI_easing_bounce_ease_in(float time, float begin, float change, float duration);
float KLI_easing_bounce_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_circ_ease_in(float time, float begin, float change, float duration);
float KLI_easing_circ_ease_out(float time, float begin, float change, float duration);
float KLI_easing_circ_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_cubic_ease_in(float time, float begin, float change, float duration);
float KLI_easing_cubic_ease_out(float time, float begin, float change, float duration);
float KLI_easing_cubic_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_elastic_ease_in(
    float time, float begin, float change, float duration, float amplitude, float period);
float KLI_easing_elastic_ease_out(
    float time, float begin, float change, float duration, float amplitude, float period);
float KLI_easing_elastic_ease_in_out(
    float time, float begin, float change, float duration, float amplitude, float period);
float KLI_easing_expo_ease_in(float time, float begin, float change, float duration);
float KLI_easing_expo_ease_out(float time, float begin, float change, float duration);
float KLI_easing_expo_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_linear_ease(float time, float begin, float change, float duration);
float KLI_easing_quad_ease_in(float time, float begin, float change, float duration);
float KLI_easing_quad_ease_out(float time, float begin, float change, float duration);
float KLI_easing_quad_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_quart_ease_in(float time, float begin, float change, float duration);
float KLI_easing_quart_ease_out(float time, float begin, float change, float duration);
float KLI_easing_quart_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_quint_ease_in(float time, float begin, float change, float duration);
float KLI_easing_quint_ease_out(float time, float begin, float change, float duration);
float KLI_easing_quint_ease_in_out(float time, float begin, float change, float duration);
float KLI_easing_sine_ease_in(float time, float begin, float change, float duration);
float KLI_easing_sine_ease_out(float time, float begin, float change, float duration);
float KLI_easing_sine_ease_in_out(float time, float begin, float change, float duration);

#ifdef __cplusplus
}
#endif
