/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newton_utils_bonus.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/09 09:32:17 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 17:01:51 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

/*
	the complex function z^3 - 1
*/
void	comp_function(t_complex *res, t_complex *z)
{
	t_complex	temp;

	complex_mult(&temp, z, z);
	complex_mult(res, &temp, z);
	set_complex(res, res->re - 1, res->im);
}

/*
	the derivative of the function z^3 - 1: 3z^2
*/
void	comp_deriv(t_complex *res, t_complex *z)
{
	t_complex	temp;

	complex_mult(res, z, z);
	set_complex(&temp, 3, 0);
	complex_mult(res, res, &temp);
}

/*
	This fractal needs 3 colors per iteration count, to represent the different
	solutions arrived at. 
*/
void	make_colors_newton(t_fractal *f)
{
	size_t		i;
	uint32_t	temp;

	if (f->size < 3 * (f->max_iters + 1))
	{
		free(f->colors);
		f->colors = malloc(sizeof(uint32_t) * 3 * (f->max_iters + 1));
	}
	if (f->colors == NULL)
		return ;
	else
		f->size = (f->max_iters + 1) * 3;
	i = 0;
	while (i < f->size)
	{
		temp = (i / 3) * 255 / f->max_iters;
		(f->colors)[i] = ((255 - temp) << 24 | 0xFF);
		(f->colors)[i + 1] = ((255 - temp) << 16 | 0xFF);
		(f->colors)[i + 2] = ((255 - temp) << 8 | 0xFF);
		i += 3;
	}
}
