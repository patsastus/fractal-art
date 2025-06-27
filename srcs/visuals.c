/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   visuals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 11:10:55 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 16:20:13 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol.h>

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
	const char	*s = "View control: arrow keys, 'r' to reset.";

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
