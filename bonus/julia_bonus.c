/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   julia_bonus.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 10:27:45 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/17 11:40:48 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

/*
	iterates the function z^2 - c starting at argument z until the absolute
	value passes f->r, or maximum iterations are reached
	returns the number of iterations performed
*/
uint32_t	iter_julia(t_complex *z, t_fractal *f)
{
	uint32_t	res;
	t_complex	temp;

	if (complex_abs(z) > f->r)
		return (0);
	complex_mult(&temp, z, z);
	set_complex(&temp, temp.re + f->c.re, temp.im - f->c.im);
	res = 1;
	while (res < f->max_iters)
	{
		if (complex_abs(&temp) > f->r)
			return (res);
		complex_mult(&temp, &temp, &temp);
		set_complex(&temp, temp.re + f->c.re, temp.im - f->c.im);
		++res;
	}
	return (res);
}

//Calculates an escape radius for the julia set with parameter c
double	calc_julia_radius(t_complex *c)
{
	double	temp;
	double	r;

	temp = complex_abs(c);
	r = 0.5;
	while (r * r - r <= temp)
	{
		r *= 1.1;
	}
	return (r);
}

/*
	initialize the julia fractal: try to read in parameter c and check for 
	errors. Calculate the escape radius, set the iterator function pointer
	and make a color palette for the current max_iterations
*/
void	init_julia(t_data *data, int argc, char **argv)
{
	if (argc == 4)
	{
		if (!(check_input_arg(argv[2]) && check_input_arg(argv[3])))
		{
			data->error = 2;
			return ;
		}
		if (argv[2][0] == '-')
			data->f.c.re = -string_to_double(argv[2] + 1);
		else
			data->f.c.re = string_to_double(argv[2]);
		if (argv[3][0] == '-')
			data->f.c.im = -string_to_double(argv[3] + 1);
		else
			data->f.c.im = string_to_double(argv[3]);
	}
	else
		data->error = 2;
	if (data->error > 0)
		return ;
	data->f.r = calc_julia_radius(&(data->f.c));
	data->iterator = iter_julia;
	make_colors(&(data->f));
	if ((data->f.colors) == NULL)
		ft_exit((void *)data);
}
