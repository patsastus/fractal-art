/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   visuals_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 11:10:55 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 16:18:25 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

/*
	BONUS: a function that gets hooked to the main mlx loop, enabled or disabled
	by setting the data->f.looping flag. loops all but the final element of the 
	color palette by one. 
	Has an internal counter so update happens every 5th frame if enabled.
	calls make_scale and draw_fractal to update images if colors were changed
*/
void	loop_colors(void *p)
{
	static char	c;
	t_data		*data;
	size_t		i;
	uint32_t	temp;

	++c;
	data = p;
	if (c % 5 == 0 && data->f.looping)
	{
		i = 0;
		temp = data->f.colors[0];
		while (i < data->f.size - 2)
		{
			data->f.colors[i] = data->f.colors[i + 1];
			i++;
		}
		data->f.colors[i] = temp;
		make_scale((void *)data);
		draw_fractal((void *)data);
	}
}

/*
	Makes a vertical bar to display the color scale.
*/
void	make_scale(t_data *data)
{
	size_t		s[2];
	uint32_t	height;
	uint32_t	i;

	height = (data->img_h / (data->f.size)) * data->f.size;
	s[0] = 0;
	while (s[0] < OFFS / 2)
	{
		s[1] = 0;
		while (s[1] < height)
		{
			i = s[1] * data->f.size / height;
			mlx_put_pixel(data->color_scale, s[0], s[1], (data->f.colors)[i]);
			(s[1])++;
		}
		(s[0])++;
	}
}

//Resets viewpoint to intial value
void	reset_view(void *p)
{
	uint32_t	scale;
	t_data		*data;

	data = p;
	scale = RESOLUTION - 2 * OFFS;
	init_anchor(&(data->a), scale / 2, scale / 2, (data->f.r) * 3 / scale);
}

//Writes some instructions on screen
void	make_text(t_data *data, int32_t x, int32_t y)
{
	const char	*s = "View control: arrow keys, 'r' to reset. Loop colors: 'a'";

	data->text = mlx_put_string(data->mlx, s, x, y);
}

/*
	Draws a fractal image. 
	First the top-left pixel gets assigned a complex value based on anchor 
	pixel, then loops through all pixels in image, calling the iterator function
	to get an iteration count for that value, translates that into a color
	via the color palette and updates the image.
	Then gets the complex value of the next pixel by addition and/or resetting
*/
void	draw_fractal(void *p)
{
	t_data		*data;
	int32_t		x;
	int32_t		y;
	t_complex	temp[2];
	uint32_t	i;

	data = p;
	x = 0;
	pixel_to_complex(&(temp[0]), 0, 0, &(data->a));
	set_complex(&(temp[1]), temp[0].re, temp[0].im);
	while ((uint32_t)x < data->img_w)
	{
		y = 0;
		while ((uint32_t)y < data->img_h)
		{
			i = (data->iterator)(&(temp[1]), &(data->f));
			mlx_put_pixel(data->img, x, y, (data->f.colors)[i]);
			set_complex(&(temp[1]), temp[1].re, temp[1].im - data->a.px_step);
			++y;
		}
		set_complex(&(temp[1]), temp[1].re + data->a.px_step, temp[0].im);
		++x;
	}
}
