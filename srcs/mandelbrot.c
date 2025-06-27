/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mandelbrot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 10:06:58 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 12:17:42 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol.h>

/*
	iterates the complex function t = t^2 + z starting at t=(0,0) until the 
	absolute value of t passes f->r, or maximum iterations are reached.
	returns the number of iterations performed
*/
uint32_t	iter_mandelbrot(t_complex *z, t_fractal *f)
{
	uint32_t	res;
	t_complex	temp;

	if (complex_abs(z) > f->r)
		return (0);
	set_complex(&temp, z->re, z->im);
	res = 1;
	while (res < f->max_iters)
	{
		if (complex_abs(&temp) > f->r)
			return (res);
		complex_mult(&temp, &temp, &temp);
		set_complex(&temp, temp.re + z->re, temp.im + z->im);
		++res;
	}
	return (res);
}

/*
	initialize the mandelbrot fractal: escape radius is a constant 2, set the 
	iterator function pointer and make a color palette for 
	the current max_iterations
*/
void	init_mandelbrot(t_data *data)
{
	data->f.r = 2;
	data->iterator = iter_mandelbrot;
	make_colors(&(data->f));
	if ((data->f.colors) == NULL)
	{
		data->error = 2;
		ft_exit((void *)data);
	}
}
