/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_bonus.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 10:42:03 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 16:14:26 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

//initial viewpoint is always centered on (0,0i)
void	init_anchor(t_anchor *a, uint32_t x, uint32_t y, double scale)
{
	a->x = x;
	a->y = y;
	a->px_step = scale;
	(a->value).re = 0;
	(a->value).im = 0;
}

/*
	initialize the MLX window, the three images (fractal image, color scale, 
	and on-screen text), as well as the anchor pixel that's used as the basis 
	to draw the fractal image.
*/
void	init_all(t_data *data)
{
	uint32_t	scale;
	uint32_t	height;

	data->mlx = mlx_init(data->width, data->height, "Fractol", false);
	if ((data->mlx) == NULL)
	{
		data->error = 3;
		ft_exit((void *)data);
	}
	data->img = mlx_new_image(data->mlx, data->img_w, data->img_h);
	height = (data->img_h / data->f.size) * data->f.size;
	data->color_scale = mlx_new_image(data->mlx, OFFS / 2, height);
	make_text(data, OFFS, data->img_h + OFFS * 5 / 4);
	if (data->img == NULL || data->color_scale == NULL || data->text == NULL)
	{
		data->error = 4;
		ft_exit((void *)data);
	}
	make_scale(data);
	scale = RESOLUTION - 2 * OFFS;
	init_anchor(&(data->a), scale / 2, scale / 2, (data->f.r) * 3 / scale);
}

/*
	parsing input parameters, and storing them in the provided structure
	argv[1] : 'j' or 'm' to specify julia or mandelbrot set.
	argv[2-3] : used if argv[1] is 'j'. real and imaginary parts of c
	data->error gets set if anything goes wrong.
*/
void	parse_params(t_data *data, int argc, char **argv)
{
	set_sizes(data);
	if (argc < 2)
	{
		data->error = 1;
		return ;
	}
	if (argv[1][0] == 'j' || argv[1][0] == 'J')
	{
		init_julia(data, argc, argv);
	}
	else if (argv[1][0] == 'm' || argv[1][0] == 'M')
	{
		init_mandelbrot(data);
	}
	else if (argv[1][0] == 'n' || argv[1][0] == 'N')
	{
		init_newton(data);
	}
	else
		data->error = 1;
}

//makes a color palette to display fractal depth.
void	make_colors(t_fractal *f)
{
	size_t		i;
	uint32_t	c[3];

	i = 0;
	if (f->size < f->max_iters + 1)
	{
		if (f->colors != NULL)
			free(f->colors);
		f->colors = malloc(sizeof(uint32_t) * ((f->max_iters) + 1));
	}
	if (f->colors == NULL)
		return ;
	else
		f->size = f->max_iters + 1;
	while (i < f->max_iters)
	{
		c[0] = sawtooth(i, f->max_iters) * 4 * 255 / f->max_iters;
		c[1] = (i / 2) * 255 / f->max_iters;
		c[2] = (i / 4) * 255 / f->max_iters;
		(f->colors)[i] = (c[0] << 24 | c[1] << 16 | c[2] << 8 | 0xFF);
		i++;
	}
	(f->colors)[f->max_iters] = 0xFFFFFFFF;
}

/*
	sets window and image sizes as well as zoom step and maximum iterations
	Currently uses header constants for most things.
*/
void	set_sizes(t_data *data)
{
	data->height = RESOLUTION;
	data->width = RESOLUTION;
	data->img_h = RESOLUTION - 2 * OFFS;
	data->img_w = RESOLUTION - 2 * OFFS;
	data->zoom = 1.1;
	data->f.max_iters = MAX_ITERS;
	data->f.colors = NULL;
	data->f.size = 0;
	data->f.looping = 0;
}
