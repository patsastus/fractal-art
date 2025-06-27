/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newton_bonus.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 15:51:50 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 17:24:12 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

static uint32_t	get_min(t_complex *z);

/*
	Tries to find a solution for the function f(z): z^3 - 1 = 0,starting at 
	argument z, by using Newton's method where we set z = z - f(z)/f'(z)
	to check whether a solution has been reached, calls check_newton() to 
	compare	against the 3 known solutions. 
	Returns the number of iterations it took, and which solution was reached:
	iterations * 3 + solution_number (0 / 1 / 2)
*/
uint32_t	iter_newton(t_complex *z, t_fractal *f)
{
	uint32_t		res;
	char			n;
	t_complex		tmp[4];

	res = 0;
	set_complex((&tmp[3]), z->re, z->im);
	while (res < f->max_iters)
	{
		comp_function(&(tmp[0]), &(tmp[3]));
		comp_deriv(&(tmp[1]), &(tmp[3]));
		complex_div(&(tmp[2]), &(tmp[0]), &(tmp[1]));
		set_complex(&(tmp[3]), tmp[3].re - tmp[2].re, tmp[3].im - tmp[2].im);
		n = newton_check(&(tmp[3]));
		if (n < 3)
			return (res * 3 + (uint32_t)n);
		++res;
	}
	return (res * 3 + get_min(&tmp[3]));
}

/*
	initialize fractal. radius is just used for initial view here, and 
	max_iterations needs to be set lower because a lot more math is done per 
	pixel
*/
void	init_newton(t_data *data)
{
	data->f.r = 2;
	data->iterator = iter_newton;
	data->f.max_iters = 30;
	make_colors_newton(&(data->f));
	if ((data->f.colors) == NULL)
	{
		data->error = 2;
		ft_exit((void *)data);
	}
}

/*
	Checks whether the complex value z is within tolerance distance of a known
	solution for the function z^3 = 1 : 
	(1, 0), (-0.5, sqrt(3) / 2) , (-0.5, -sqrt(3) / 2)
*/
char	newton_check(t_complex *z)
{
	const double	tol = 0.0001;
	const double	n = sqrt(3) / 2;
	t_complex		temp;

	set_complex(&temp, z->re - 1, z->im);
	if (complex_abs(&temp) < tol)
		return (0);
	set_complex(&temp, z->re + 0.5, z->im - n);
	if (complex_abs(&temp) < tol)
		return (1);
	set_complex(&temp, z->re + 0.5, z->im + n);
	if (complex_abs(&temp) < tol)
		return (2);
	return (4);
}

/*
	returns the closest solution to the current z. called when max iterations 
	were reached to still differentiate between the different solutions.
*/
static uint32_t	get_min(t_complex *z)
{
	double			min;
	const double	n = sqrt(3) / 2;
	t_complex		temp;
	uint32_t		ret;

	min = HUGE_VAL;
	set_complex(&temp, z->re - 1, z->im);
	if (complex_abs(&temp) < min)
	{
		ret = 0;
		min = complex_abs(&temp);
	}
	set_complex(&temp, z->re + 0.5, z->im - n);
	if (complex_abs(&temp) < min)
	{
		ret = 1;
		min = complex_abs(&temp);
	}
	set_complex(&temp, z->re + 0.5, z->im + n);
	if (complex_abs(&temp) < min)
		return (2);
	return (ret);
}
