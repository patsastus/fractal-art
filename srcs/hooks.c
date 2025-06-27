/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hooks.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 16:15:26 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 15:47:19 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol.h>

//panning: updates anchor pixel to reflect change in view
static void	update_pan(t_data *data, char direction)
{
	uint32_t	xp;
	uint32_t	yp;
	t_complex	z;

	xp = data->img_w / 2;
	yp = data->img_h / 2;
	if (direction % 4 == 0)
		xp -= data->img_w / 10;
	if (direction % 4 == 1)
		xp += data->img_w / 10;
	if (direction % 4 == 2)
		yp -= data->img_h / 10;
	if (direction % 4 == 3)
		yp += data->img_h / 10;
	pixel_to_complex(&z, xp, yp, &(data->a));
	data->a.value.re = z.re;
	data->a.value.im = z.im;
	data->a.x = data->img_w / 2;
	data->a.y = data->img_h / 2;
}

//zooming: updates anchor pixel to reflect zooming in or out
static void	update_zoom(t_data *data, double d, int32_t x, int32_t y)
{
	uint32_t	xp;
	uint32_t	yp;
	t_complex	z;

	if ((uint32_t)x > OFFS && (uint32_t)x < OFFS + data->img_w)
		xp = x - OFFS;
	else
		xp = data->img_w / 2;
	if ((uint32_t)y > OFFS && (uint32_t)y < OFFS + data->img_h)
		yp = y - OFFS;
	else
		yp = data->img_h / 2;
	if (xp != (uint32_t)(data->a.x) || yp != (uint32_t)(data->a.x))
	{
		pixel_to_complex(&z, xp, yp, &(data->a));
		set_complex(&(data->a.value), z.re, z.im);
		data->a.x = xp;
		data->a.y = yp;
	}
	if (d > 0 && data->a.px_step < (HUGE_VAL / 1000))
		data->a.px_step *= data->zoom;
	if (d < 0 && data->a.px_step > 1e-150)
		data->a.px_step *= (2 - data->zoom);
}

void	handle_keys(mlx_key_data_t keydata, void *param)
{
	t_data	*data;

	data = param;
	if (keydata.key == MLX_KEY_ESCAPE && keydata.action == MLX_PRESS)
		ft_exit((void *)data);
	if (keydata.key == MLX_KEY_R && keydata.action == MLX_PRESS)
		reset_view((void *)data);
	if (keydata.key == MLX_KEY_RIGHT && keydata.action == MLX_PRESS)
		update_pan(data, 1);
	if (keydata.key == MLX_KEY_LEFT && keydata.action == MLX_PRESS)
		update_pan(data, 0);
	if (keydata.key == MLX_KEY_UP && keydata.action == MLX_PRESS)
		update_pan(data, 2);
	if (keydata.key == MLX_KEY_DOWN && keydata.action == MLX_PRESS)
		update_pan(data, 3);
	draw_fractal((void *)data);
}

void	handle_mouse(double xdelta, double ydelta, void *param)
{
	t_data	*data;
	int32_t	x;
	int32_t	y;

	(void)xdelta;
	data = (t_data *)param;
	mlx_get_mouse_pos(data->mlx, &x, &y);
	update_zoom(data, ydelta, x, y);
	draw_fractal((void *)data);
}
