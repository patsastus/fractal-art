/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   complex_bonus.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nraatika <nraatika@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 10:21:49 by nraatika          #+#    #+#             */
/*   Updated: 2025/06/16 16:14:26 by nraatika         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fractol_bonus.h>

void	complex_mult(t_complex *res, t_complex *one, t_complex *two)
{
	t_complex	temp;

	temp.re = (one->re) * (two->re) - (one->im) * (two->im);
	temp.im = (one->re) * (two->im) + (one->im) * (two->re);
	res->re = temp.re;
	res->im = temp.im;
}

void	complex_div(t_complex *res, t_complex *num, t_complex *den)
{
	double	conjugate;

	conjugate = den->re * den->re + den->im * den->im;
	res->re = (num->re * den->re + num->im * den->im) / conjugate;
	res->im = (num->im * den->re - num->re * den->im) / conjugate;
}

double	complex_abs(t_complex *z)
{
	return (sqrt((z->re) * (z->re) + (z->im) * (z->im)));
}

void	set_complex(t_complex *z, double re, double im)
{
	z->re = re;
	z->im = im;
}

void	pixel_to_complex(t_complex *z, int32_t x, int32_t y, t_anchor *a)
{
	z->re = a->value.re + (x - a->x) * (a->px_step);
	z->im = a->value.im + (a->y - y) * (a->px_step);
}
