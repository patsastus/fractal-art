/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   julia.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 10:27:45 by nraatika          #+#    #+#             */
/*   Updated: 2025/05/27 17:00:10 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
//#include <stdio.h>
#include "fractol.h"

int iters(t_comp *z, t_comp *c, double r)
{
	int	res;
	t_comp	temp;

	res = 0;
	ft_mult(&temp, z, z);
	ft_add(&temp, &temp, c);
	while (res < MAX_ITER)
	{
		if (ft_abs(&temp) > r)
			return (res);
		ft_mult(&temp, &temp, &temp);
		ft_add(&temp, &temp, c);
		res++;
	}
	return (res);
}

void	new_image(t_image *im, t_anchor *a)
{
	int				x;
	int				y;
	t_param			*params;
	t_comp			temp;
	unsigned int	color;

	params = get_params();
	x = 0;
	while (x < a->width)
	{
		y = 0;
		while (y < a->height)
		{
			pixel_to_z(&temp, x, y, a);
			color = get_color(iters(&temp, &(params->c), params->r));
			set_pixel(im, x, y, color);
			y++;
		}
		x++;
	}
}

t_param	*get_params(void)
{
	static t_param p;
	return (&p);
}

double	calc_r(t_comp *c)
{
	double temp;
	double r;

	temp = ft_abs(c);
	r = temp;
	while (r*r - r <= temp)
	{
		r = r*1.1;
	}
	return r;
}

unsigned int	get_color(int i)
{
	(void)i;
	unsigned int u;
//	printf("%d ", i);
	if (i < MAX_ITER)
	{
		u = i * 256 / MAX_ITER << 16;
		return (u);
	}
	return (0xFFFFFFFF);
}
