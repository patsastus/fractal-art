/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fract-ol.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 14:20:43 by nraatika          #+#    #+#             */
/*   Updated: 2025/05/27 16:45:54 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "fractol.h"

int main(void)
{
	void	*mlx;
	void	*mlx_win;

	mlx = mlx_init();
	mlx_win = mlx_new_window(mlx, 1080, 1080, "Fract-ol");
	run_fractal(mlx, mlx_win);

}

void	make_image(t_image *im, int w, int h, void *mlx)
{
	im->img = mlx_new_image(mlx, w, h);
	im->addr = mlx_get_data_addr(im->img, &(im->bpp), &(im->linelen), 
		&(im->endian));
}

void	set_pixel(t_image *im, int x, int y, unsigned int color)
{
	char	*dst;

	dst = im->addr + (y * im->linelen + x * (im->bpp / 8));
	*(unsigned int *)dst = color;
}

void	pixel_to_z(t_comp *z, int x, int y, t_anchor *a)
{
	z->re = a->value.re + (a->x - x) * (a->step);
	z->im = a->value.im + (a->y - y) * (a->step);
} 

void	run_fractal(void *mlx, void *mlx_win)
{
	t_image		im;
	t_param		*params;
	t_anchor	a;	
	int			scale = 800;

	params = get_params();
	params->c.re = -0.4;
	params->c.im = 0.6;
	params->r = calc_r(&(params->c));
	init_anchor(&a, scale/2, scale/2, (params->r)/scale);
	make_image(&im, scale, scale, mlx);
	new_image(&im, &a);
	mlx_put_image_to_window(mlx, mlx_win, im.img, 50, 50);
	mlx_loop(mlx);
}

void	init_anchor(t_anchor *a, int w, int h, double scale)
{
	a->x = w / 2;
	a->y = h / 2;
	a->step = scale;
	(a->value).re = 0;
	(a->value).im = 0;
	a->height = h;
	a->width = w;
}
